#ifndef	_LOGFILE_H_
#define _LOGFILE_H_

#define LOG_SIZE				131072 			// 128 KB
#define MAX_LOG_SIZE			1024 * 1024		// 1 MB
#define LOG_ERROR_MSG_SIZE		2048
#define LOG_SPIN_COUNT			4000
#define LOG_EVERY_MINUTE		(60 * 1000)
#define LOG_EVERY_TIME			1
#define LOG_DONT_CARE			0

enum LogFileType {
	LOGFILE_ONE_FILE = 0,
	LOGFILE_PER_YEAR,
	LOGFILE_PER_MONTH,
	LOGFILE_PER_DAY,
	LOGFILE_PER_HOUR,
	LOGFILE_PER_TENMIN,
};

enum LogLevel {
	LL_NONE = 0,
	LL_ERROR,
	LL_DEBUG,
	LL_CALL,
	LL_ARGS,
	LL_ALL	 = LL_ARGS,
};

class CLogFile {
public:
	CLogFile();
	virtual ~CLogFile();

public:
	void SetPrint(bool fPrint = true);
	void SetTrace(bool fTrace = true);
	void SetSend(bool fSend = true);
	BOOL PutTimeLogV(LPCTSTR fmt, va_list args);
	//	Log의 각 라인의 앞에 시간을 남긴다. 각 라인 끝에 \n이 자동으로 붙는다.
	//		log_message 남길 message

	BOOL PutTimeLog(LPCTSTR fmt, ...);
	BOOL PutLevelLogV(LogLevel level, LPCTSTR fmt, va_list args);
	BOOL PutLevelLog(LogLevel level, LPCTSTR fmt, ...);
	void SetLogLevel(LogLevel level);
	BOOL PutErrorLog(LPCTSTR title, DWORD error_code, BOOL get_msg = FALSE, LPCTSTR msg = NULL);
	//  System Function의 Error를 기록한다.
	//		title		제목이다.
	//		error_code	GetLastError(), WSAGetLastError(), errno 등의 값이다.
	//		get_msg		FALSE가 아니면 system에서 Error Message를 가져온다.
	//		msg			추가로 기록할 message이다.

	BOOL PutLog(LPCTSTR log_message);
	//	Log를 남긴다.
	//	log_message:남길 message이다.

	///	Log file의 이름, 저장형태를 지정한다. 사용하기 전 반드시 한번은 불러야 한다.
	BOOL SetLogFile(
		LPCTSTR		filename,						///< Log file의 이름(full path)이다.
		LogFileType type	= LOGFILE_PER_DAY,		///< Log file을 얼마마다 새로 만들 것인지 지정한다.
		DWORD		size	= LOG_SIZE, 			///< memory buffer 크기. Byte 단위.
		DWORD		count	= LOG_DONT_CARE,		///< 이 횟수 이상 PutLog()가 불리면 disk에 저장한다. 0이면 무시.
		DWORD		time	= LOG_EVERY_MINUTE);	///< 최근 disk에 기록한지 이만한 시간이 흐른 후 PutLog()가 불리면 disk에 저장한다. Millisecond 단위이며, 0이면 무시.

	///	SetLogFile과 거의 동일. isCSV 변수만 추가.
	BOOL SetLogFileCsv(
		LPCTSTR		filename,						///< Log file의 이름(full path)이다.
		LogFileType type	= LOGFILE_PER_DAY,		///< Log file을 얼마마다 새로 만들 것인지 지정한다.
		DWORD		size	= LOG_SIZE, 			///< memory buffer 크기. Byte 단위.
		DWORD		count	= LOG_DONT_CARE,		///< 이 횟수 이상 PutLog()가 불리면 disk에 저장한다. 0이면 무시.
		DWORD		time	= LOG_EVERY_MINUTE);	///< 최근 disk에 기록한지 이만한 시간이 흐른 후 PutLog()가 불리면 disk에 저장한다. Millisecond 단위이며, 0이면 무시.

	BOOL PutBinLog(BYTE *src, DWORD size);

protected:
	bool				mPrint;
	bool				mTrace;
	bool				mSend;
	void				Send(LPCTSTR msg) {}
	int					mLevel;
	TCHAR				mDelim;
	DWORD				file_type;
	DWORD				max_log_size, max_log_count, max_log_time;
	DWORD				log_size, log_count, last_write_time;
	TCHAR				log_file_name[_MAX_PATH], temp_file_name[_MAX_PATH];
	LPTSTR				log_data;
	SYSTEMTIME			last_date;
	CRITICAL_SECTION	cs;

protected:
	LPCTSTR GetLogFileName(void);
	int		PrepareBuffer(LPCTSTR in_buf, int in_buf_len, LPSTR *out_buf);
	HANDLE	openLogFile();
	BOOL	logCountExpired();
	BOOL	logTimeExpired();
	BOOL	WriteLogFile(LPCTSTR buffer);
	BOOL	WriteLogData();
};

inline	BOOL CLogFile::PutLevelLogV(LogLevel level, LPCTSTR fmt, va_list args)
{
	if (level <= mLevel) {
		return PutTimeLogV(fmt, args);
	}
	return TRUE;
}

inline	BOOL CLogFile::PutLevelLog(LogLevel level, LPCTSTR fmt, ...)
{
	BOOL flag = FALSE;
	va_list args;

	va_start(args, fmt);

	flag = PutLevelLogV(level, fmt, args);

	va_end(args);

	return flag;
}

inline	void CLogFile::SetLogLevel(LogLevel level)
{
	mLevel = level;
}

inline BOOL CLogFile::SetLogFileCsv(LPCTSTR filename, LogFileType type, DWORD size, DWORD count, DWORD time)
{
	mDelim = TEXT(',');
	return SetLogFile(filename, type, size, count, time);
}

inline BOOL CLogFile::logCountExpired()
{
	return (max_log_count > 0 && log_count >= max_log_count);
}

inline BOOL CLogFile::logTimeExpired()
{
	return (max_log_time > 0 && GetTickCount() - last_write_time >= max_log_time);
}

inline void CLogFile::SetPrint(bool fPrint)
{
	mPrint = fPrint;
}

inline void CLogFile::SetTrace(bool fTrace)
{
	mTrace = fTrace;
}

inline void CLogFile::SetSend(bool fSend)
{
	mSend = fSend;
}


#endif // _LOGFILE_H_
