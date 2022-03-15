// UVCopencvdirectshowDrawImage.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "atlstr.h"
#include <windows.h>
#include "UVCopencvdirectshowDrawImage.h"
#include "vfw.h"
#include <opencv2/opencv.hpp>
#include "camerads.h"
#include <stdio.h>
#include <iostream>  
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <fstream>
#include "CRC16Calc.h"
#include <errno.h>

#pragma comment(lib,"vfw32.lib")

using namespace cv;
using namespace std;

#define MAX_LOADSTRING 100

// ȫ�ֱ���: 
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

CCameraDS camera;
int UVCdevice = 0;	//0����ʼǱ��Դ�����ͷ��1��������USB����ͷ

void save_img(Mat frame, IplImage *pFrame);
int write_csv(char *file_name, float *src, char* abs_path);

int m_iTargetDistance;
void DrawImage(float* Depth, int iWidth, int iHeight, HWND hWnd);
void GetColor(int num, int *color);
DWORD WINAPI ThreadProc(LPVOID lpParam);

// �˴���ģ���а����ĺ�����ǰ������: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, HWND *hWnd);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO:  �ڴ˷��ô��롣
	MSG msg;
	HACCEL hAccelTable;
	HWND hWnd = NULL;

	// ��ʼ��ȫ���ַ���
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_UVCOPENCVDIRECTSHOWDRAWIMAGE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ִ��Ӧ�ó����ʼ��: 
	if (!InitInstance(hInstance, nCmdShow, &hWnd))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_UVCOPENCVDIRECTSHOWDRAWIMAGE));

	if (hWnd == NULL)
	{
		MessageBox(NULL,(LPCWSTR)L"hWnd == NULL",(LPCWSTR)L"Account Details",MB_DEFBUTTON2);
	}

	int cx = 10;
	CString s;
    #if 0
	s.Format(_T("�����ǣ�%d"), cx);
	MessageBox(NULL, (LPCWSTR)s, (LPCWSTR)L"Account Details", MB_DEFBUTTON2);
    #endif

	//������ȡ����ͷ��Ŀ
	int cam_count;
	cam_count = CCameraDS::CameraCount();
	s.Format(_T("There are %d cameras.\n"), cam_count);
	MessageBox(NULL, (LPCWSTR)s, (LPCWSTR)L"cam_counts", MB_DEFBUTTON2);

	//��ȡ��������ͷ������
	for (int i = 0; i < cam_count; i++)
	{
		char camera_name[1024] = "fuck";
		unsigned short camera_name_display[50];
		int retval = CCameraDS::CameraName(i, camera_name, sizeof(camera_name));
		if (retval > 0)
		{
			/*8λ�ַ���ҪתΪ16λ�ַ��洢��������ʾ����*/
			unsigned char j = 0;
			string camera_name_temp = camera_name;
			for (j = 0; j < camera_name_temp.size(); j++)
			{
				camera_name_display[j] = camera_name[j];
			}
			camera_name_display[j] = '\0';
			MessageBox(NULL, (LPCWSTR)camera_name_display, (LPCWSTR)L"fuck", MB_DEFBUTTON2);
		}
		else
		{
			s.Format(_T("Can not get Camera %d name.\n"), i);
			MessageBox(NULL, (LPCWSTR)s, (LPCWSTR)L"which camera cannot get", MB_DEFBUTTON2);
		}
	}

	if (!camera.OpenCamera(UVCdevice, true, 360, 240)) //����������ѡ�񴰿ڣ��ô����ƶ�ͼ���͸�, ��true�������Կ�������һ��YUV2,1280X720,�����ٸ�Ϊfalse
	{
		fprintf(stderr, "Can not open camera.\n");
		system("pause");
		return -1;
	}

	HANDLE hThread;
	hThread = CreateThread(NULL,//ָ���������̵İ�ȫ���ԣ�һ��ΪNULL
		0,//initial stack size ��ʼ��ջ��С
		ThreadProc,//thread function ָ���̺߳���
		(LPVOID)hWnd,//thread argument �������ݣ��������������͵�ָ��
		0,//creation option �����̺߳���߳�״̬ 0��ʾ����ִ��
		NULL);//thread identifier �߳�id
	

	// ����Ϣѭ��: 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  ����:  MyRegisterClass()
