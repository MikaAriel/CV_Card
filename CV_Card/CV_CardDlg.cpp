
// CV_CardDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "CV_Card.h"
#include "CV_CardDlg.h"
#include "afxdialogex.h"
#include "vfw.h"
#include "ColorConv.h"

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

void Otzu_Threshold(unsigned char * orgImg, unsigned char *outImg, int height, int width)
{
	register int  i, t;

	//histogram set
	int hist[256];
	float prob[256];
	for (i = 0; i < 256; i++)
	{
		hist[i] = 0;
		prob[i] = 0.0f;
	}
	for (i = 0; i < height*width; i++)
		hist[(int)orgImg[i]]++;
	for (i = 0; i < 256; i++)
		prob[i] = (float)hist[i] / (float)(height*width);

	float wsv_min = 100000.0f;
	float wsv_u1, wsv_u2, wsv_s1, wsv_s2;
	int wsv_t;

	for (t = 0; t < 256; t++)
	{
		//q1,q2 계산
		float q1 = 0.0f, q2 = 0.0f;

		for (i = 0; i < t; i++)
			q1 += prob[i];
		for (i = t; i < 256; i++)
			q2 += prob[i];
		if (q1 == 0 || q2 == 0)
			continue;

		//u1, u2 계산
		float u1 = 0.0f, u2 = 0.0f;
		for (i = 0; i < t; i++)
			u1 += i*prob[i];
		u1 /= q1;
		for (i = t; i < 256; i++)
			u2 += i*prob[i];
		u2 /= q2;

		//s1,s2 계산
		float s1 = 0.0f, s2 = 0.0f;
		for (i = 0; i < t; i++)
			s1 += (i - u1)*(i - u1)*prob[i];
		s1 /= q1;
		for (i = t; i < 256; i++)
			s2 += (i - u2)*(i - u2)*prob[i];
		s2 /= q2;

		float wsv = q1*s1 + q2*s2;

		if (wsv < wsv_min)
		{
			wsv_min = wsv;
			wsv_t = t;
			wsv_u1 = u1;
			wsv_u2 = u2;
			wsv_s1 = s1;
			wsv_s2 = s2;
		}
	}

	for (i = 0; i < height*width; i++)
	{
		if (orgImg[i] < wsv_t)
			outImg[i] = 0;
		else
			outImg[i] = 255;
	}
}

VOID RGB2Gray(char *InImg, char *OutImg, int height, int width)
{
	int iCntX;
	int iCntY;
	int iJump;
	unsigned char r, g, b, gray;
	iJump = 0;
	for (iCntY = 0; height > iCntY; ++iCntY)
	{  // RGB값을 찾아서 변환해 주는 곳
		for (iCntX = 0; width > iCntX; ++iCntX, iJump += 3)
		{
			b = InImg[iJump];    // Blue
			g = InImg[iJump + 1];  // Green
			r = InImg[iJump + 2];  // Red

			gray = (r + b + g) / 3;

			OutImg[iJump] = gray;
			OutImg[iJump + 1] = gray;
			OutImg[iJump + 2] = gray;
		}
	}
}
VOID RGB2Binary(char *InImg, char *OutImg, int height, int width)
{
	int iCntX;
	int iCntY;
	int iJump;
	int i = 0;
	unsigned char* tempArray = new unsigned char[height * width];
	unsigned char* outArray = new unsigned char[height * width];
	iJump = 0;
	for (iCntY = 0; height > iCntY; ++iCntY)
	{  // RGB값을 찾아서 변환해 주는 곳
		for (iCntX = 0; width > iCntX; ++iCntX, iJump += 3)
		{
			tempArray[i] = InImg[iJump];
			i++;
		}
	}
	Otzu_Threshold(tempArray, outArray, height, width);
	i = 0;
	iJump = 0;
	for (iCntY = 0; height > iCntY; ++iCntY)
	{  // RGB값을 찾아서 변환해 주는 곳
		for (iCntX = 0; width > iCntX; ++iCntX, iJump += 3)
		{
			OutImg[iJump] = outArray[i];
			OutImg[iJump + 1] = outArray[i];
			OutImg[iJump + 2] = outArray[i];
			i++;
		}
	}
	/*for (iCntY = 0; height > iCntY; ++iCntY)
	{  // RGB값을 찾아서 변환해 주는 곳
		for (iCntX = 0; width > iCntX; ++iCntX, iJump += 3)
		{
			binary = (InImg[iJump] + InImg[iJump + 1] + InImg[iJump + 2]) / 3;
			if (binary < 128)
				binary = 0;
			else
				binary = 255;
			OutImg[iJump] = binary;
			OutImg[iJump + 1] = binary;
			OutImg[iJump + 2] = binary;
		}
	}*/
	delete[] tempArray;
	delete[] outArray;
}
// CCV_CardDlg 메시지 처리기
LRESULT CALLBACK VideoCallback(HWND hwnd, LPVIDEOHDR lpVHdr)
{
	char *p = (char *)(lpVHdr->lpData);
	char *pGray = new char[lpVHdr->dwBytesUsed];
	char *pBinary = new char[lpVHdr->dwBytesUsed];
	CDC *pdc = dp->GetDC(); //윈도우 DC를 얻음

	int a = lpVHdr->dwBytesUsed;

	BITMAPINFOHEADER *bmp = (BITMAPINFOHEADER *)bmp_color_head;
	
	StretchDIBits(
		pdc->m_hDC,				// hDC
		0,						// DestX
		0,						// DestY
		bmp->biWidth,			// nDestWidth
		bmp->biHeight,			// nDestHeight
		0,						// SrcX
		0,						// SrcY
		bmp->biWidth,			// SrcW
		bmp->biHeight,			// SrcH
		p,						// lpBits, 영상데이터
		(LPBITMAPINFO)bmp,		// lpBitsInfo
		DIB_RGB_COLORS,
		SRCCOPY);				// wUsage*/
	RGB2Gray(p, pGray, bmp->biHeight, bmp->biWidth);
	StretchDIBits(
		pdc->m_hDC,				// hDC
		320,					// DestX
		0,						// DestY
		bmp->biWidth,			// nDestWidth
		bmp->biHeight,			// nDestHeight
		0,						// SrcX
		0,						// SrcY
		bmp->biWidth,			// SrcW
		bmp->biHeight,			// SrcH
		pGray,					// lpBits, 영상데이터
		(LPBITMAPINFO)bmp,		// lpBitsInfo
		DIB_RGB_COLORS,
		SRCCOPY);				// wUsage*/
	RGB2Binary(pGray, pBinary, bmp->biHeight, bmp->biWidth);
	StretchDIBits(
		pdc->m_hDC,				// hDC
		0,						// DestX
		240,					// DestY
		bmp->biWidth,			// nDestWidth
		bmp->biHeight,			// nDestHeight
		0,						// SrcX
		0,						// SrcY
		bmp->biWidth,			// SrcW
		bmp->biHeight,			// SrcH
		pBinary,				// lpBits, 영상데이터
		(LPBITMAPINFO)bmp,		// lpBitsInfo
		DIB_RGB_COLORS,
		SRCCOPY);				// wUsage*/
	dp->ReleaseDC(pdc);

	delete[] pGray;
	delete[] pBinary;
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

	ShowWindow(SW_SHOW);

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
	bmp->biWidth = 320;
	bmp->biHeight = 240;
	capSetVideoFormat(d_cap, bmp, 40);
	capOverlay(d_cap, TRUE);
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