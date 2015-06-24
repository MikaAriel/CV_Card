
// CV_CardDlg.h : 헤더 파일
//

#pragma once

#define MAX_WIDTH 320
#define MAX_HEIGHT 240

enum Type {TY_NONE = 0, SPADE = 1, DIAMOND, ClOVER, HEART,};
enum Number {NUM_NONE = 0, A = 1, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, J, Q, K, };

struct MinERR
{
	MinERR()
	{
		minAreaErr = 9999;
		minPerimErr = 9999;
		AreaErrWeight = 0.0f;
		PerimErrWeight = 0.0f;
		areaIndex = -1;
		perimIndex = -1;
		minRedErr = 9999;
		minGreenErr = 9999;
		minBlueErr = 9999;
		RedErrWeight = 0.0f;
		GreenErrWeight = 0.0f;
		BlueErrWeight = 0.0f;
		redIndex = -1;
		blueIndex = -1;
		greenIndex = -1;
	}
	int minAreaErr;
	int minPerimErr;
	int minRedErr;
	int minGreenErr;
	int minBlueErr;
	int redIndex;
	int greenIndex;
	int blueIndex;
	float PerimErrWeight;
	float AreaErrWeight;
	float RedErrWeight;
	float GreenErrWeight;
	float BlueErrWeight;
	int areaIndex;
	int perimIndex;
};
class Feature
{
public:
	Feature()
	{
		red, green, blue = 0;
		area = 0;
		perim = 0;
		x = 0;
		y = 0;
		shape = TY_NONE;
		number = NUM_NONE;
		startX = MAX_WIDTH;
		startY = MAX_HEIGHT;
		endX = 0;
		endY = 0;
	}
	virtual ~Feature()
	{
	}
public:
	int area;
	int perim;
	int red, green, blue;
	int x, y;
	int startX, startY, endX, endY;
	Type shape;
	Number number;
};

// CCV_CardDlg 대화 상자
class CCV_CardDlg : public CDialogEx
{
// 생성입니다.
public:
	CCV_CardDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_CV_CARD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL DestroyWindow();
};