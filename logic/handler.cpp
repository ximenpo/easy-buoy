#include	"stdafx.h"

#include	<cassert>

#include	<fstream>
#include	<iterator>
#include	<string>
#include	<memory>

#include	<Shlwapi.h>
#include	<ShellAPI.h>
#include	<ShlObj.h>

#include	"simple/string.h"
#include	"simple/timestamp.h"
#include	"simple/stringify.h"
#include	"simple/turing_calculator.h"

#include	"libs/BitmapHDC.h"
#include	"libs/win-utils.h"

#include	"img.h"
#include	"handler.h"

static	BitmapHDC	g_MemDC;
static	SIZE		g_szBuoy	= {};
static	HWND		g_hWndBuoy	= NULL;
static	std::string	g_sWndClass;

static	timestamp			g_timestamp;
static	stringify_data		g_cfg;
static	std::string			g_searchpath;

struct	ShortcutInfo{
	std::string	caption;
	std::string	file_name;
	bool		all_user;
	double		timestamp;
};
static	std::deque<ShortcutInfo>	g_shortcuts;

struct	BuoyInfo{
	std::string	caption;
	std::string	img_file;
	std::string	rgn_file;
	RECT		rc_close;
	SIZE		offset;
	RECT		rc_icon;
};
static	BuoyInfo		g_buoyinfo	= {};

//////////////////////////////////////////////////////////////////////////////
//
//	function modules
//
//////////////////////////////////////////////////////////////////////////////

