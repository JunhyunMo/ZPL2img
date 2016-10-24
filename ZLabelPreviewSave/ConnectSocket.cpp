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
	/*TCHAR szBuffer[1024];
	::ZeroMemory(szBuffer, sizeof(szBuffer));

	if(Receive(szBuffer, sizeof(szBuffer)) > 0)
	{
		CZLabelPreviewSaveDlg* pMain = (CZLabelPreviewSaveDlg*)AfxGetMainWnd();
		pMain->m_ctlListSocket.AddString(szBuffer);
		pMain->m_ctlListSocket.SetCurSel(pMain->m_ctlListSocket.GetCount() -1);
	}*/

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
	
	//if(Receive((BYTE*)szBuffer, sizeof(szBuffer)) > 200) //대충 200
	//if(Receive((BYTE*)szBuffer, sizeof(szBuffer)) > 30) //2016-07-14 빈 라벨 이미지^XA^LH0,0^FS\r\n^FO0,0^FD ^FS\r\n^XZ
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
		/*strLog.Format(_T("[RCV]%s"),strRcv);
		GetLog()->Debug(strLog.GetBuffer());
		pMain->AddLogSocket(strLog);*/


		int nIdx = strRcv.Find(L"^XA"); 
		if( nIdx >= 0) //
		{
			pMain->m_strZPL = strRcv;
			//Focus	
			pMain->SetForegroundWindow();
			pMain->GetDlgItem(IDC_EXPLORER)->SetFocus();
			//
			pMain->ProcessStart();
		}
		else if( nIdx == -1) //^XA 없으면...
		{
			::ZeroMemory(szBuffer, sizeof(szBuffer));
			strcpy_s(szBuffer,"RETRY");

			::ZeroMemory(szTBuffer, sizeof(szTBuffer));
			pMain->MBCS2Unicode(szBuffer,szTBuffer);

			if(pMain->m_Socket.Send((LPVOID)szBuffer, strlen(szBuffer) + 1) == TRUE)
			{
				strLog.Format(L"[SND-DMS] %s", szTBuffer);
				//GetLog()->Debug(strLog.GetBuffer());
				pMain->LogSend2DMS(szTBuffer);
				pMain->AddLogSocket(strLog);
			}
			else
			{
				strLog.Format(L"[ERROR][SND-DMS] - %s", szTBuffer);
				//GetLog()->Debug(strLog.GetBuffer());
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
		//pMain->TcpIpDisconn();
		//pMain->AddLog(L"Diconnect");
		//pMain->GetDlgItem(IDC_BT_TCPIP_CONN)->EnableWindow(TRUE);
	}
	else
	{
		/*pMain->TcpIpDisconn();
		pMain->AddLog(L"Diconnect");
		pMain->GetDlgItem(IDC_BT_TCPIP_CONN)->EnableWindow(TRUE);*/
	}
	//pMain->SetTimer(IDD_ZPL_SNDRCV_DIALOG,1000,NULL);
	
	CSocket::OnReceive(nErrorCode);

}