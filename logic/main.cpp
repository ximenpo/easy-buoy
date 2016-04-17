#include	"stdafx.h"

#include	<WindowsX.h>

#include	"simple/string.h"

#include	"funcs.h"
#include	"libs/BitmapHDC.h"
#include	"libs/HtmlUtils.h"

void	main_procedure(HWND hWnd){
	KillTimer(hWnd, 100);

	RECT	rc	= {};
	get_desktop_icon_RECT("剑雨江湖", &rc);

	//OutputDebugString(string_format("%ld,%ld,%ld,%ld\n", rc.left, rc.top, rc.right, rc.bottom).c_str());
	
	char	wnd_class[MAX_PATH]	= {};
	GetClassName(hWnd, wnd_class, sizeof(wnd_class) - 1);
	HWND	hWndBuoy	= CreateWindowEx(NULL, wnd_class, "点我试试", WS_VISIBLE | WS_BORDER | WS_CHILD, rc.right - 20, rc.top - 20, 100, 100, get_desktop_SysListView_HWND(), NULL, NULL, NULL);
}

void	handle_draw(HWND hWnd, HDC hdc){
	static	BitmapHDC	dc;
	dc.Initialize(100, 100, hdc, RGB(255,0,0));

	RECT	rc	= {0,0,100,100};
	ExtTextOut(dc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	DrawHTML(dc, "<b>你好</b>, world", -1, &rc, DT_CENTER, 0);

	BitBlt(hdc, 0, 0, 100, 100, dc, 0, 0, SRCCOPY);
}

void	handle_click(HWND hWnd, int x, int y){
	PostQuitMessage(0);
}
