
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
#define LEARNINGCOUNT 100

#define MAXAREA 7500

#define LEARNING 0
#define LEARNCARD 0
#define READING 1

unsigned char bmp_color_head[40] = {
	40, 0, 0, 0, 184, 0, 0, 0, 148, 0,
	0, 0, 1, 0, 24, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 18, 11, 0, 0, 18, 11,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

CWnd *dp;
HWND d_cap;


unsigned char BinaryIMG[MAX_WIDTH * MAX_HEIGHT];
unsigned char FeatureMap[MAX_WIDTH * MAX_HEIGHT];
unsigned char RedMap[MAX_WIDTH * MAX_HEIGHT];
unsigned char BlueMap[MAX_WIDTH * MAX_HEIGHT];
unsigned char GreenMap[MAX_WIDTH * MAX_HEIGHT];

unsigned int CountFeature = 0;
Feature ObjectArray[255];

Feature LearnedObject[255];
Feature AccObject[255];


void ReadingCard(int index);

CCV_CardDlg::CCV_CardDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCV_CardDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	ZeroMemory(RedMap, sizeof(RedMap));
	ZeroMemory(BlueMap, sizeof(BlueMap));
	ZeroMemory(GreenMap, sizeof(GreenMap));
#if !LEARNING
	ReadingCard(READING);
#endif
}

void CCV_CardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCV_CardDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()

void OnMedianFilter(int height, int width)
{
	CString title;
	CString my_name;

	int mc = 0, mr = 0;
	int median[9];
	int temp;
	int heightm1 = height - 1;
	int widthm1 = width - 1;

	for (int i = 1; i < heightm1; i++)
	{
		for (int j = 1; j < widthm1; j++)
		{
			for (mc = 0; mc < 3; mc++)
			{
				for (mr = 0; mr < 3; mr++)
				{
					median[mc * 3 + mr] = BinaryIMG[((i + mc - 1) * width) + j + mr - 1];
				}
			}

			for (int k = 0; k < 9; k++)
			{
				for (int l = 0; l < 9; l++)
				{
					if (median[l] < median[l + 1])
					{
						temp = median[l];
						median[l] = median[l + 1];
						median[l + 1] = temp;
					}
				}
			}
			BinaryIMG[i * width + j] = median[4];
		}
	}
}

