// ConnectSocket.cpp : implementation file
//

#include "stdafx.h"
#include "ZLabelPreviewSave.h"
#include "ConnectSocket.h"
#include "ZLabelPreviewSaveDlg.h"

// CConnectSocket

CConnectSocket::CConnectSocket()
{
	m_strRcvZPL = L"";
}

CConnectSocket::~CConnectSocket()
{
}

// CConnectSocket member functions
void CConnectSocket::OnClose(int nErrorCode)
{
	((CZLabelPreviewSaveDlg*)AfxGetMainWnd())->Disconnect2DMS();
	AfxGetMainWnd()->SetTimer(IDD_ZLABELPREVIEWSAVE_DIALOG+1, 1000*5, NULL); //DMS 접속
}

void CConnectSocket::OnReceive(int nErrorCode)
{
	CString strTmp = _T(""), strIP = _T("");
	UINT nPort = 0;
	
	CHAR	szBuffer[1024*10];
	::ZeroMemory(szBuffer, sizeof(szBuffer));

	TCHAR	szTBuffer[1024*10*2];
	::ZeroMemory(szTBuffer, sizeof(szTBuffer));
	
	GetPeerName(strIP, nPort);

	CString strRcv,strSend;
	CString strLog;

	CZLabelPreviewSaveDlg* pMain = (CZLabelPreviewSaveDlg*)AfxGetMainWnd();
	
	if(Receive((BYTE*)szBuffer, sizeof(szBuffer)) >= 5) //2016-10-10 "RESET" packet 처리
	{
		pMain->MBCS2Unicode(szBuffer,szTBuffer);
		
		//2016-10-21 정리
		strRcv.Format(_T("%s"),szTBuffer);		
		pMain->LogRcvDMS(strRcv);
		strLog.Format(L"[RCV-DMS] %s", szTBuffer);
		pMain->AddLogSocket(strLog);

		if(strRcv != L"RESET") //2016-10-17
		{
			m_strRcvZPL = strRcv;
		}
		else if(strRcv == L"RESET")
		{
			pMain->ResetByDMS(m_strRcvZPL);
			return;
		}


		int nIdx = m_strRcvZPL.Find(L"^XA"); 
		int nIdx2 = m_strRcvZPL.Find(L"^XZ");
		if( nIdx >= 0 && nIdx2 > 0 ) // 2017-01-09
		{
			pMain->m_strZPL = m_strRcvZPL;
		
			pMain->SetFocusOnWebCtrl(); //2016-10-26
			//
			pMain->ProcessStart();
		}
		else if( nIdx == -1 || nIdx2 == -1) //^XA or ^XZ없으면...2017-01-09
		{
			::ZeroMemory(szBuffer, sizeof(szBuffer));
			strcpy_s(szBuffer,"RETRY");

			::ZeroMemory(szTBuffer, sizeof(szTBuffer));
			pMain->MBCS2Unicode(szBuffer,szTBuffer);

			if(pMain->m_Socket.Send((LPVOID)szBuffer, strlen(szBuffer) + 1) == TRUE)
			{
				strLog.Format(L"[SND-DMS] %s", szTBuffer);
				pMain->LogSend2DMS(szTBuffer);
				pMain->AddLogSocket(strLog);
			}
			else
			{
				strLog.Format(L"[ERROR][SND-DMS] - %s", szTBuffer);
				pMain->LogSend2DMS(szTBuffer);
				pMain->AddLogSocket(strLog);
			}
			pMain->PrepareNewZPL(); //2015-10-04 RETRY시 TAB order 초기화-테스트 要
		}
	}

	CSocket::OnReceive(nErrorCode);
}


// CConnectSocket2 - ZEBRA
CConnectSocket2::CConnectSocket2()
{
}

CConnectSocket2::~CConnectSocket2()
{
}

// CConnectSocket member functions
void CConnectSocket2::OnClose(int nErrorCode)
{
	((CZLabelPreviewSaveDlg*)AfxGetMainWnd())->Disconnect2ZEBRA();
}

void CConnectSocket2::OnReceive(int nErrorCode)
{
	int nRead = 0;
	CString strLog;

	CHAR chBuff[1024];
	::ZeroMemory(chBuff, sizeof(chBuff));

	TCHAR Buff[1024*2];
	::ZeroMemory(Buff, sizeof(Buff));
	CZLabelPreviewSaveDlg* pMain = (CZLabelPreviewSaveDlg*)AfxGetMainWnd();

	nRead = Receive((BYTE*)chBuff, sizeof(chBuff));

	//if(nRead != SOCKET_ERROR)
	if(nRead >= 0)
	{
		if(strlen(chBuff) > 0) 
		{
			pMain->MBCS2Unicode(chBuff,Buff);
			strLog.Format(_T("[RCV-ZEBRA] %s"),Buff);
			pMain->AddLogSocket(strLog);
			pMain->ParseZEBRAResponse(Buff);
			
		}
	}
	
	CSocket::OnReceive(nErrorCode);
}