//
//	monitor_shotcuts
//
static	void	monitor_shotcuts(){
	// ¼ì²â¿ª¹Ø
	{
		bool	b_recreate	= false;
		string_tobool(g_cfg.get_value("config/recreate_shortcuts", "false"),	b_recreate);
		if(!b_recreate){
			return;
		}
	}

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
//	show buoy window
//
bool	show_buoy(const char* buoy_name){
	if(NULL == buoy_name){
		return	false;
	}
	stringify::node_container*	items	= g_cfg.get_container(buoy_name);
	if(NULL == items){
		return	false;
	}

	// fetch buoy info
	{
		g_buoyinfo.caption	= g_cfg.get_value(buoy_name, "caption", "");
		g_buoyinfo.img_file	= g_cfg.get_value(buoy_name, "img",		"");
		g_buoyinfo.rgn_file	= g_cfg.get_value(buoy_name, "rgn",		"");
		string_tonumbers(g_cfg.get_value(buoy_name, "offset", "0,0"),			g_buoyinfo.offset.cx, g_buoyinfo.offset.cy);
		string_tonumbers(g_cfg.get_value(buoy_name, "close_rect", "0,0,0,0"),	g_buoyinfo.rc_close.left, g_buoyinfo.rc_close.top, g_buoyinfo.rc_close.right, g_buoyinfo.rc_close.bottom);
		
		char	buf[MAX_PATH]	={};
		if(SearchPath(g_searchpath.c_str(), g_buoyinfo.img_file.c_str(), NULL, sizeof(buf) - 1, buf, NULL)){
			g_buoyinfo.img_file.assign(buf);
		}
		if(SearchPath(g_searchpath.c_str(), g_buoyinfo.rgn_file.c_str(), NULL, sizeof(buf) - 1, buf, NULL)){
			g_buoyinfo.rgn_file.assign(buf);
		}
	}

	if(!win_get_desktop_icon_rect(g_buoyinfo.caption.c_str(), &g_buoyinfo.rc_icon)){
		return	false;
	}

	if(!img_load(g_buoyinfo.img_file.c_str()) || !img_fetch_size(&g_szBuoy)){
		return	true;
	}

	size_t	size	= 0;
	if(!win_load_file_data(g_buoyinfo.rgn_file.c_str(), NULL, size)){
		return	true;
	}
	std::auto_ptr<char>	data(new char[size]);
	if(!win_load_file_data(g_buoyinfo.rgn_file.c_str(), data.get(), size)){
		return	true;
	}

	g_hWndBuoy	= CreateWindowEx(
		NULL, 
		g_sWndClass.c_str(), 
		NULL, 
		WS_VISIBLE | WS_CHILD, 
		g_buoyinfo.rc_icon.right + g_buoyinfo.offset.cx, 
		g_buoyinfo.rc_icon.top + g_buoyinfo.offset.cy, 
		g_szBuoy.cx, 
		g_szBuoy.cy, 
		win_get_desktop_SysListView(), 
		NULL, 
		NULL, 
		NULL
		);

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

void	hide_buoy(){
	if(NULL != g_hWndBuoy){
		PostMessage(g_hWndBuoy, WM_DESTROY, 0, 0);
	}
}

bool	has_buoy(){
	return	(NULL != g_hWndBuoy);
}

#define FX(FUNK,LEN,BIN,DO) if(!strncmp(buf,FUNK,LEN)) { move = LEN + fetch_func_params(buf+LEN,BIN,x,y); { DO; } }
class	BuoyCalculator	: public turing_calculator {
public:
	double		resume_timestamp;
protected:
	variable	do_find_function(const char* buf, int& move) {
		calculator::variable x,y;
		switch (*buf) {
		case 'H':
			FX("HasBuoy(",		8,0, return has_buoy()?1.0:0.0;);
			FX("HideBuoy(",		9,0, hide_buoy();return 1.0;);
			break;
		case 'S':
			FX("ShowBuoy(",		9,1, show_buoy(get_str_variable(size_t(x)));return 1.0;);
			break;
		case 'W':
			FX("Wait(",			5,1, {
				if(get_matchine())get_matchine()->set_paused(true);
				this->resume_timestamp	= g_timestamp.now() + x;
				return 1.0;
			});
			break;
		}
		return	turing_calculator::do_find_function(buf, move);
	}
};
#undef	FX
static	turing_machine			g_machine;
static	BuoyCalculator			g_calculator;
static	std::deque<string_line>	g_instructions;

//////////////////////////////////////////////////////////////////////////////
//
//	main procedure
//
//////////////////////////////////////////////////////////////////////////////

bool	main_procedure(HWND hWnd){
	// fetch window class
	if(g_sWndClass.empty()){
		char	wnd_class[MAX_PATH]	= {};
		GetClassName(hWnd, wnd_class, sizeof(wnd_class) - 1);
		g_sWndClass	= wnd_class;
	}

	// handle shortcuts
	monitor_shotcuts();

	// refresh buoy window
	if(NULL != g_hWndBuoy){
		RECT	rc;
		if(win_get_desktop_icon_rect(g_buoyinfo.caption.c_str(), &rc) && 0 != memcmp(&rc, &g_buoyinfo.rc_icon, sizeof(rc))){
			g_buoyinfo.rc_icon	= rc;
			MoveWindow(
				g_hWndBuoy,
				g_buoyinfo.rc_icon.right + g_buoyinfo.offset.cx, 
				g_buoyinfo.rc_icon.top + g_buoyinfo.offset.cy, 
				g_szBuoy.cx, 
				g_szBuoy.cy,
				TRUE
				);
		}
		if(img_is_animation()){
			InvalidateRect(g_hWndBuoy, NULL, FALSE);
		}
		if(img_is_animation()){
			InvalidateRect(g_hWndBuoy, NULL, FALSE);
		}
	}

	if(g_machine.is_paused() && g_timestamp.now() > g_calculator.resume_timestamp){
		g_machine.set_paused(false);
	}
	if(!g_machine.is_paused()){
		g_machine.run(false);
	}

	return	!g_machine.is_started();
	/*
	static	double		old_g_timestamp;

	PROCEDURE_BEGIN(&g_ctx);

	while(!show_buoy("½£Óê½­ºþ")){
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

	while(!show_buoy("½£Óê½­ºþ")){
	PROCEDURE_YIELD();
	}

	old_g_timestamp	= g_timestamp.now();
	while(NULL != g_hWndBuoy){
	PROCEDURE_YIELD();
	}

	KillTimer(hWnd, 100);
	PostQuitMessage(0);
	PROCEDURE_END();
	*/
}

//////////////////////////////////////////////////////////////////////////////
//
//	event handlers
//
//////////////////////////////////////////////////////////////////////////////

//
//	handle_init_app
//
bool	handle_init_app(const char* search_paths, const char* cfg_files){
	if(NULL == search_paths || NULL == cfg_files) {
		return	false;
	}
	
	//	change working dir
	{
#if	defined(NDEBUG)
		SetCurrentDirectory(win_get_root_path().c_str());
#endif
	}

	{
		g_searchpath	= search_paths;
		std::deque<std::string>	cfgs;
		string_split(cfg_files, ";", std::back_inserter(cfgs));

		bool	cfg_loaded	= false;
		std::deque<std::string>::const_iterator	it	= cfgs.begin();
		std::deque<std::string>::const_iterator	it_end	= cfgs.end();
		for(; it != it_end; ++it){
			char	buf[MAX_PATH]	={};
			if(!SearchPath(g_searchpath.c_str(), it->c_str(), NULL, sizeof(buf) - 1, buf, NULL)){
				continue;
			}

			std::ifstream	ifs(buf);
			if(ifs && stringify_from_ini_stream(g_cfg, ifs)){
				cfg_loaded	= true;
			}
		}

		if(!cfg_loaded){
			return	false;
		}
	}

	// respons to the system bits
#if	!defined(_WIN64)
	{
		if(win_is_64bits_system()){
			std::string	sApp	= g_cfg.get_value("config/app_64bits", "");
			if(!sApp.empty()){
				ShellExecute(NULL, "open", (win_get_root_path() + sApp).c_str(), string_format("\"%s\" \"%s\"", search_paths, cfg_files).c_str(), win_get_root_path().c_str(), SW_SHOW);
			}
			return	false;
		}
	}
#endif

	//	shortcuts initialzation
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

	// instructions
	{
		std::string	file	= g_cfg.get_value("config/instruction_file", "");
		{
			if(file.empty()){
				return	false;
			}

			char	buf[MAX_PATH]	={};
			if(!SearchPath(g_searchpath.c_str(), file.c_str(), NULL, sizeof(buf) - 1, buf, NULL)){
				return	false;
			}
			file.assign(buf);
		}

		std::ifstream	ifs(file);
		if(ifs){
			std::copy(std::istream_iterator<string_line>(ifs),
				std::istream_iterator<string_line>(),
				std::back_inserter(g_instructions)
				);
		}

		if(g_instructions.empty()){
			return	false;
		}

		if(!g_calculator.load_instructions(g_instructions.begin(), g_instructions.size())){
			return	false;
		}
	}

	// virtual machine
	{
		g_machine.instruction_executor = bind(&turing_calculator::execute_instruction, &g_calculator);
		g_machine.start();
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
		BitBlt(hdc, 0, 0, g_szBuoy.cx, g_szBuoy.cy, g_MemDC, 0, 0, SRCCOPY);
	}
}

//
//	handle_click
//
void	handle_click(HWND hWnd, int x, int y){
	if(hWnd != g_hWndBuoy){
		return;
	}

	POINT	pt	= {x, y};
	if(!PtInRect(&g_buoyinfo.rc_close, pt)){
		std::deque<ShortcutInfo>::iterator	it, it_end;
		for(it = g_shortcuts.begin(), it_end = g_shortcuts.end(); it != it_end; ++it){
			if(it->caption != g_buoyinfo.caption){
				continue;
			}
			char folder[MAX_PATH] = {0};
			SHGetSpecialFolderPath(0, folder, it->all_user?CSIDL_COMMON_DESKTOPDIRECTORY:CSIDL_DESKTOPDIRECTORY,	FALSE);
			std::string	slink	= string_format("%s\\%s.lnk", folder, g_buoyinfo.caption.c_str());
			win_run_shortcut(slink.c_str(), win_get_root_path().c_str());
			break;
		}
	}

	g_hWndBuoy	= NULL;
	g_MemDC.UnInitialize();

	img_destroy();

	ShowWindow(hWnd, SW_HIDE);
	DestroyWindow(hWnd);
}
