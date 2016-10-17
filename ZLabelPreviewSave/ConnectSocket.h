#pragma once

// CConnectSocket command target

class CConnectSocket : public CSocket
{
public:
	CConnectSocket();
	virtual ~CConnectSocket();
	virtual void OnClose(int nErrorCode);
	virtual void OnReceive(int nErrorCode);

	CString	m_strRcvZPL; //2016-10-17
};


