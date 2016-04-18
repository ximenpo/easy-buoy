#include	"stdafx.h"

#include	<WindowsX.h>
#include	<Shlwapi.h>
#include	<ShellAPI.h>
#include	<ShlObj.h>


#include	"simple/string.h"
#include	"simple/procedure.h"
#include	"simple/timestamp.h"

#include	"funcs.h"
#include	"libs/BitmapHDC.h"
#include	"libs/HtmlUtils.h"

BitmapHDC	g_dcBuoy;
SIZE		g_szBuoy	= {};
HWND		g_hWndBuoy	= NULL;
HBITMAP		g_hBitmap	= NULL;

procedure_context	g_ctx;

void	show_buoy_window(HWND hWnd){
	RECT	rc	= {};
	if(!get_desktop_icon_RECT("剑雨江湖", &rc)){
		return;
	}

	if(NULL == g_hBitmap){
		g_hBitmap	= (HBITMAP)LoadImage(NULL, (get_app_root_path() + "buoy.bmp").c_str(),IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION|LR_DEFAULTSIZE|LR_LOADFROMFILE);
	}
	if(NULL == g_hBitmap){
		return;
	}

	if(!get_bitmap_size(g_hBitmap, g_szBuoy.cx, g_szBuoy.cy)){
		return;
	}

	void*	data	= NULL;
	size_t	size	= 0;
	if(!fetch_resource_data(NULL, "BUOY_RGN", "BUOY_RGN", data, size)){
		return;
	}

	char	wnd_class[MAX_PATH]	= {};
	GetClassName(hWnd, wnd_class, sizeof(wnd_class) - 1);

	g_hWndBuoy	= CreateWindowEx(NULL, wnd_class, NULL, WS_VISIBLE | WS_BORDER | WS_CHILD, rc.right - 15, rc.top - 20, g_szBuoy.cx, g_szBuoy.cy, get_desktop_SysListView_HWND(), NULL, NULL, NULL);

	XFORM xform;	  
	xform.eM11 = (FLOAT) 1.0;          
	xform.eM22 = (FLOAT) 1.0; 
	xform.eM12 = (FLOAT) 0.0;       
	xform.eM21 = (FLOAT) 0.0;             
	xform.eDx  = (FLOAT) 0.0;             
	xform.eDy  = (FLOAT) 0.0; 

	HRGN rgn = ExtCreateRegion(&xform, size,(RGNDATA*)data);
	::SetWindowRgn(g_hWndBuoy, rgn, TRUE);
}

void	main_procedure(HWND hWnd){

	static	timestamp	timestamp_;
	static	double		old_timestamp_;

	PROCEDURE_BEGIN(&g_ctx);

	show_buoy_window(hWnd);

	old_timestamp_	= timestamp_.now();
	while(NULL != g_hWndBuoy){
		PROCEDURE_YIELD();
	}

	old_timestamp_	= timestamp_.now();
	while(timestamp_.now() - old_timestamp_ <= 15 * 60){
		PROCEDURE_YIELD();
	}

	show_buoy_window(hWnd);

	old_timestamp_	= timestamp_.now();
	while(NULL != g_hWndBuoy){
		PROCEDURE_YIELD();
	}

	KillTimer(hWnd, 100);
	PostQuitMessage(0);
	PROCEDURE_END();
}

void	handle_init_app(){
	SetCurrentDirectory(get_app_root_path().c_str());
}

void	handle_draw(HWND hWnd, HDC hdc){
	if(hWnd != g_hWndBuoy){
		return;
	}

	if(NULL == g_dcBuoy){
		g_dcBuoy.Initialize(g_szBuoy.cx, g_szBuoy.cy, hdc, RGB(0,0,0));
		if(NULL != g_hBitmap){
			::SelectObject(g_dcBuoy, g_hBitmap);
		}
	}

	//RECT	rc	= {0,0,100,100};
	//ExtTextOut(dc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	//DrawHTML(dc, "<b>你好</b>, world", -1, &rc, DT_CENTER, 0);

	if(NULL != g_dcBuoy){
		BitBlt(hdc, 0, 0, g_szBuoy.cx, g_szBuoy.cy, g_dcBuoy, 0, 0, SRCCOPY);
	}
}

void	handle_click(HWND hWnd, int x, int y){
	if(hWnd != g_hWndBuoy){
		return;
	}

	RECT	rc	= {37, 42, 120, 66};
	POINT	pt	= {x, y};
	if(!PtInRect(&rc, pt)){
		// TODO: 启动应用程序
		ShellExecute(NULL, "open", (get_app_root_path() + "6998_剑雨江湖.exe").c_str(), NULL, get_app_root_path().c_str(), SW_SHOW);
	}

	g_hWndBuoy	= NULL;
	g_dcBuoy.UnInitialize();

	ShowWindow(hWnd, SW_HIDE);
	DestroyWindow(hWnd);
}

void	handle_start_64bits_app(){
	// TODO: 启动64位应用程序
	ShellExecute(NULL, "open", (get_app_root_path() + "windesk-buoy.amd64.exe").c_str(), NULL, get_app_root_path().c_str(), SW_SHOW);
}
