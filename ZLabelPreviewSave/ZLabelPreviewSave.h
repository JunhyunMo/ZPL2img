
// ZLabelPreviewSave.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CZLabelPreviewSaveApp:
// �� Ŭ������ ������ ���ؼ��� ZLabelPreviewSave.cpp�� �����Ͻʽÿ�.
//

class CZLabelPreviewSaveApp : public CWinApp
{
public:
	CZLabelPreviewSaveApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CZLabelPreviewSaveApp theApp;