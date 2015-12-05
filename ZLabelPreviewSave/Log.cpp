#include "stdafx.h"
#include "Log.h"
#include "stdarg.h"

using namespace LogLib;

//////////////////////////////////////////////////////////////////////
// LogGC
class LogGC
{
public:
	LogGC() {}
	~LogGC() { delete Log::m_pInstance; Log::m_pInstance = NULL; }
};
LogGC s_GC;

Log *Log::m_pInstance = NULL;

Log::Log()
: m_iServiceType(0)
, m_pDebugLog(NULL)
, m_pAlarmLog(NULL)
, m_pCommLog(NULL)
{
}

Log::~Log()
{
	if(m_pDebugLog) { delete m_pDebugLog; m_pDebugLog = NULL; }
	if(m_pAlarmLog) { delete m_pAlarmLog; m_pAlarmLog = NULL; }
	if(m_pCommLog) { delete m_pCommLog; m_pCommLog = NULL; }
}

Log * Log::Instance()
{
	if (NULL == m_pInstance)
	{
		m_pInstance = new Log();
		if(m_pInstance) m_pInstance->InitLog();
	}

	return m_pInstance;
}

bool Log::InitLog()
{
	m_iServiceType	= 0;
    wcscpy(m_szServiceName, L"CLIENT_LOG");

	if (isDiskExist(_T("d:\\"))) m_szLogDrive = L'D';
	else m_szLogDrive = L'C';

	return true;
}

void Log::Debug(WCHAR* fmt, ...)
{
	WCHAR buffer[MAX_LOG_STRING]={0,};

	va_list args;
	va_start(args, fmt);
	int	len = _vsnwprintf(buffer, MAX_LOG_STRING,  fmt, args);
	buffer[MAX_LOG_STRING - 1] = L'\0';
	va_end(args);

	if (len > 0) GetDebugLog()->PutTimeLog(buffer);
}

void Log::Alarm(WCHAR* fmt, ...)
{
	WCHAR buffer[MAX_LOG_STRING]={0,};

	va_list args;
	va_start(args, fmt);
	int	len = _vsnwprintf(buffer, MAX_LOG_STRING,  fmt, args);
	buffer[MAX_LOG_STRING - 1] = L'\0';
	va_end(args);

	if (len > 0) GetAlarmLog()->PutTimeLog(buffer);
}

void Log::Comm(WCHAR* fmt, ...)
{
	WCHAR buffer[MAX_LOG_STRING]={0,};

	va_list args;
	va_start(args, fmt);
	int	len = _vsnwprintf(buffer, MAX_LOG_STRING,  fmt, args);
	buffer[MAX_LOG_STRING - 1] = L'\0';
	va_end(args);

	if (len > 0) GetCommLog()->PutTimeLog(buffer);
}

bool Log::isDiskExist(WCHAR *directory)
{
	ULARGE_INTEGER ulAvailable, ulTotal, ulFree; 
	WCHAR fullPath[MAX_PATH] = {0, };
	BOOL rtn;

	if (NULL != directory) 
	{
		wcsncpy(fullPath, directory, MAX_PATH);
		fullPath[MAX_PATH - 1] = '\0';
	}
	else
	{
		_ASSERT(FALSE);
		return false;
	}

	rtn = GetDiskFreeSpaceEx(fullPath, &ulAvailable, &ulTotal, &ulFree);
	if (rtn == FALSE) return false;
	if (ulFree.QuadPart == 0)
		return false;

	return true;
}

