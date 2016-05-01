#include	"stdafx.h"

#include	<string>
#include	<memory>

#include	<Shlwapi.h>
#include	<ShellAPI.h>
#include	<ShlObj.h>

#include	"simple/string.h"
#include	"simple/procedure.h"
#include	"simple/timestamp.h"

#include	"libs/win-utils.h"
#include	"libs/BitmapHDC.h"

BitmapHDC	g_dcBuoy;
SIZE		g_szBuoy	= {};
HWND		g_hWndBuoy	= NULL;
HBITMAP		g_hBitmap	= NULL;

procedure_context	g_ctx;

void	create_shortcuts(){
	char FolderPath[MAX_PATH] = {0};
	SHGetSpecialFolderPath(0, FolderPath, (true?CSIDL_COMMON_DESKTOPDIRECTORY:CSIDL_DESKTOPDIRECTORY), FALSE);
	if(!PathFileExists(string_format("%s\\%s.lnk", FolderPath, "剑雨江湖").c_str())){
		win_create_shortcut(
			(win_get_root_path() + "6998_剑雨江湖.exe").c_str(),
			win_get_root_path().c_str(), 
			string_format("%s\\%s.lnk", FolderPath, "剑雨江湖").c_str()
			);
	}
}

bool	show_buoy_window(HWND hWnd){
	RECT	rc	= {};
	if(!win_get_desktop_icon_rect("剑雨江湖", &rc)){
		create_shortcuts();
		return	false;
	}

	if(NULL == g_hBitmap){
		g_hBitmap	= (HBITMAP)LoadImage(NULL, (win_get_root_path() + "buoy.bmp").c_str(),IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION|LR_DEFAULTSIZE|LR_LOADFROMFILE);
	}
	if(NULL == g_hBitmap){
		return	true;
	}

	if(!win_get_bitmap_size(g_hBitmap, g_szBuoy.cx, g_szBuoy.cy)){
		return	true;
	}
	
	size_t	size	= 0;
	if(!win_load_file_data("buoy.rgn", NULL, size)){
		return	true;
	}
	std::auto_ptr<char>	data(new char[size]);
	if(!win_load_file_data("buoy.rgn", data.get(), size)){
		return	true;
	}

	char	wnd_class[MAX_PATH]	= {};
	GetClassName(hWnd, wnd_class, sizeof(wnd_class) - 1);

	g_hWndBuoy	= CreateWindowEx(NULL, wnd_class, NULL, WS_VISIBLE | WS_BORDER | WS_CHILD, rc.right - 15, rc.top - 160, g_szBuoy.cx, g_szBuoy.cy, win_get_desktop_SysListView(), NULL, NULL, NULL);

	XFORM xform;
	xform.eM11 = (FLOAT) 1.0;          
	xform.eM22 = (FLOAT) 1.0; 
	xform.eM12 = (FLOAT) 0.0;       
	xform.eM21 = (FLOAT) 0.0;             
	xform.eDx  = (FLOAT) 0.0;             
	xform.eDy  = (FLOAT) 0.0; 

	HRGN rgn = ExtCreateRegion(&xform, DWORD(size),(RGNDATA*)data.get());
	::SetWindowRgn(g_hWndBuoy, rgn, TRUE);
	return	true;
}

void	main_procedure(HWND hWnd){
	create_shortcuts();

	static	timestamp	timestamp_;
	static	double		old_timestamp_;

	PROCEDURE_BEGIN(&g_ctx);

	while(!show_buoy_window(hWnd)){
		PROCEDURE_YIELD();
	}

	old_timestamp_	= timestamp_.now();
	while(NULL != g_hWndBuoy){
		PROCEDURE_YIELD();
	}

	old_timestamp_	= timestamp_.now();
	while(timestamp_.now() - old_timestamp_ <= 15 * 60){
		PROCEDURE_YIELD();
	}

	while(!show_buoy_window(hWnd)){
		PROCEDURE_YIELD();
	}

	old_timestamp_	= timestamp_.now();
	while(NULL != g_hWndBuoy){
		PROCEDURE_YIELD();
	}

	KillTimer(hWnd, 100);
	PostQuitMessage(0);
	PROCEDURE_END();
}

void	handle_start_64bits_app(){
	// TODO: 启动64位应用程序
	ShellExecute(NULL, "open", (win_get_root_path() + "easy-buoy.amd64.exe").c_str(), NULL, win_get_root_path().c_str(), SW_SHOW);
}

bool	handle_init_app(){
	// locust: 根据系统启动对应的程序
#if	!defined(_WIN64)
	{
		if(win_is_64bits_system()){
			handle_start_64bits_app();
			return	false;
		}
	}
#endif

	SetCurrentDirectory(win_get_root_path().c_str());
	return	true;
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
		BitBlt(hdc, 0, 0, g_szBuoy.cx, g_szBuoy.cy, g_dcBuoy, 1, 1, SRCCOPY);
	}
}

void	handle_click(HWND hWnd, int x, int y){
	if(hWnd != g_hWndBuoy){
		return;
	}

	RECT	rc	= {282, 137, 293, 148};
	POINT	pt	= {x, y};
	if(!PtInRect(&rc, pt)){
		// TODO: 启动应用程序
		ShellExecute(NULL, "open", (win_get_root_path() + "6998_剑雨江湖.exe").c_str(), NULL, win_get_root_path().c_str(), SW_SHOW);
	}

	g_hWndBuoy	= NULL;
	g_dcBuoy.UnInitialize();

	ShowWindow(hWnd, SW_HIDE);
	DestroyWindow(hWnd);
}
