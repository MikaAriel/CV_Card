
// CV_Card.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CCV_CardApp:
// �� Ŭ������ ������ ���ؼ��� CV_Card.cpp�� �����Ͻʽÿ�.
//

class CCV_CardApp : public CWinApp
{
public:
	CCV_CardApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CCV_CardApp theApp;