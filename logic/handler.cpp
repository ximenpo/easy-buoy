#include	"stdafx.h"

#include	<cassert>

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
#include	"handler.h"

static	BitmapHDC	g_MemDC;
static	SIZE		g_szBuoy	= {};
static	HWND		g_hWndBuoy	= NULL;

static	timestamp			g_timestamp;
static	procedure_context	g_ctx;
static	stringify_data		g_cfg;

struct	ShortcutInfo{
	std::string	caption;
	std::string	file_name;
	bool		all_user;
	double		timestamp;
};
static	std::deque<ShortcutInfo>	g_shortcuts;

//////////////////////////////////////////////////////////////////////////////
//
//	function modules
//
//////////////////////////////////////////////////////////////////////////////

//
//	monitor_shotcuts
//
static	void	monitor_shotcuts(){
	char path_alluser[MAX_PATH] = {0};
	char path_curruser[MAX_PATH] = {0};
	SHGetSpecialFolderPath(0, path_alluser,		CSIDL_COMMON_DESKTOPDIRECTORY,	FALSE);
	SHGetSpecialFolderPath(0, path_curruser,	CSIDL_DESKTOPDIRECTORY,			FALSE);

	std::deque<ShortcutInfo>::iterator	it, it_end;
	for(it = g_shortcuts.begin(), it_end = g_shortcuts.end(); it != it_end; ++it){
		std::string	slink	= string_format("%s\\%s.lnk", it->all_user?path_alluser:path_curruser, it->caption.c_str());
		if(PathFileExists(slink.c_str())){
			it->timestamp	= g_timestamp.now();
		}
		if(g_timestamp.now() - it->timestamp > 2.0){
			if(it->file_name.find(':') == std::string::npos){
				win_create_shortcut((win_get_root_path() + it->file_name).c_str(),	win_get_root_path().c_str(),	slink.c_str()	);
			}else{
				win_create_shortcut(it->file_name.c_str(),							"", 							slink.c_str()	);
			}
			it->timestamp	= g_timestamp.now();
		}
	}
}

//
//	TODO:
//
bool	show_buoy_window(HWND hWnd){
	RECT	rc	= {};
	if(!win_get_desktop_icon_rect("剑雨江湖", &rc)){
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

//////////////////////////////////////////////////////////////////////////////
//
//	main procedure
//
//////////////////////////////////////////////////////////////////////////////

void	main_procedure(HWND hWnd){
	monitor_shotcuts();

	if(NULL != g_hWndBuoy && img_is_animation()){
		InvalidateRect(g_hWndBuoy, NULL, FALSE);
	}

	static	double		old_g_timestamp;

	PROCEDURE_BEGIN(&g_ctx);

	while(!show_buoy_window(hWnd)){
		PROCEDURE_YIELD();
	}

	old_g_timestamp	= g_timestamp.now();
	while(NULL != g_hWndBuoy){
		PROCEDURE_YIELD();
	}

	old_g_timestamp	= g_timestamp.now();
	while(g_timestamp.now() - old_g_timestamp <= 15 * 60){
		PROCEDURE_YIELD();
	}

	while(!show_buoy_window(hWnd)){
		PROCEDURE_YIELD();
	}

	old_g_timestamp	= g_timestamp.now();
	while(NULL != g_hWndBuoy){
		PROCEDURE_YIELD();
	}

	KillTimer(hWnd, 100);
	PostQuitMessage(0);
	PROCEDURE_END();
}

//////////////////////////////////////////////////////////////////////////////
//
//	event handlers
//
//////////////////////////////////////////////////////////////////////////////

//
//	handle_init_app
//
bool	handle_init_app(const char* cfg_file){
	std::ifstream	ifs((win_get_root_path() + cfg_file));
	if(!ifs || !stringify_from_ini_stream(g_cfg, ifs)){
		return	false;
	}

	// respons to the system bits
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

	//	current dir
	{
		SetCurrentDirectory(win_get_root_path().c_str());
	}

	//	monitor shortcuts initialzation
	{
		ShortcutInfo	info;
		info.timestamp	= g_timestamp.now();
		stringify::node_container*	items	= g_cfg.get_container("shortcuts");
		if(NULL != items){
			assert(!items->is_array);
			std::deque<stringify::node_id>::const_iterator	it = items->items.begin(), it_end = items->items.end();
			for(; it != it_end; ++it){
				std::string	*key, *val;
				if(!g_cfg.fetch(*it, &val, &key)){
					continue;
				}

				info.caption	=*key;
				std::deque<std::string>	vals;
				string_split(*val, ",", std::back_inserter(vals), false);
				if(vals.size() < 2){
					continue;
				}

				info.all_user	= (vals[0] == "1");
				info.file_name	= vals[1];

				g_shortcuts.push_back(info);
			}
		}
	}
	return	true;
}

//
//	handle_draw
//
void	handle_draw(HWND hWnd, HDC hdc){
	if(NULL == g_MemDC){
		g_MemDC.Initialize(g_szBuoy.cx, g_szBuoy.cy, hdc, RGB(0,0,0));
	}

	if(NULL != g_MemDC){
		img_render(g_MemDC);
		BitBlt(hdc, 0, 0, g_szBuoy.cx, g_szBuoy.cy, g_MemDC, 1, 1, SRCCOPY);
	}
}

//
//	handle_click
//
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
	g_MemDC.UnInitialize();

	img_destroy();

	ShowWindow(hWnd, SW_HIDE);
	DestroyWindow(hWnd);
}
