// ConnectSocket.cpp : implementation file
//

#include "stdafx.h"
#include "ZLabelPreviewSave.h"
#include "ConnectSocket.h"
#include "ZLabelPreviewSaveDlg.h"
#include "Define.h" //2017-07-06
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
	//2017-07-06
	ShutDown();
	Close();
	CSocket::OnClose(nErrorCode);

	//2017-01-16
	 ((CZLabelPreviewSaveDlg*)AfxGetMainWnd())->m_bDMSconnected = FALSE; //2017-07-12 hot fix
	int nElapse = ((CZLabelPreviewSaveDlg*)AfxGetMainWnd())->m_nDMS_ConnectTerm * 1000;
	//AfxGetMainWnd()->SetTimer(IDD_ZLABELPREVIEWSAVE_DIALOG+1,nElapse, NULL); 
	AfxGetMainWnd()->SetTimer(TIMER_DMS_CONNECT,nElapse, NULL); //2017-07-06

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
		pMain->DisplayLogSocket(strLog);

		//if(strRcv != L"RESET" && strRcv.Left(4) != L"TIME" ) //2016-10-17
		if(strRcv != L"RESET" && strRcv.Left(4) != L"TIME" && strRcv != L"INITIALIZE" && strRcv != L"IMAGE_BREAK") //2017-01-23 //2017-08-07 IMAGE_BREAK 추가
		{
			m_strRcvZPL = strRcv;
		}
		else if(strRcv == L"RESET")
		{
			pMain->ResetByDMS(m_strRcvZPL);
			CSocket::OnReceive(nErrorCode);
			return;
		}
		else if(strRcv.Left(4) == L"TIME")
		{
			pMain->TimeSync(strRcv.Mid(8));
			CSocket::OnReceive(nErrorCode);
			return;
		}
		else if(strRcv == L"INITIALIZE") //2017-01-23  사용안함.(장비초기화시 사용할지 모르나, 가능성 희박)
		{
			pMain->Initialize();
			CSocket::OnReceive(nErrorCode);
			return;
		}
		else if(strRcv == L"IMAGE_BREAK") //2017-08-07
		{
			pMain->m_bImgBreak = TRUE;
			CSocket::OnReceive(nErrorCode);
			return;
		}

		pMain->m_bImgBreak = FALSE; //2017-08-07

		int nIdx = m_strRcvZPL.Find(L"^XA"); 
		int nIdx2 = m_strRcvZPL.Find(L"^XZ");
		if( nIdx >= 0 && nIdx2 > 0 ) // 2017-01-09
		{
			//2017-07-19 추가 - !!!
			pMain->m_nZplToDo = 1;
			//2017-01-18
			/*if(pMain->m_strZPL == L"")
			{
				pMain->m_strZPL = m_strRcvZPL;
			}
			else
			{
				CSocket::OnReceive(nErrorCode);
				return;
			}*/
			//V2.46 2017-09-19
			if(pMain->m_strZPL == L"")
			{
				pMain->m_strZPL = m_strRcvZPL;
			}
			else
			{
				pMain->LogPassZPL(m_strRcvZPL);
				CSocket::OnReceive(nErrorCode);
				return;
			}
			//

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
				strLog.Format(L"[SND-DMS] in void CConnectSocket::OnReceive(...) - missing ^XA or ^XZ %s", szTBuffer);
				GetLog()->Debug(strLog.GetBuffer(0));

				pMain->LogSend2DMS(szTBuffer);
				pMain->DisplayLogSocket(strLog);
			}
			else
			{
				strLog.Format(L"[ERROR][SND-DMS] - %s", szTBuffer);
				//pMain->LogSend2DMS(szTBuffer);
				GetLog()->Debug(strLog.GetBuffer(0));
				pMain->DisplayLogSocket(strLog);
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
	//2017-07-06
	ShutDown();
	Close();
	CSocket::OnClose(nErrorCode);
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
			pMain->DisplayLogSocket(strLog);
			pMain->ParseZEBRAResponse(Buff);
			pMain->Disconnect2ZEBRA(); //2017-07-14
			
		}
	}
	
	CSocket::OnReceive(nErrorCode);
}