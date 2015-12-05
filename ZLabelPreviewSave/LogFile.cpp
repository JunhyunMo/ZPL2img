#include "stdafx.h"
#include "LogFile.h"
#include <tchar.h>

class CLogCritical 
{
	CRITICAL_SECTION m_CriticalSection;
public:
	CLogCritical() { InitializeCriticalSection(&m_CriticalSection); }
	~CLogCritical() { DeleteCriticalSection(&m_CriticalSection); }
	operator LPCRITICAL_SECTION() { return &m_CriticalSection; }
};

CLogCritical log_cs;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogFile::CLogFile()
: log_data(NULL)
, last_write_time(0)
, file_type(LOGFILE_ONE_FILE)
, max_log_size(MAX_LOG_SIZE)
, log_size(0)
, log_count(0)
, max_log_count(0)	// count no check
, max_log_time(0)   // time no check
, mDelim(TEXT(' '))
, mLevel(LL_ALL)
, mPrint(false)
, mTrace(false)
, mSend(false)
{
	*log_file_name			= TEXT('\0');
	*temp_file_name			= TEXT('\0');

	GetLocalTime(&last_date);
	InitializeCriticalSectionAndSpinCount(&cs, LOG_SPIN_COUNT);
}

CLogFile::~CLogFile()
{
	WriteLogData();
	if (log_data) {
		//	HeapFree(hHeap, 0, log_data);
		VirtualFree(log_data, 0, MEM_RELEASE);
	}
	DeleteCriticalSection(&cs);
}

static BOOL CreateUpperDirectory(LPCTSTR filename)
{
	TCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	TCHAR path[_MAX_PATH];

	_wsplitpath(filename, drive, dir, fname, ext);
	int len = _tcslen(dir);
	if (len <= 1) { // "/"
		return TRUE;
	}
	dir[len - 1] = _T('\0');

	_stprintf(path, _T("%s%s"), drive, dir);
	if (!CreateUpperDirectory(path)) {
		return FALSE;
	}

	if (!CreateDirectory(path, NULL)) {
		DWORD error = GetLastError(); 

		switch (error) {
		case ERROR_ALREADY_EXISTS:
			return TRUE;
		default:
			return FALSE;
		}
	}
	return TRUE; 
}

BOOL CLogFile::PutTimeLog(LPCTSTR fmt, ...)
{
	BOOL flag = FALSE;
	va_list args;

	va_start(args, fmt);

	flag = PutTimeLogV(fmt, args);

	va_end(args);

	return flag;
}

BOOL CLogFile::SetLogFile(LPCTSTR filename, LogFileType type, DWORD size, DWORD count, DWORD time)
{
	if(!CreateUpperDirectory(filename))
		return FALSE; 

	WriteLogData();

	_tcscpy(log_file_name, filename);
	file_type = type;
	max_log_size = min(size, MAX_LOG_SIZE);
	max_log_count = count;
	max_log_time = time;
	if (log_data) {
		/*
		LPTSTR temp = (LPTSTR)HeapReAlloc(hHeap, 0, log_data, max_log_size);
		if (temp == NULL)
		HeapFree(hHeap, 0, log_data);
		log_data = temp;
		*/
		LPTSTR temp = (LPTSTR)VirtualAlloc(NULL, max_log_size, MEM_COMMIT, PAGE_READWRITE);
		if (temp)
			memcpy(temp, log_data, log_size);
		VirtualFree(log_data, 0, MEM_RELEASE);
		log_data = temp;
	}
	else if (max_log_size > 0) {
		//		log_data = (LPTSTR)HeapAlloc(hHeap, 0, max_log_size);
		log_data = (LPTSTR)VirtualAlloc(NULL, max_log_size, MEM_COMMIT, PAGE_READWRITE);
	}

	if (max_log_size > 0 && log_data == NULL) {
		return FALSE;
	}

	return TRUE;
}

BOOL CLogFile::PutLog(LPCTSTR log_message)
{
	DWORD len = _tcslen(log_message) * sizeof(TCHAR);
	BOOL result = TRUE;

	EnterCriticalSection(&cs);
	if (file_type != LOGFILE_ONE_FILE) {
		BOOL write = FALSE;
		SYSTEMTIME st;

		GetLocalTime(&st);
		switch (file_type) {
		case LOGFILE_PER_YEAR:
			if (st.wYear != last_date.wYear)
				write = TRUE;
			break;
		case LOGFILE_PER_MONTH:
			if (st.wMonth != last_date.wMonth || st.wYear != last_date.wYear)
				write = TRUE;
			break;
		case LOGFILE_PER_DAY:
			if (st.wDay != last_date.wDay || st.wMonth != last_date.wMonth || st.wYear != last_date.wYear)
				write = TRUE;
			break;
		case LOGFILE_PER_HOUR:
			if (st.wHour != last_date.wHour || st.wDay != last_date.wDay || st.wMonth != last_date.wMonth || st.wYear != last_date.wYear)
				write = TRUE;
			break;
		case LOGFILE_PER_TENMIN:
			if (st.wMinute / 10 != last_date.wMinute / 10 || st.wHour != last_date.wHour || st.wDay != last_date.wDay || st.wMonth != last_date.wMonth || st.wYear != last_date.wYear)
				write = TRUE;
			break;
		}
		if (write)
			result = WriteLogData();
		last_date = st;
	}

	if (log_size + len >= max_log_size - 1)
		result = WriteLogData();
	if (result) {
		if (len >= max_log_size)
			result = WriteLogFile(log_message);
		else {
			if (log_data) {
				memcpy(log_data + log_size / sizeof(TCHAR), log_message, len);
				log_size += len;
				log_count++;

				if (logCountExpired() || logTimeExpired()) {
					result = WriteLogData();
					if (result == FALSE) {
						log_size -= len;
						log_count--;
					}
				}
			}
		}
	}
	LeaveCriticalSection(&cs);

	return result;
}

