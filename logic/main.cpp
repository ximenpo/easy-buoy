#include	"stdafx.h"

#include	<WindowsX.h>

#include	"simple/string.h"

#include	"funcs.h"

void	main_procedure(HWND hWnd){
	KillTimer(hWnd, 100);

	RECT	rc	= {};
	get_desktop_icon_RECT("Ω£”ÍΩ≠∫˛", &rc);

	OutputDebugString(string_format("%ld,%ld,%ld,%ld\n", rc.left, rc.top, rc.right, rc.bottom).c_str());
	
	char	wnd_class[MAX_PATH]	= {};
	GetClassName(hWnd, wnd_class, sizeof(wnd_class) - 1);
	CreateWindowEx(NULL, wnd_class, "µ„Œ“ ‘ ‘", WS_VISIBLE | WS_BORDER | WS_CHILD, rc.left, rc.top, 40, 40, get_desktop_SysListView_HWND(), NULL, NULL, NULL);
}

void	main_draw(HWND hWnd, HDC hdc){
	RECT	rc	= {0,0,100,100};
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
}
