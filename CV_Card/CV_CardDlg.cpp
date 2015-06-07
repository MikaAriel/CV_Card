
// CV_CardDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "CV_Card.h"
#include "CV_CardDlg.h"
#include "afxdialogex.h"
#include "vfw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCV_CardDlg ��ȭ ����


unsigned char bmp_color_head[40] = {
	40, 0, 0, 0, 184, 0, 0, 0, 148, 0,
	0, 0, 1, 0, 24, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 18, 11, 0, 0, 18, 11,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

CWnd *dp;
HWND d_cap;

CCV_CardDlg::CCV_CardDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCV_CardDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCV_CardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCV_CardDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CCV_CardDlg �޽��� ó����
LRESULT CALLBACK VideoCallback(HWND hwnd, LPVIDEOHDR lpVHdr)
{
	char *p = (char *)(lpVHdr->lpData);


	CDC *pdc = dp->GetDC(); //������ DC�� ����


	BITMAPINFOHEADER *bmp = (BITMAPINFOHEADER *)bmp_color_head;

	StretchDIBits(
		pdc->m_hDC,              // hDC
		0,                                // DestX
		0,                                // DestY
		bmp->biWidth,             // nDestWidth
		bmp->biHeight,            // nDestHeight
		0,                                // SrcX
		0,                                // SrcY
		bmp->biWidth,             // SrcW
		bmp->biHeight,            // SrcH
		p,                               // lpBits, ������Ÿ
		(LPBITMAPINFO)bmp,  // lpBitsInfo
		DIB_RGB_COLORS,
		SRCCOPY);               // wUsage

	dp->ReleaseDC(pdc);

	return (LRESULT)TRUE;
}
//
BOOL CCV_CardDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	ShowWindow(SW_MAXIMIZE);

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.

	dp = this;

	d_cap = capCreateCaptureWindow(NULL,                        // No name 
		0, //WS_CHILD | WS_VISIBLE,          // no style. 
		0, 0, 150, 150,                                 // an arbitrary size 
		0,                                                  // no parent 
		0);                                                 // don't care about the id 

	CAPTUREPARMS cp;
	capCaptureGetSetup(d_cap, &cp, sizeof(cp)); // get the current defaults 

	// The default value is 66667, which corresponds to 15 frames per second
	//cp.dwRequestMicroSecPerFrame = m_dwMicroSecPerFrame; // Set desired frame rate 

	cp.fMakeUserHitOKToCapture = FALSE;
	cp.fYield = TRUE;          // we want capture on a background thread. 
	cp.wNumVideoRequested = 10;                // Maximum number of video buffers to allocate
	cp.fCaptureAudio = FALSE;
	cp.vKeyAbort = 0;                 // If no key is provided, 
	cp.fAbortLeftMouse = FALSE;
	cp.fAbortRightMouse = FALSE;
	cp.fLimitEnabled = FALSE;          // we want to stop 
	cp.fMCIControl = FALSE;

	capCaptureSetSetup(d_cap, &cp, sizeof(cp));

	char achDeviceName[80], achDeviceVersion[100];
	int dwRes, gwDeviceIndex, UseableDriver = 1;

	// Try to connect one of the MSVIDEO drivers
	for (int wIndex = 0; wIndex < 5; wIndex++) {
		if (capGetDriverDescription(wIndex,
			(LPTSTR)achDeviceName, sizeof(achDeviceName) / sizeof(TCHAR),
			(LPTSTR)achDeviceVersion, sizeof(achDeviceVersion) / sizeof(TCHAR))) {

			if (UseableDriver) {
				// The capDriverConnect macro connects a capture window to a capture driver
				dwRes = capDriverConnect(d_cap, wIndex);
				if (dwRes) {
					gwDeviceIndex = wIndex;
					UseableDriver = 0;
				}
			}
		} // end of if (capGetDriverDesc..())
	}

	if (UseableDriver)
		AfxMessageBox(L"��밡���� ���� �Է� ��ġ�� ã�� ���߽��ϴ�.", MB_ICONEXCLAMATION);

	// capture������ ũ�⸦ 320x240���� �д�.
	BITMAPINFOHEADER *bmp = (BITMAPINFOHEADER *)bmp_color_head;
	bmp->biWidth = 800;
	bmp->biHeight = 600;
	capSetVideoFormat(d_cap, bmp, 40);

	capSetCallbackOnVideoStream(d_cap, &VideoCallback);
	capSetCallbackOnFrame(d_cap, &VideoCallback);  // also use for single 

	// initiates streaming video capture without writing data to a file. 
	capCaptureSequenceNoFile(d_cap);

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CCV_CardDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CCV_CardDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



BOOL CCV_CardDlg::DestroyWindow()
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	capSetCallbackOnVideoStream(d_cap, NULL);
	if (VideoCallback) {
		FreeProcInstance(VideoCallback);
		//		VideoCallback = NULL; 
	}

	// Disconnect the current capture driver 
	capCaptureStop(d_cap);
	capDriverDisconnect(d_cap);
	::DestroyWindow(d_cap);
	return CDialogEx::DestroyWindow();
}
