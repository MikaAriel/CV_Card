
// CV_CardDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "CV_Card.h"
#include "CV_CardDlg.h"
#include "afxdialogex.h"
#include "vfw.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCV_CardDlg 대화 상자


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


// CCV_CardDlg 메시지 처리기
LRESULT CALLBACK VideoCallback(HWND hwnd, LPVIDEOHDR lpVHdr)
{
	char *p = (char *)(lpVHdr->lpData);


	CDC *pdc = dp->GetDC(); //윈도우 DC를 얻음


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
		p,                               // lpBits, 영상데이타
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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	ShowWindow(SW_MAXIMIZE);

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

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
		AfxMessageBox(L"사용가능한 영상 입력 장치를 찾지 못했습니다.", MB_ICONEXCLAMATION);

	// capture영상의 크기를 320x240으로 둔다.
	BITMAPINFOHEADER *bmp = (BITMAPINFOHEADER *)bmp_color_head;
	bmp->biWidth = 800;
	bmp->biHeight = 600;
	capSetVideoFormat(d_cap, bmp, 40);

	capSetCallbackOnVideoStream(d_cap, &VideoCallback);
	capSetCallbackOnFrame(d_cap, &VideoCallback);  // also use for single 

	// initiates streaming video capture without writing data to a file. 
	capCaptureSequenceNoFile(d_cap);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CCV_CardDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CCV_CardDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



BOOL CCV_CardDlg::DestroyWindow()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
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
