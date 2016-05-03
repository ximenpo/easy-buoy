#include	"stdafx.h"

#include	<fstream>
#include	<string>
#include	<memory>

#include	<Shlwapi.h>
#include	<ShellAPI.h>
#include	<ShlObj.h>

#include	"simple/string.h"
#include	"simple/procedure.h"
#include	"simple/timestamp.h"
#include	"simple/stringify.h"

#include	"libs/BitmapHDC.h"
#include	"libs/win-utils.h"

#include	"img.h"

static	BitmapHDC	g_MemDC;
static	SIZE		g_szBuoy	= {};
static	HWND		g_hWndBuoy	= NULL;

procedure_context	g_ctx;
stringify_data		g_cfg;

void	create_shortcuts(){
	char FolderPath[MAX_PATH] = {0};
	SHGetSpecialFolderPath(0, FolderPath, (true?CSIDL_COMMON_DESKTOPDIRECTORY:CSIDL_DESKTOPDIRECTORY), FALSE);
	if(!PathFileExists(string_format("%s\\%s.lnk", FolderPath, "���꽭��").c_str())){
		win_create_shortcut(
			(win_get_root_path() + "6998_���꽭��.exe").c_str(),
			win_get_root_path().c_str(), 
			string_format("%s\\%s.lnk", FolderPath, "���꽭��").c_str()
			);
	}
}

bool	show_buoy_window(HWND hWnd){
	RECT	rc	= {};
	if(!win_get_desktop_icon_rect("���꽭��", &rc)){
		create_shortcuts();
		return	false;
	}

	if(!img_load((win_get_root_path() + "buoy.bmp").c_str())){
		return	true;
	}

	if(!img_fetch_size(&g_szBuoy)){
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

	if(NULL != g_hWndBuoy){
		InvalidateRect(g_hWndBuoy, NULL, FALSE);
		UpdateWindow(g_hWndBuoy);
	}

	static	double		old_timestamp_;
	static	timestamp	timestamp_;

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

bool	handle_init_app(const char* cfg_file){
	std::ifstream	ifs((win_get_root_path() + cfg_file));
	if(!ifs || !stringify_from_ini_stream(g_cfg, ifs)){
		return	false;
	}

#if	!defined(_WIN64)
	{
		if(win_is_64bits_system()){
			std::string	sApp	= g_cfg.get_value("config/app_64bits", "");
			if(!sApp.empty()){
				ShellExecute(NULL, "open", (win_get_root_path() + sApp).c_str(), NULL, win_get_root_path().c_str(), SW_SHOW);
			}
			return	false;
		}
	}
#endif

	SetCurrentDirectory(win_get_root_path().c_str());
	return	true;
}

void	handle_draw(HWND hWnd, HDC hdc){
	if(NULL == g_MemDC){
		g_MemDC.Initialize(g_szBuoy.cx, g_szBuoy.cy, hdc, RGB(0,0,0));
	}

	if(NULL != g_MemDC){
		img_render(g_MemDC);
		BitBlt(hdc, 0, 0, g_szBuoy.cx, g_szBuoy.cy, g_MemDC, 1, 1, SRCCOPY);
	}
}

void	handle_click(HWND hWnd, int x, int y){
	if(hWnd != g_hWndBuoy){
		return;
	}

	RECT	rc	= {282, 137, 293, 148};
	POINT	pt	= {x, y};
	if(!PtInRect(&rc, pt)){
		// TODO: ����Ӧ�ó���
		ShellExecute(NULL, "open", (win_get_root_path() + "6998_���꽭��.exe").c_str(), NULL, win_get_root_path().c_str(), SW_SHOW);
	}

	g_hWndBuoy	= NULL;
	g_MemDC.UnInitialize();

	img_destroy();

	ShowWindow(hWnd, SW_HIDE);
	DestroyWindow(hWnd);
}