void OnErosion(int height, int width)
{
	CString title;
	CString my_name;

	int mr, mc;
	int newValue;
	int MaskBox[3][3] = 
	{ 
		{ 255, 0, 255 },
		{ 0, 0, 0 },
		{ 255, 0, 255 }
	};
	unsigned char Erosion[MAX_WIDTH * MAX_HEIGHT];
	ZeroMemory(Erosion, sizeof(Erosion));

	for (int i = 1; i < height - 1; i++)
	{
		for (int j = 1; j < width - 1; j++)
		{
			if (BinaryIMG[(i * width) + j] == 255)
			{
				int flag = 0;
				for (mr = 0; mr < 3; mr++)
				{
					for (mc = 0; mc < 3; mc++)
					{
						if (MaskBox[mr][mc] == 0 &&
							MaskBox[mr][mc] != BinaryIMG[((i + mr - 1) * width) + j + mc - 1])
							flag++;
					}
				}
				if (flag == 0)
					Erosion[(i*width) + j] = 0;
				else
					Erosion[(i*width) + j] = 255;
			}
		}
	}
	memcpy(BinaryIMG, Erosion, sizeof(Erosion));
}
void OnDilation(int height, int width)
{
	CString title;
	CString my_name;

	int mr, mc;
	int newValue;
	int MaskBox[3][3] =
	{
		{ 255, 0, 255 },
		{ 0, 0, 0 },
		{ 255, 0, 255 }
	};
	unsigned char Dilation[MAX_WIDTH * MAX_HEIGHT];
	ZeroMemory(Dilation, sizeof(Dilation));

	for (int i = 1; i < height - 1; i++)
	{
		for (int j = 1; j < width - 1; j++)
		{
			int flag = 0;
			for (mr = 0; mr < 3; mr++)
			{
				for (mc = 0; mc < 3; mc++)
				{
					if (MaskBox[mr][mc] == 0 &&
						MaskBox[mr][mc] == BinaryIMG[((i + mr - 1) * width) + j + mc - 1])
						flag++;
				}
			}
			if (flag != 0)
				Dilation[(i*width) + j] = 0;
			else
				Dilation[(i*width) + j] = 255;
		}
	}
	memcpy(BinaryIMG, Dilation, sizeof(Dilation));
}
void OnFeatureDivision(int height, int width)
{
	int i, j, x, y, val, index;
	int number = 0, featID = 1;
	int newVal, oldVal, nbr1, nbr2, nbr4, nbr8;
	memcpy(FeatureMap, BinaryIMG, sizeof(BinaryIMG));

	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			int Cur = y * width + x;
			int CurUp = (y - 1) * width + x;
			val = FeatureMap[Cur];
			index = 0;
			nbr1 = FeatureMap[CurUp + 1];	if (nbr1 != 0) index += 1;
			nbr2 = FeatureMap[CurUp];		if (nbr2 != 0) index += 2;
			nbr4 = FeatureMap[CurUp - 1];	if (nbr4 != 0) index += 4;
			nbr8 = FeatureMap[Cur - 1];		if (nbr8 != 0) index += 8;

			if (val != 0)
			{
				switch (index)
				{
				case 0:
					if (number < 255)
					{
						number++;
						newVal = featID;
						featID++;
					}
					break;
				case 1:
				case 3:
				case 7:
				case 11:
				case 15:
					newVal = nbr1;
					break;
				case 2:
				case 6:
				case 10:
				case 14:
					newVal = nbr2;
					break;
				case 4:
				case 12:
					newVal = nbr4;
					break;
				case 8:
					newVal = nbr8;
					break;
				default:
					newVal = nbr1;
					switch (index) {
					case 5: // 1 + 4
						oldVal = nbr4;
						break;
					case 9: // 1 + 8
						oldVal = nbr8;
						break;
					case 13: // 1 + 8 + 4
						oldVal = nbr8;
						break;
					}
					if (oldVal != newVal)
					{
						number--;
						for (i = 1; i < y; i++)
						{
							for (j = 1; j < width - 1; j++)
							{
								if (FeatureMap[i * width + j] == oldVal)
									FeatureMap[i * width + j] = newVal;
							}
						}
						for (j = 1; j < x; j++)
						{
							if (FeatureMap[y * width + j] == oldVal)
								FeatureMap[y * width + j] = newVal;
						}
					}
					break;
				}	//switch
				FeatureMap[Cur] = newVal;
			}	//if is feature
		}	//for x
	}
	CountFeature = number;
	char tempSort[255];
	int sortCount = 1;
	ZeroMemory(tempSort, 255);
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			int Cur = y * width + x;
			if (FeatureMap[Cur] > 0)
			{
				if (tempSort[FeatureMap[Cur]] == 0)
				{
					tempSort[FeatureMap[Cur]] = sortCount;
					sortCount++;
				}
				if (tempSort[FeatureMap[Cur]] != 0)
				{
					FeatureMap[Cur] = tempSort[FeatureMap[Cur]];
				}
			}
		}
	}
	/*FILE* pfile = NULL;
	pfile = fopen("arr.txt", "w");
	if (pfile == NULL)
		return;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			fprintf(pfile, "%d ", FeatureMap[i * width + j]);
		}
		fprintf(pfile, "\n");
	}
	fclose(pfile);*/
}
void OnSubFeatureDivision(int startX, int startY, int endX, int endY)
{
	int i, j, x, y, val, index;
	int number = 0, featID = 1;
	int newVal, oldVal, nbr1, nbr2, nbr4, nbr8;
	memcpy(FeatureMap, BinaryIMG, sizeof(BinaryIMG));

	for (y = startY; y < endY - 1; y++)
	{
		for (x = startX; x < endX - 1; x++)
		{
			int Cur = y * (endX - startX) + x;
			int CurUp = (y - 1) * (endX - startX) + x;
			val = FeatureMap[Cur];
			index = 0;
			nbr1 = FeatureMap[CurUp + 1];	if (nbr1 != 0) index += 1;
			nbr2 = FeatureMap[CurUp];		if (nbr2 != 0) index += 2;
			nbr4 = FeatureMap[CurUp - 1];	if (nbr4 != 0) index += 4;
			nbr8 = FeatureMap[Cur - 1];		if (nbr8 != 0) index += 8;

			if (val != 0)
			{
				switch (index)
				{
				case 0:
					if (number < 255)
					{
						number++;
						newVal = featID;
						featID++;
					}
					break;
				case 1:
				case 3:
				case 7:
				case 11:
				case 15:
					newVal = nbr1;
					break;
				case 2:
				case 6:
				case 10:
				case 14:
					newVal = nbr2;
					break;
				case 4:
				case 12:
					newVal = nbr4;
					break;
				case 8:
					newVal = nbr8;
					break;
				default:
					newVal = nbr1;
					switch (index) {
					case 5: // 1 + 4
						oldVal = nbr4;
						break;
					case 9: // 1 + 8
						oldVal = nbr8;
						break;
					case 13: // 1 + 8 + 4
						oldVal = nbr8;
						break;
					}
					if (oldVal != newVal)
					{
						number--;
						for (i = 1; i < y; i++)
						{
							for (j = 1; j < (endX - startX) - 1; j++)
							{
								if (FeatureMap[i * (endX - startX) + j] == oldVal)
									FeatureMap[i * (endX - startX) + j] = newVal;
							}
						}
						for (j = 1; j < x; j++)
						{
							if (FeatureMap[y * (endX - startX) + j] == oldVal)
								FeatureMap[y * (endX - startX) + j] = newVal;
						}
					}
					break;
				}	//switch
				FeatureMap[Cur] = newVal;
			}	//if is feature
		}	//for x
	}
	CountFeature = number;
	char tempSort[255];
	int sortCount = 1;
	ZeroMemory(tempSort, 255);
	for (y = startY; y < endY - 1; y++)
	{
		for (x = startX; x < endX - 1; x++)
		{
			int Cur = y * (endX - startX) + x;
			if (FeatureMap[Cur] > 0)
			{
				if (tempSort[FeatureMap[Cur]] == 0)
				{
					tempSort[FeatureMap[Cur]] = sortCount;
					sortCount++;
				}
				if (tempSort[FeatureMap[Cur]] != 0)
				{
					FeatureMap[Cur] = tempSort[FeatureMap[Cur]];
				}
			}
		}
	}
	/*FILE* pfile = NULL;
	pfile = fopen("arr.txt", "w");
	if (pfile == NULL)
	return;
	for (int i = 0; i < height; i++)
	{
	for (int j = 0; j < width; j++)
	{
	fprintf(pfile, "%d ", FeatureMap[i * width + j]);
	}
	fprintf(pfile, "\n");
	}
	fclose(pfile);*/
}
void OnAreaFraction(int height, int width)
{
	ZeroMemory(ObjectArray, sizeof(ObjectArray));
	for (int i = 0; i < CountFeature; i++)
	{
		ObjectArray[i].startX = MAX_WIDTH;
		ObjectArray[i].startY = MAX_HEIGHT;
	}
	int nbr1 = 0, nbr2 = 0, nbr4 = 0, nbr8 = 0;

	for (int i = 1; i < height - 1; i++)
	{
		for (int j = 1; j < width - 1; j++)
		{
			int Cur = i * width + j;
			int CurUp = (i - 1) * width + j;
			int val = FeatureMap[Cur];
			int index = 0;

			nbr8 = FeatureMap[Cur - 1];		if (nbr8 > 0 && nbr8 < CountFeature) index += 8;
			nbr4 = FeatureMap[CurUp - 1];	if (nbr4 > 0 && nbr8 < CountFeature) index += 4;
			nbr2 = FeatureMap[CurUp];		if (nbr2 > 0 && nbr8 < CountFeature) index += 2;
			nbr1 = FeatureMap[CurUp + 1];	if (nbr1 > 0 && nbr8 < CountFeature) index += 1;

			if (val > 0 )
			{
				ObjectArray[val - 1].area++;
				ObjectArray[val - 1].x += j;
				ObjectArray[val - 1].y += i;
				if (ObjectArray[val - 1].startX > j)
					ObjectArray[val - 1].startX = j;
				if (ObjectArray[val - 1].endX < j)
					ObjectArray[val - 1].endX = j;

				if (ObjectArray[val - 1].startY > i)
					ObjectArray[val - 1].startY = i;
				if (ObjectArray[val - 1].endY < i)
					ObjectArray[val - 1].endY = i;

				if (nbr8 < val)
				{
					ObjectArray[val - 1].perim++;
				}
				if (RedMap[Cur] == 255)
					ObjectArray[val - 1].red++;
				if (GreenMap[Cur] == 255)
					ObjectArray[val - 1].green++;
				if (BlueMap[Cur] == 255)
					ObjectArray[val - 1].blue++;
			}
			else
			{
				if (nbr8 > val)
				{
					ObjectArray[val - 1].perim++;
				}
			}
		}
	}
	for (int i = 0; i < CountFeature; i++)
	{
		if (ObjectArray[i].area != 0 && ObjectArray[i].perim != 0)
		{
			ObjectArray[i].perim *= 1.5708;
			ObjectArray[i].x /= ObjectArray[i].area;
			ObjectArray[i].y /= ObjectArray[i].area;
			ObjectArray[i].y = height - ObjectArray[i].y;

			ObjectArray[i].startY = height - ObjectArray[i].startY;
			ObjectArray[i].endY = height - ObjectArray[i].endY;

			if (ObjectArray[i].red > 50)
				ObjectArray[i].red = MAXAREA - ObjectArray[i].area;
			if (ObjectArray[i].green > 50)
				ObjectArray[i].green = MAXAREA - ObjectArray[i].area;
			if (ObjectArray[i].blue > 50)
				ObjectArray[i].blue = MAXAREA - ObjectArray[i].area;
		}
	}
	/*int nbr1 = 0, nbr2 = 0, nbr4 = 0, nbr8 = 0;

	for (int i = 1; i < height - 1; i++)
	{
		for (int j = 1; j < width - 1; j++)
		{
			int Cur = i * width + j;
			int CurUp = (i - 1) * width + j;
			int val = FeatureMap[Cur];
			int index = 0;

			nbr8 = FeatureMap[Cur - 1];		if (nbr8 > 0 && nbr8 < CountFeature) index += 8;
			nbr4 = FeatureMap[CurUp - 1];	if (nbr4 > 0 && nbr8 < CountFeature) index += 4;
			nbr2 = FeatureMap[CurUp];		if (nbr2 > 0 && nbr8 < CountFeature) index += 2;
			nbr1 = FeatureMap[CurUp + 1];	if (nbr1 > 0 && nbr8 < CountFeature) index += 1;

			if (val > 0.5f)
				area++;
			if (((val > 0.5f) && (nbr8 < 0.5f) || (val < 0.5f) && (nbr8 > 0.5f)))
				icept++;
		}
	}*/
}
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
	
	/*for (int i = 0; i < height*width; i++)
	{
		if (orgImg[i] < 100)
			outImg[i] = 0;
		else
			outImg[i] = 255;
	}*/
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

			if ((b + g + r) > 220)
			{
				if (r <= 200 && g <= 200 && b > 200)
				{
					BlueMap[(iCntY * width) + iCntX] = 255;
				}
				if (b <= 200 && r <= 200 && g > 200)
				{
					GreenMap[(iCntY * width) + iCntX] = 255;
				}
				if (b <= 200 && g <= 200 && r >= 200)
				{
					RedMap[(iCntY * width) + iCntX] = 255;
				}
			}
			gray = (int)(r * 0.30 + g * 0.59 + b * 0.11);
			//gray = (r + b + g) / 3;

			OutImg[iJump] = gray;
			OutImg[iJump + 1] = gray;
			OutImg[iJump + 2] = gray;
		}
	}
	static int k = 0;
	if (k == 5)
	{
		int count = 0;
		FILE* pfile = NULL;
		pfile = fopen("red.txt", "w");
		if (pfile == NULL)
			return;
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				fprintf(pfile, "%d ",RedMap[i * width + j]);
			}
			fprintf(pfile, "\n");
		}
		fclose(pfile);
	}
	k++;
	/*static int k = 0;
	if (k == 5)
	{
		int count = 0;
		FILE* pfile = NULL;
		pfile = fopen("red.txt", "w");
		if (pfile == NULL)
			return;
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				if (RedMap[i * width + j] == 255)
					count++;
			}
		}
		fprintf(pfile, "%d\n", count);
		fclose(pfile);
	}
	k++;*/
}
VOID RGB2Binary(char *InImg, char *OutImg, int height, int width)
{
	int iCntX;
	int iCntY;
	int iJump;
	int i = 0;

	unsigned char* tempArray = new unsigned char[height * width];

	iJump = 0;
	for (iCntY = 0; height > iCntY; ++iCntY)
	{  // RGB값을 찾아서 변환해 주는 곳
		for (iCntX = 0; width > iCntX; ++iCntX, iJump += 3)
		{
			tempArray[i] = InImg[iJump];
			i++;
		}
	}
	Otzu_Threshold(tempArray, BinaryIMG, height, width);

	//
	//OnErosion(height, width);
	//OnDilation(height, width);
	//OnErosion(height, width);
	//OnMedianFilter(height, width);
	//OnErosion(height, width);
	//OnDilation(height, width);
	//OnDilation(height, width);
	//

	i = 0;
	iJump = 0;
	for (iCntY = 0; height > iCntY; ++iCntY)
	{  // RGB값을 찾아서 변환해 주는 곳
		for (iCntX = 0; width > iCntX; ++iCntX, iJump += 3)
		{
			OutImg[iJump] = BinaryIMG[i];
			OutImg[iJump + 1] = BinaryIMG[i];
			OutImg[iJump + 2] = BinaryIMG[i];
			i++;
		}
	}
	delete[] tempArray;
}
void LearningCard(int index)
{
	static int accCount = 1;

	if (accCount > 10)
	{
		AccObject[index].area += ObjectArray[0].area;
		AccObject[index].perim += ObjectArray[0].perim;
		for (int i = 0; i < MAX_WIDTH * MAX_HEIGHT; i++)
		{
			if (RedMap[i] == 255)
				AccObject[index].red++;
			if (GreenMap[i] == 255)
				AccObject[index].green++;
			if (BlueMap[i] == 255)
				AccObject[index].blue++;
		}
	}
	if (accCount == LEARNINGCOUNT+10)
	{
		switch (index)
		{
		case 0:
			AccObject[index].shape = DIAMOND;
			AccObject[index].number = TWO;
			break;
		case 1:
			AccObject[index].shape = DIAMOND;
			AccObject[index].number = THREE;
			break;
		case 2:
			AccObject[index].shape = DIAMOND;
			AccObject[index].number = FIVE;
			break;
		case 3:
			AccObject[index].shape = DIAMOND;
			AccObject[index].number = SEVEN;
			break;
		case 4:
			AccObject[index].shape = ClOVER;
			AccObject[index].number = THREE;
			break;
		case 5:
			AccObject[index].shape = ClOVER;
			AccObject[index].number = FIVE;
			break;
		case 6:
			AccObject[index].shape = ClOVER;
			AccObject[index].number = SEVEN;
			break;
		case 7:
			AccObject[index].shape = HEART;
			AccObject[index].number = SEVEN;
			break;
		case 8:
			AccObject[index].shape = SPADE;
			AccObject[index].number = TWO;
			break;
		case 9:
			AccObject[index].shape = SPADE;
			AccObject[index].number = FIVE;
			break;
		}
		AccObject[index].area /= accCount - 10;
		AccObject[index].perim /= accCount - 10;
		AccObject[index].red /= accCount - 10;
		AccObject[index].green /= accCount - 10;
		AccObject[index].blue /= accCount - 10;
		FILE* pFile = NULL;
		pFile = fopen("LearningData", "ab");
		if (pFile == NULL)
			return;
		fseek(pFile, 0, SEEK_END);
		fwrite(&AccObject[index], 1, sizeof(Feature), pFile);
		fclose(pFile);
		AfxMessageBox(L"학습 완료!");
	}
	accCount++;
}
void ReadingCard(int index)
{
	FILE* pFile = NULL;
	pFile = fopen("LearningData", "rb");
	if (pFile == NULL)
		return;
	fread(LearnedObject, 1, sizeof(Feature) * index , pFile);
	fclose(pFile);
}
// CCV_CardDlg 메시지 처리기
LRESULT CALLBACK VideoCallback(HWND hwnd, LPVIDEOHDR lpVHdr)
{
	static bool RunCam = true;
	char *p = (char *)(lpVHdr->lpData);
	char *pGray = new char[lpVHdr->dwBytesUsed];
	char *pBinary = new char[lpVHdr->dwBytesUsed];
	CDC *pdc = dp->GetDC(); //윈도우 DC를 얻음

	HPEN MyPen, OldPen;
	TCHAR result[256];
	TCHAR cardType[16];
	TCHAR cardNum[16];

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
	RGB2Binary(pGray, pBinary, bmp->biHeight, bmp->biWidth);

	OnFeatureDivision(bmp->biHeight, bmp->biWidth);
	StretchDIBits(
		pdc->m_hDC,				// hDC
		MAX_WIDTH,				// DestX
		0,					// DestY
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

	OnAreaFraction(bmp->biHeight, bmp->biWidth);

	for (int i = 0; i < CountFeature; i++)
	{
		MinERR err;
		for (int j = 0; j < LEARNCARD + 1; j++)
		{
			if (LearnedObject[j].area != 0 && LearnedObject[j].perim != 0)
			{
				int areaErr = abs(LearnedObject[j].area - ObjectArray[i].area);
				int perimErr = abs(LearnedObject[j].perim - ObjectArray[i].perim);
				int redErr = abs(LearnedObject[j].red - ObjectArray[i].red);
				/*if (perimErr == 0)
				{
					err.perimIndex = j;
				}
				if (areaErr == 0)
				{
					err.areaIndex = j;
				}*/
				if (redErr != 0 && err.minRedErr > redErr)
				{
					err.minRedErr = redErr;
					err.redIndex = j;
				}
				if (areaErr != 0 && err.minAreaErr > areaErr)
				{
					err.AreaErrWeight = LearnedObject[j].area / areaErr;
					err.minAreaErr = areaErr;
					err.areaIndex = j;
				}
				if (perimErr != 0 && err.minPerimErr > perimErr)
				{
					err.PerimErrWeight = LearnedObject[j].perim / perimErr;
					err.minPerimErr = perimErr;
					err.perimIndex = j;
				}
			}
			if ((err.PerimErrWeight * 10) < err.AreaErrWeight)
			{
				if (ObjectArray[i].red > 250 || LearnedObject[err.areaIndex].red > 250)
				{
					ObjectArray[i].shape = LearnedObject[err.redIndex].shape;
					ObjectArray[i].number = LearnedObject[err.redIndex].number;
				}
				else
				{
					ObjectArray[i].shape = LearnedObject[err.areaIndex].shape;
					ObjectArray[i].number = LearnedObject[err.areaIndex].number;
				}
			}
			else
			{
				if (ObjectArray[i].red > 250 || LearnedObject[err.perimIndex].red > 250)
				{
					ObjectArray[i].shape = LearnedObject[err.redIndex].shape;
					ObjectArray[i].number = LearnedObject[err.redIndex].number;
				}
				else
				{
					ObjectArray[i].shape = LearnedObject[err.perimIndex].shape;
					ObjectArray[i].number = LearnedObject[err.perimIndex].number;
				}
			}

			switch (ObjectArray[i].shape)
			{
			case TY_NONE:
				lstrcpy(cardType, L"None");
				break;
			case SPADE:
				lstrcpy(cardType, L"♠");
				break;
			case DIAMOND:
				lstrcpy(cardType, L"◆");
				break;
			case ClOVER:
				lstrcpy(cardType, L"♣");
				break;
			case HEART:
				lstrcpy(cardType, L"♥");
				break;
			}
			switch (ObjectArray[i].number)
			{
			case NUM_NONE:
				lstrcpy(cardNum, L"None");
				break;
			case A:
				lstrcpy(cardNum, L"A");
				break;
			case TWO:
				lstrcpy(cardNum, L"2");
				break;
			case THREE:
				lstrcpy(cardNum, L"3");
				break;
			case FOUR:
				lstrcpy(cardNum, L"4");
				break;
			case FIVE:
				lstrcpy(cardNum, L"5");
				break;
			case SIX:
				lstrcpy(cardNum, L"6");
				break;
			case SEVEN:
				lstrcpy(cardNum, L"7");
				break;
			case EIGHT:
				lstrcpy(cardNum, L"8");
				break;
			case NINE:
				lstrcpy(cardNum, L"9");
				break;
			case TEN:
				lstrcpy(cardNum, L"10");
				break;
			case J:
				lstrcpy(cardNum, L"J");
				break;
			case Q:
				lstrcpy(cardNum, L"Q");
				break;
			case K:
				lstrcpy(cardNum, L"K");
				break;
			}
			MyPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
			OldPen = (HPEN)SelectObject(pdc->m_hDC, MyPen);
			Ellipse(pdc->m_hDC, ObjectArray[i].x - 3, ObjectArray[i].y - 3, ObjectArray[i].x + 3, ObjectArray[i].y + 3);
			Ellipse(pdc->m_hDC, ObjectArray[i].startX - 3, ObjectArray[i].startY - 3, ObjectArray[i].startX + 3, ObjectArray[i].startY + 3);
			Ellipse(pdc->m_hDC, ObjectArray[i].endX - 3, ObjectArray[i].endY - 3, ObjectArray[i].endX + 3, ObjectArray[i].endY + 3);
			SelectObject(pdc->m_hDC, OldPen);
			DeleteObject(MyPen);
			swprintf(result, L"[%d]%s%s  ",i,cardType, cardNum);
			TextOut(pdc->m_hDC, ObjectArray[i].x, ObjectArray[i].y, result, _tcslen(result));
			swprintf(result, L"ID : %d, Area : %d, Perim : %d, Red : %d ,CardType : %s %s, Feature : %d       ", i, ObjectArray[i].area, ObjectArray[i].perim, ObjectArray[i].red, cardType, cardNum, CountFeature);
			TextOut(pdc->m_hDC, MAX_WIDTH, MAX_HEIGHT + (i * 20), result, _tcslen(result));
		}
	}
	for (int i = 0; i < 10; i++)
	{
		switch (LearnedObject[i].shape)
		{
		case TY_NONE:
			lstrcpy(cardType, L"None");
			break;
		case SPADE:
			lstrcpy(cardType, L"♠");
			break;
		case DIAMOND:
			lstrcpy(cardType, L"◆");
			break;
		case ClOVER:
			lstrcpy(cardType, L"♣");
			break;
		case HEART:
			lstrcpy(cardType, L"♥");
			break;
		}
		switch (LearnedObject[i].number)
		{
		case NUM_NONE:
			lstrcpy(cardNum, L"None");
			break;
		case A:
			lstrcpy(cardNum, L"A");
			break;
		case TWO:
			lstrcpy(cardNum, L"2");
			break;
		case THREE:
			lstrcpy(cardNum, L"3");
			break;
		case FOUR:
			lstrcpy(cardNum, L"4");
			break;
		case FIVE:
			lstrcpy(cardNum, L"5");
			break;
		case SIX:
			lstrcpy(cardNum, L"6");
			break;
		case SEVEN:
			lstrcpy(cardNum, L"7");
			break;
		case EIGHT:
			lstrcpy(cardNum, L"8");
			break;
		case NINE:
			lstrcpy(cardNum, L"9");
			break;
		case TEN:
			lstrcpy(cardNum, L"10");
			break;
		case J:
			lstrcpy(cardNum, L"J");
			break;
		case Q:
			lstrcpy(cardNum, L"Q");
			break;
		case K:
			lstrcpy(cardNum, L"K");
			break;
		}
		swprintf(result, L"Area : %d, Perim : %d, Red : %d %s%s    ", LearnedObject[i].area, LearnedObject[i].perim, LearnedObject[i].red, cardType, cardNum);
		TextOut(pdc->m_hDC, 0, MAX_HEIGHT + (i * 20), result, _tcslen(result));
	}
	/*TextOut(pdc->m_hDC, 0, 0, L"Color", 5);
	TextOut(pdc->m_hDC, 320, 0, L"GrayScale", 9);
	TextOut(pdc->m_hDC, 0, 240, L"Binary", 6);*/

	//
#if LEARNING
	LearningCard(LEARNCARD);
#endif
	//
	dp->ReleaseDC(pdc);

	ZeroMemory(RedMap, sizeof(RedMap));
	ZeroMemory(BlueMap, sizeof(BlueMap));
	ZeroMemory(GreenMap, sizeof(GreenMap));
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
	bmp->biWidth = MAX_WIDTH;
	bmp->biHeight = MAX_HEIGHT;
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