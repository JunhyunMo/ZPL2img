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
	//	Log�� �� ������ �տ� �ð��� �����. �� ���� ���� \n�� �ڵ����� �ٴ´�.
	//		log_message ���� message

	BOOL PutTimeLog(LPCTSTR fmt, ...);
	BOOL PutLevelLogV(LogLevel level, LPCTSTR fmt, va_list args);
	BOOL PutLevelLog(LogLevel level, LPCTSTR fmt, ...);
	void SetLogLevel(LogLevel level);
	BOOL PutErrorLog(LPCTSTR title, DWORD error_code, BOOL get_msg = FALSE, LPCTSTR msg = NULL);
	//  System Function�� Error�� ����Ѵ�.
	//		title		�����̴�.
	//		error_code	GetLastError(), WSAGetLastError(), errno ���� ���̴�.
	//		get_msg		FALSE�� �ƴϸ� system���� Error Message�� �����´�.
	//		msg			�߰��� ����� message�̴�.

	BOOL PutLog(LPCTSTR log_message);
	//	Log�� �����.
	//	log_message:���� message�̴�.

	///	Log file�� �̸�, �������¸� �����Ѵ�. ����ϱ� �� �ݵ�� �ѹ��� �ҷ��� �Ѵ�.
	BOOL SetLogFile(
		LPCTSTR		filename,						///< Log file�� �̸�(full path)�̴�.
		LogFileType type	= LOGFILE_PER_DAY,		///< Log file�� �󸶸��� ���� ���� ������ �����Ѵ�.
		DWORD		size	= LOG_SIZE, 			///< memory buffer ũ��. Byte ����.
		DWORD		count	= LOG_DONT_CARE,		///< �� Ƚ�� �̻� PutLog()�� �Ҹ��� disk�� �����Ѵ�. 0�̸� ����.
		DWORD		time	= LOG_EVERY_MINUTE);	///< �ֱ� disk�� ������� �̸��� �ð��� �帥 �� PutLog()�� �Ҹ��� disk�� �����Ѵ�. Millisecond �����̸�, 0�̸� ����.

	///	SetLogFile�� ���� ����. isCSV ������ �߰�.
	BOOL SetLogFileCsv(
		LPCTSTR		filename,						///< Log file�� �̸�(full path)�̴�.
		LogFileType type	= LOGFILE_PER_DAY,		///< Log file�� �󸶸��� ���� ���� ������ �����Ѵ�.
		DWORD		size	= LOG_SIZE, 			///< memory buffer ũ��. Byte ����.
		DWORD		count	= LOG_DONT_CARE,		///< �� Ƚ�� �̻� PutLog()�� �Ҹ��� disk�� �����Ѵ�. 0�̸� ����.
		DWORD		time	= LOG_EVERY_MINUTE);	///< �ֱ� disk�� ������� �̸��� �ð��� �帥 �� PutLog()�� �Ҹ��� disk�� �����Ѵ�. Millisecond �����̸�, 0�̸� ����.

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
