#pragma once

#include "LogFile.h"

namespace LogLib {

	enum LogType 
	{
		MIN_LOG_TYPE	= 0,
		LOG_DEBUG		= MIN_LOG_TYPE, 
		MAX_LOG_TYPE,
	};
};

#ifndef		__T
#define		__T(x)	L ## x
#define		_T(x)	__T(x)
#endif

#define		T__FUNCTION__	_T(__FUNCTION__)

#define		MAX_LOG_STRING				1000
#define		MAX_SERVERNAME_LENGTH		32

class LogGC;
class Log 
{
private:
	WCHAR			m_szLogDrive;
	CLogFile*		m_pDebugLog;
	CLogFile*		m_pAlarmLog;
	CLogFile*		m_pCommLog;

private:
	WCHAR			m_szServiceName[MAX_SERVERNAME_LENGTH];
	int				m_iServiceType;
	static Log*		m_pInstance;

public:
	friend class	LogGC;
	static Log*		Instance();

protected:
	Log();
	Log(Log &rhs) {};

	bool isDiskExist(WCHAR *directory);

public:
	~Log();

public:
	bool	InitLog();
	int		GetServerType();
	void	Debug(WCHAR *fmt, ...);
	void	Alarm(WCHAR *fmt, ...);
	void	Comm(WCHAR *fmt, ...);
	LPCWSTR		GetLogFileName(LPWSTR buffer, size_t size, LPCWSTR logName);
	CLogFile*	GetDebugLog();
	CLogFile*	GetAlarmLog();
	CLogFile*	GetCommLog();
	void InitDebugLog();
	void InitAlarmLog();
	void InitCommLog();
};

inline Log* GetLog()
{
	return Log::Instance();
}

inline LPCWSTR Log::GetLogFileName(LPWSTR buffer, size_t size, LPCWSTR logName)
{
	//_snwprintf(buffer, size, _T("./TCP_LOG/%s.txt"), logName);
	_snwprintf(buffer, size, _T("./LOG/%s.txt"), logName);
	return buffer;
}

inline int Log::GetServerType()
{
	return m_iServiceType;
}

inline CLogFile *Log::GetDebugLog()
{
	if (m_pDebugLog)
		return m_pDebugLog;

	m_pDebugLog = new CLogFile;
	if (m_pDebugLog) {
		InitDebugLog();
		m_pDebugLog->SetPrint();
		m_pDebugLog->SetTrace();
	}

	return m_pDebugLog;
}

inline CLogFile *Log::GetAlarmLog()
{
	if (m_pAlarmLog)
		return m_pAlarmLog;

	m_pAlarmLog = new CLogFile;
	if (m_pAlarmLog) {
		InitAlarmLog();
		m_pAlarmLog->SetPrint();
		m_pAlarmLog->SetTrace();
	}

	return m_pAlarmLog;
}

inline CLogFile *Log::GetCommLog()
{
	if (m_pCommLog)
		return m_pCommLog;

	m_pCommLog = new CLogFile;
	if (m_pCommLog) {
		InitCommLog();
		m_pCommLog->SetPrint();
		m_pCommLog->SetTrace();
	}

	return m_pAlarmLog;
}

inline void Log::InitDebugLog()
{
	WCHAR buffer[MAX_PATH];
	m_pDebugLog->SetLogFileCsv(
		//GetLog()->GetLogFileName(buffer, MAX_PATH, _T("log_debug")),
		GetLog()->GetLogFileName(buffer, MAX_PATH, _T("log_ZPL2img")),
		LOGFILE_PER_DAY, 
		LOG_SIZE, 
		LOG_EVERY_TIME);
}

inline void Log::InitAlarmLog()
{
	WCHAR buffer[MAX_PATH];
	m_pAlarmLog->SetLogFileCsv(
		GetLog()->GetLogFileName(buffer, MAX_PATH, _T("log_alarm")),
		LOGFILE_PER_DAY, 
		LOG_SIZE, 
		LOG_EVERY_TIME);
}

inline void Log::InitCommLog()
{
	WCHAR buffer[MAX_PATH];
	m_pCommLog->SetLogFileCsv(
		GetLog()->GetLogFileName(buffer, MAX_PATH, _T("log_comm")),
		LOGFILE_PER_DAY, 
		LOG_SIZE, 
		LOG_EVERY_TIME);
}

