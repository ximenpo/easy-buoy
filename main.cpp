#include	"stdafx.h"

#include	"simple/string.h"

#include	<CommCtrl.h>
#include	<comdef.h>

#include	"funcs.h"

#include	"simple/string.h"

void	main_procedure(HWND hWnd){
	KillTimer(hWnd, 100);

	RECT	rc	= {};
	get_desktop_icon_RECT("ªÿ ’’æ", &rc);

	OutputDebugString(string_format("%ld,%ld,%ld,%ld\n", rc.left, rc.top, rc.right, rc.bottom).c_str());
}
