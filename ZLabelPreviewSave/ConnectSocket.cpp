// ConnectSocket.cpp : implementation file
//

#include "stdafx.h"
#include "ZLabelPreviewSave.h"
#include "ConnectSocket.h"
#include "ZLabelPreviewSaveDlg.h"

// CConnectSocket

CConnectSocket::CConnectSocket()
{
}

CConnectSocket::~CConnectSocket()
{
}

// CConnectSocket member functions
void CConnectSocket::OnClose(int nErrorCode)
{
	((CZLabelPreviewSaveDlg*)AfxGetMainWnd())->Disconnect2DMS();
	AfxGetMainWnd()->SetTimer(IDD_ZLABELPREVIEWSAVE_DIALOG + 1, 1000 * 5, NULL); //DMS 접속
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
	
	if(Receive((BYTE*)szBuffer, sizeof(szBuffer)) > 200) //대충 200
	{
		pMain->MBCS2Unicode(szBuffer,szTBuffer);
		
		strRcv.Format(_T("%s"),szTBuffer);	
		strLog.Format(_T("[RCV]%s"),strRcv);
		GetLog()->Debug(strLog.GetBuffer());
		pMain->AddLogSocket(strLog);

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
				strLog.Format(L"[SND}%s",szTBuffer);
				GetLog()->Debug(strLog.GetBuffer());
				pMain->AddLogSocket(strLog);
			}
			else
			{
				strLog.Format(L"ERROR-[SND}%s",szTBuffer);
				GetLog()->Debug(strLog.GetBuffer());
				pMain->AddLogSocket(strLog);
			}
			pMain->PrepareNewZPL(); //2015-10-04 RETRY시 TAB order 초기화-테스트 要
		}
	}

	CSocket::OnReceive(nErrorCode);
}