static int LFToCRLF(LPCSTR s, int s_len, LPSTR t, int t_len)
{
	const CHAR CR = '\r', LF = '\n', EOS = '\0';

	int i, j = 0, len = s_len < 0 ? strlen(s) : s_len - 1;
	for (i = 0; i < len; i++) {
		if (s[i] == LF && i > 0 && s[i - 1] != CR) {
			if (t_len == 0)
				j += 2;
			else if (j < t_len - 2) {
				t[j++] = CR;
				t[j++] = LF;
			}
			else {
				j = 0;
				break;
			}
		}
		else {
			if (t_len == 0)
				j++;
			else if (j < t_len - 1)
				t[j++] = s[i];
			else {
				j = 0;
				break;
			}
		}
	}
	if (t_len == 0)
		j++;
	else {
		if (j > 0)
			t[j++] = EOS;
		else
			SetLastError(ERROR_INSUFFICIENT_BUFFER);
	}

	return j;
}

int CLogFile::PrepareBuffer(LPCTSTR in_buf, int in_buf_len, LPSTR *out_buf)
{
	int		buf_len;
	LPSTR	buffer;

#ifdef _UNICODE
	int temp_buf_len = WideCharToMultiByte(CP_THREAD_ACP, 0, in_buf, in_buf_len, NULL, 0, NULL, NULL);
	LPSTR temp_buf = (LPSTR)VirtualAlloc(NULL, temp_buf_len, MEM_COMMIT, PAGE_READWRITE);
	if (NULL == temp_buf) {
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		return 0;
	}

	WideCharToMultiByte(CP_THREAD_ACP, 0, in_buf, in_buf_len, temp_buf, temp_buf_len, NULL, NULL);
	//--------------------------------------------------------------------------------------
	buf_len = LFToCRLF(temp_buf, temp_buf_len, NULL, 0);
	buffer = (LPSTR)VirtualAlloc(NULL, buf_len, MEM_COMMIT, PAGE_READWRITE);
	if (NULL == buffer) {
		VirtualFree(temp_buf, 0, MEM_RELEASE);
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		return 0;
	}
	LFToCRLF(temp_buf, temp_buf_len, buffer, buf_len);
	//--------------------------------------------------------------------------------------
	VirtualFree(temp_buf, 0, MEM_RELEASE);
#else
	//--------------------------------------------------------------------------------------
	buf_len = LFToCRLF(in_buf, in_buf_len, NULL, 0);
	buffer = (LPSTR)VirtualAlloc(NULL, buf_len, MEM_COMMIT, PAGE_READWRITE);
	if (NULL == buffer) {
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		return 0;
	}
	LFToCRLF(in_buf, in_buf_len, buffer, buf_len);
	//--------------------------------------------------------------------------------------
#endif

	*out_buf = buffer;
	return buf_len;
}

BOOL CLogFile::WriteLogData()
{
	if (log_size > 0) {
		log_data[log_size / sizeof(TCHAR)] = TEXT('\0');
		if (WriteLogFile(log_data)) {
			log_size = 0;
			log_count = 0;
			return TRUE;
		}
		else
			return FALSE;
	}
	else
		return TRUE;
}

BOOL CLogFile::WriteLogFile(LPCTSTR log_message)
{
	BOOL result = FALSE;

	_ASSERT(log_message);
	if (log_message) {
		LPSTR write_buf;
		int len = PrepareBuffer(log_message, _tcslen(log_message) + 1, &write_buf);
		if (len <= 0)
			return FALSE;

		EnterCriticalSection(log_cs);
		{
			HANDLE hFile = openLogFile();
			if (hFile != INVALID_HANDLE_VALUE) {
				DWORD bytes;

				SetFilePointer(hFile, 0, NULL, FILE_END);
				_ASSERT(write_buf);
				if (WriteFile(hFile, write_buf, len - 1, &bytes, NULL)) {
					last_write_time = GetTickCount();
					result = TRUE;
				}
				CloseHandle(hFile);
			}
		}
		LeaveCriticalSection(log_cs);

		VirtualFree(write_buf, 0, MEM_RELEASE);
	}
	else 
		return TRUE;

	return result;
}