//
//  Ŀ��:  ע�ᴰ���ࡣ
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_UVCOPENCVDIRECTSHOWDRAWIMAGE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_UVCOPENCVDIRECTSHOWDRAWIMAGE);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   ����:  BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, HWND *hWnd)
//
//   Ŀ��:  ����ʵ�����������������
//
//   ע��: 
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, HWND *hWnd)
{

   hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

   *hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      800, 400, 480, 360, NULL, NULL, hInstance, NULL);

   if (!*hWnd)
   {
      return FALSE;
   }

   ShowWindow(*hWnd, nCmdShow);
   UpdateWindow(*hWnd);

   return TRUE;
}

//
//  ����:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��:    ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
//
//

void PaintWindow(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	int as = 0;
	for (int x = 0; x < 50; x++){
		for (int y = 0; y < 50; y++){
			SetPixel(hdc, x + as, y + as, RGB(0x55, 0x55, 0x55));  // green
		}
	}
	as++;
	EndPaint(hwnd, &ps);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// �����˵�ѡ��: 
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		//hdc = BeginPaint(hWnd, &ps);
		// TODO:  �ڴ���������ͼ����...
		PaintWindow(hWnd);
		//EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// �����ڡ������Ϣ�������
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void DrawImage(float* Depth, int iWidth, int iHeight, HWND hWnd)
{
	//////////////////////////���������ͼ//////////////////////////////////////

	int H_CUT = 0;
	int V_CUT = 0;

	if (1)
	{
		int count = 0;

		int size = iWidth * iHeight;

		float* temp = new float[size];

		for (int i = V_CUT; i < iHeight - V_CUT; i++)
		{
			for (int j = H_CUT; j < iWidth - H_CUT; j++)
			{
				temp[count] = Depth[i * iWidth + j]; // B  //ͼ��ת
				count++;
			}
		}


		int* hotmap = new int[iWidth * iHeight * 3];

		int a = 0;

		for (int i = 0; i < (iWidth - 2 * H_CUT) * (iHeight - 2 * V_CUT); i++)
		{
			int color[3];

			int d = 0;

			if (temp[i] < 0)
			{
				d = 0;
			}
			else
			{
				d = (temp[i] / m_iTargetDistance*128.0);
			}

			GetColor(d, color);

			hotmap[a] = color[2];//B
			hotmap[a + 1] = color[1];//G
			hotmap[a + 2] = color[0];//R
			a += 3;
		}

		BYTE *pRGB = new BYTE[iWidth * iHeight * 3];
		int iImgWidth = iWidth;	// ��Ϊ16������
		int iImgHeight = iHeight;


		for (int j = 0; j < (iWidth - 2 * H_CUT) * (iHeight - 2 * V_CUT) * 3; j += 3)
		{
			pRGB[j] = hotmap[j]; // B 
			pRGB[j + 1] = hotmap[j + 1]; // G
			pRGB[j + 2] = hotmap[j + 2];   // R
		}

		if (pRGB != NULL)
		{
			BITMAPINFOHEADER bmpInfoHeader;

			HDRAWDIB bmpDC;
			HDC hdc = ::GetDC(hWnd);
			bmpDC = DrawDibOpen();

			RECT rect;
			int iDispWidth = 0;
			int iDispHeight = 0;

			::memset(&bmpInfoHeader, 0x00, sizeof(BITMAPINFOHEADER));
			bmpInfoHeader.biSize = sizeof(bmpInfoHeader);
			bmpInfoHeader.biWidth = iImgWidth - 2 * H_CUT;
			bmpInfoHeader.biHeight = iImgHeight - 2 * V_CUT;
			bmpInfoHeader.biPlanes = 1;
			bmpInfoHeader.biBitCount = 24;
			bmpInfoHeader.biCompression = BI_RGB;
			bmpInfoHeader.biSizeImage = 0;
			::GetWindowRect(hWnd, &rect);
			iDispWidth = rect.right - rect.left;
			iDispHeight = rect.bottom - rect.top;

			DrawDibDraw(bmpDC, hdc,
				0, 0,
				iDispWidth, iDispHeight,
				&bmpInfoHeader,
				pRGB,
				0, 0, -1, -1,
				0);

			if (bmpDC != NULL)
			{
				DrawDibClose(bmpDC);
				bmpDC = NULL;
			}
			::ReleaseDC(hWnd, hdc);

			delete[] pRGB;
		}

		delete[]temp;
		delete[]hotmap;
	}
}

void GetColor(int num, int *color)
{
	if (num == 0)
	{
		color[0] = 255;
		color[1] = 255;
		color[2] = 255;
	}
	else if (0 < num && num < 64)
	{
		color[0] = 255;//R
		color[1] = num * 4;//G
		color[2] = 0;//B
	}
	else if (64 <= num && num < 128)
	{
		color[0] = 255 - (num - 64) * 4;
		color[1] = 255;
		color[2] = 0;
	}
	else if (128 <= num && num < 192)
	{
		color[0] = 0;
		color[1] = 255;
		color[2] = (num - 192) * 4;
	}
	else if (192 <= num && num < 256)
	{
		color[0] = 0;
		color[1] = 255 - (num - 192) * 4;
		color[2] = 255;
	}
	else if (num >= 256)
	{
		color[0] = 0;
		color[1] = 0;
		color[2] = 255 - num / 4.0;
	}

}

void reverse_byte_order(float* a)
{
	unsigned char size = sizeof(float);
	unsigned char *p = (unsigned char *)malloc(size);
	unsigned char *pa = (unsigned char *)a;
	memcpy((void*)p, (void*)a, size);
	for (unsigned char i = 0; i < size; i++)
	{
		*(pa + i) = *(p + size - i - 1);
	}
}

DWORD WINAPI ThreadProc(LPVOID lpParam){

	float* Depth = new float[180 * 240];
	m_iTargetDistance = 500;
	//IplImage *pFrame = NULL;
	CString s;
	while (1)
	{
		//��ȡһ֡
		//MessageBox(NULL, (LPCWSTR)L"capturing", (LPCWSTR)L"imageSize", MB_DEFBUTTON2);
		IplImage *pFrame = camera.QueryFrame();
		//MessageBox(NULL, (LPCWSTR)L"QueryFrame finished", (LPCWSTR)L"imageSize", MB_DEFBUTTON2);
		cvFlip(pFrame, pFrame, 0);
		Mat frame(360, 240, CV_16UC3, pFrame->imageData);

		//s.Format(_T("size = %d.\n"), pFrame->imageSize);
		//MessageBox(NULL, (LPCWSTR)s, (LPCWSTR)L"imageSize", MB_DEFBUTTON2);

		for (int i = 0; i < 180 * 240; i++)
		{
			memcpy(&Depth[i], frame.data + 4 * i, sizeof(float));
		}
		write_csv("srcdepth1.csv", Depth, "");
		for (int i = 0; i < 180 * 240; i++)
		{
			reverse_byte_order(&Depth[i]);
		}
		write_csv("srcdepth2.csv", Depth, "");
		DrawImage(Depth, 240, 180, (HWND)lpParam);
		save_img(frame, pFrame);

		pFrame = NULL;

		Sleep(250); 
	}

	return 0;
}

void save_img(Mat frame, IplImage *pFrame)
{
	char filename[256];
	char filename1[32] = "images/RawData";
	char filename2[32] = ".raw";
	FILE *pfile;
	int fileflag;
	CString s;

	strcpy_s(filename, filename1);
	strcat_s(filename, filename2);

	int errNum = 0;
	fileflag = fopen_s(&pfile, filename, "wb");
	if (fileflag != 0)
	{
		MessageBox(NULL, (LPCWSTR)L"open file fail!", (LPCWSTR)L"imageSize", MB_DEFBUTTON2);
		errNum = errno;
		s.Format(_T("errNum = %d.\n"), errNum);
		MessageBox(NULL, (LPCWSTR)s, (LPCWSTR)L"imageSize", MB_DEFBUTTON2);
	}
	fwrite(frame.data, 1, pFrame->imageSize, pfile);
	fclose(pfile);
}

int write_csv(char *file_name, float *src, char* abs_path)
{
	char relative_path[256] = "";
	strcat_s(relative_path, abs_path);
	strcat_s(relative_path, file_name);

	//log_info("output_path =%s \n", relative_path);

	ofstream file(relative_path);
	file.trunc;
	if (file)
	{
		for (int i = 0; i < 180; i++)
		{
			for (int j = 0; j < 240; j++)
			{
				file << src[i*240+j];
				if (j == 239) file << endl;
				else file << ",";
			}
		}
	}

	return 0;
}
