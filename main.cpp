#include	"stdafx.h"

#include	<WindowsX.h>

#include	"simple/string.h"

#include	"funcs.h"

void	main_procedure(HWND hWnd){
	KillTimer(hWnd, 100);

	RECT	rc	= {};
	get_desktop_icon_RECT("Ω£”ÍΩ≠∫˛", &rc);

	OutputDebugString(string_format("%ld,%ld,%ld,%ld\n", rc.left, rc.top, rc.right, rc.bottom).c_str());

	CreateWindowEx(NULL, "Button", "µ„Œ“ ‘ ‘", WS_VISIBLE | WS_BORDER | WS_CHILD, rc.left, rc.top, 40, 40, get_desktop_SysListView_HWND(), NULL, NULL, NULL);
}