BOOL CLogFile::PutErrorLog(LPCTSTR title, DWORD error_code, BOOL get_msg, LPCTSTR msg)
{
	TCHAR error_msg[LOG_ERROR_MSG_SIZE];

	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		_stprintf(error_msg, TEXT("%04d-%02d-%02d %02d:%02d:%02d%c%s\n"),
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, mDelim,
			title ? title : TEXT(""));
	}

	if (error_code != 0) {
		if (get_msg) {
			LPTSTR lpMsgBuf;

			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&lpMsgBuf, 0, NULL);
			_stprintf(error_msg + _tcslen(error_msg), TEXT("  Error: %d - %s"), error_code, lpMsgBuf);
			LocalFree(lpMsgBuf);
		}
		else
			_stprintf(error_msg + _tcslen(error_msg), TEXT("  Error: %d\n"), error_code);
	}
	if (msg != NULL)
		_stprintf(error_msg + _tcslen(error_msg), TEXT("  %s\n"), msg);

	return PutLog(error_msg);
}

BOOL CLogFile::PutTimeLogV(LPCTSTR fmt, char *args)
{
	WCHAR msg[LOG_ERROR_MSG_SIZE];
	WCHAR msg_trans[LOG_ERROR_MSG_SIZE];

	if (fmt == NULL)
		return FALSE;

	_vsntprintf(msg_trans, LOG_ERROR_MSG_SIZE - 1, fmt, args);
	msg_trans[LOG_ERROR_MSG_SIZE - 1] = 0;

	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		_sntprintf(msg, LOG_ERROR_MSG_SIZE, TEXT("%04d-%02d-%02d %02d:%02d:%02d%c%s\n"),
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, mDelim,
			msg_trans);
		msg[LOG_ERROR_MSG_SIZE - 1] = 0;
	}

	if (mPrint)	wprintf(msg);
	if (mTrace)	OutputDebugString(msg);
	if (mSend) Send(msg);

	return PutLog(msg);
}

LPCTSTR CLogFile::GetLogFileName()
{
	if (file_type != LOGFILE_ONE_FILE) {
		WCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];

		_wsplitpath(log_file_name, drive, dir, fname, ext);
		switch (file_type) {
		case LOGFILE_PER_YEAR:
			_stprintf(temp_file_name, TEXT("%s%s%s_%04d"), drive, dir, fname, last_date.wYear);
			break;
		case LOGFILE_PER_MONTH:
			_stprintf(temp_file_name, TEXT("%s%s%s_%04d_%02d"), drive, dir, fname, last_date.wYear, last_date.wMonth);
			break;
		case LOGFILE_PER_DAY:
			_stprintf(temp_file_name, TEXT("%s%s%s_%04d_%02d_%02d"), drive, dir, fname, last_date.wYear, last_date.wMonth, last_date.wDay);
			break;
		case LOGFILE_PER_HOUR:
			_stprintf(temp_file_name, TEXT("%s%s%s_%04d_%02d_%02d.%02d"), drive, dir, fname, last_date.wYear, last_date.wMonth, last_date.wDay, last_date.wHour);
			break;
		case LOGFILE_PER_TENMIN:
			_stprintf(temp_file_name, TEXT("%s%s%s_%04d_%02d_%02d.%02d.%1d0"), drive, dir, fname, last_date.wYear, last_date.wMonth, last_date.wDay, last_date.wHour, last_date.wMinute / 10);
			break;
		}

		_stprintf(temp_file_name + _tcslen(temp_file_name), TEXT("%s"), ext);

		return temp_file_name;
	}
	else 
		return log_file_name;
}

BOOL CLogFile::PutBinLog(BYTE *src, DWORD size)
{
	BOOL result = FALSE;

	EnterCriticalSection(log_cs);

	// last_date를 항상 현재 날짜, 시간으로 가져 오도록 갱신한다. by Lim
	GetLocalTime(&last_date);

	HANDLE hFile = openLogFile();
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD bytes;

		SetFilePointer(hFile, 0, NULL, FILE_END);
		if (WriteFile(hFile, src, size, &bytes, NULL)) {
			result = TRUE;
		}
		CloseHandle(hFile);
	}
	LeaveCriticalSection(log_cs);

	return result;
}

HANDLE CLogFile::openLogFile()
{
	int savedLastError = GetLastError();

	HANDLE hFile = CreateFile(GetLogFileName(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	// If the specified file exists before the function call and dwCreationDisposition is CREATE_ALWAYS or OPEN_ALWAYS, 
	// a call to GetLastError returns ERROR_ALREADY_EXISTS 
	// (even though the function has succeeded). 
	// If the file does not exist before the call, GetLastError returns zero.

	if (INVALID_HANDLE_VALUE != hFile) {
		SetLastError(savedLastError);
	}

	return hFile;
}