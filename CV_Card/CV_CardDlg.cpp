
// CV_CardDlg.cpp : ���� ����
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
		//q1,q2 ���
		float q1 = 0.0f, q2 = 0.0f;

		for (i = 0; i < t; i++)
			q1 += prob[i];
		for (i = t; i < 256; i++)
			q2 += prob[i];
		if (q1 == 0 || q2 == 0)
			continue;

		//u1, u2 ���
		float u1 = 0.0f, u2 = 0.0f;
		for (i = 0; i < t; i++)
			u1 += i*prob[i];
		u1 /= q1;
		for (i = t; i < 256; i++)
			u2 += i*prob[i];
		u2 /= q2;

		//s1,s2 ���
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
	{  // RGB���� ã�Ƽ� ��ȯ�� �ִ� ��
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
	{  // RGB���� ã�Ƽ� ��ȯ�� �ִ� ��
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
	{  // RGB���� ã�Ƽ� ��ȯ�� �ִ� ��
		for (iCntX = 0; width > iCntX; ++iCntX, iJump += 3)
		{
			OutImg[iJump] = outArray[i];
			OutImg[iJump + 1] = outArray[i];
			OutImg[iJump + 2] = outArray[i];
			i++;
		}
	}
	/*for (iCntY = 0; height > iCntY; ++iCntY)
	{  // RGB���� ã�Ƽ� ��ȯ�� �ִ� ��
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
// CCV_CardDlg �޽��� ó����
LRESULT CALLBACK VideoCallback(HWND hwnd, LPVIDEOHDR lpVHdr)
{
	char *p = (char *)(lpVHdr->lpData);
	char *pGray = new char[lpVHdr->dwBytesUsed];
	char *pBinary = new char[lpVHdr->dwBytesUsed];
	CDC *pdc = dp->GetDC(); //������ DC�� ����

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
		p,						// lpBits, ��������
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
		pGray,					// lpBits, ��������
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
		pBinary,				// lpBits, ��������
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

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	ShowWindow(SW_SHOW);

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
	bmp->biWidth = 320;
	bmp->biHeight = 240;
	capSetVideoFormat(d_cap, bmp, 40);
	capOverlay(d_cap, TRUE);
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