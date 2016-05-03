// easy-buoy.cpp : ����Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include "easy-buoy.h"

// locust: ������Ϣ����
#include	<WindowsX.h>
#include	"libs/win-utils.h"
#include	"logic/handler.h"

#define MAX_LOADSTRING 100

// ȫ�ֱ���:
HINSTANCE hInst;								// ��ǰʵ��
TCHAR szTitle[MAX_LOADSTRING];					// �������ı�
TCHAR szWindowClass[MAX_LOADSTRING];			// ����������

// locust: ȫ�ֶ���
static	HWND	g_hWnd	= NULL;

// �˴���ģ���а����ĺ�����ǰ������:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// locust: ��ʼ��Ӧ�ó���
	{
		TCHAR szCfgFile[MAX_LOADSTRING];
		LoadString(hInstance, IDS_CONFIG_FILE, szCfgFile, MAX_LOADSTRING);
		if(!handle_init_app(__argc >= 2 ? __argv[1] : szCfgFile)){
			return	FALSE;
		}
	}

	MSG msg;

	// ��ʼ��ȫ���ַ���
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WINDESKBUOY, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// ִ��Ӧ�ó����ʼ��:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	// ����Ϣѭ��:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}



//
//  ����: MyRegisterClass()
//
//  Ŀ��: ע�ᴰ���ࡣ
//
//  ע��:
//
//    ����ϣ��
//    �˴�������ӵ� Windows 95 �еġ�RegisterClassEx��
//    ����֮ǰ�� Win32 ϵͳ����ʱ������Ҫ�˺��������÷������ô˺���ʮ����Ҫ��
//    ����Ӧ�ó���Ϳ��Ի�ù�����
//    ����ʽ��ȷ�ġ�Сͼ�ꡣ
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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDESKBUOY));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WINDESKBUOY);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   ����: InitInstance(HINSTANCE, int)
//
//   Ŀ��: ����ʵ�����������������
//
//   ע��:
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

#if	APP_HIDE_MAINWND
	// locust: hide main window
	{
		hWnd = CreateWindowEx(WS_EX_NOACTIVATE, szWindowClass, szTitle, WS_POPUP,
			CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

		if (!hWnd)
		{
			return FALSE;
		}

		ShowWindow(hWnd, SW_HIDE);
		UpdateWindow(hWnd);
	}
#else
	{
		hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

		if (!hWnd)
		{
			return FALSE;
		}

		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);
	}
#endif

	// locust: start timer
	{
		g_hWnd	= hWnd;
		SetTimer(hWnd, 100, 20, NULL);
	}

	return TRUE;
}

//
//  ����: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��: ���������ڵ���Ϣ��
//
//  WM_COMMAND	- ����Ӧ�ó���˵�
//  WM_PAINT	- ����������
//  WM_DESTROY	- �����˳���Ϣ������
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	// locust: timer handler
	{
		UNREFERENCED_PARAMETER(wmId);
		UNREFERENCED_PARAMETER(wmEvent);

		switch (message)
		{
		case WM_TIMER:		main_procedure(hWnd);		break;
		}
	}

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// locust: ��ͼ�Ĵ���
		if(hWnd != g_hWnd && NULL != g_hWnd){
			handle_draw(hWnd, hdc);
		}
		EndPaint(hWnd, &ps);
		break;
		// locust: �����Ĵ���
	case WM_ERASEBKGND:
		if(hWnd != g_hWnd){
			return	TRUE;
		}
		break;
		// locust: ���������
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		if(hWnd != g_hWnd){
			handle_click(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		}
		break;
	case WM_DESTROY:
		// locust: ���������
		//PostQuitMessage(0);
		if(hWnd == g_hWnd){
			PostQuitMessage(0);
		}else{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
