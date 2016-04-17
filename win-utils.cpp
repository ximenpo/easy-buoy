#include	"stdafx.h"

#include	<CommCtrl.h>

//
//	判断是否64位系统
//
bool	is_64bits_windows(){
	typedef VOID (WINAPI *LPFN_GetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);

	SYSTEM_INFO si;
	LPFN_GetNativeSystemInfo fnGetNativeSystemInfo = (LPFN_GetNativeSystemInfo)GetProcAddress( GetModuleHandle(_T("kernel32")), "GetNativeSystemInfo");;
	if (NULL != fnGetNativeSystemInfo){
		fnGetNativeSystemInfo(&si);
	}else{
		GetSystemInfo(&si);
	}
	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
		si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 ){
			return true;
	}

	return false;
}

//
//	根据类名获取子窗口（第一个），亦可用 FindWindowEx 代替
//
HWND	find_child_by_class(HWND hWnd, const char* sClass){
	HWND	hChild	= GetWindow(hWnd, GW_CHILD);
	if(NULL == hChild){
		return	hChild;
	}

	do{
		char	buf[MAX_PATH]	= {};
		GetClassName(hChild, buf, sizeof(buf) - 1);
		if(0 == _strcmpi(buf, sClass)){
			return	hChild;
		}

		hChild	= GetWindow(hChild, GW_HWNDNEXT);
	}while(hChild != NULL);

	return	NULL;
}

//
//	获取桌面SysListView窗口句柄
//
HWND	get_desktop_SysListView_HWND(){
	HWND	hWnd	= GetDesktopWindow();
	if(NULL != hWnd){
		hWnd	=	find_child_by_class(hWnd, "Progman");
		if(NULL == hWnd){
			hWnd	= FindWindow("Progman", NULL);
		}
	}
	if(NULL != hWnd){
		hWnd	=	find_child_by_class(hWnd, "SHELLDLL_DefView");
	}
	if(NULL != hWnd){
		hWnd	=	find_child_by_class(hWnd, "SysListView32");
	}
	return	hWnd;
}

//
//	获取桌面图标矩形
//
bool	get_desktop_icon_RECT(const char* psCaption, RECT* pRect){
	HANDLE	hProcess	= NULL;
	HWND	hView	= get_desktop_SysListView_HWND();
	{
		DWORD PID;
		GetWindowThreadProcessId(hView, &PID);
		hProcess=OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, PID);
		if(NULL == hProcess){
			return	false;
		}
	}

	LVITEM	xItem,	*pRemoteItem		= NULL;
	char	sText[512],	*pRemoteText	= NULL;

	pRemoteItem	= (LVITEM*)	VirtualAllocEx(hProcess, NULL, sizeof(xItem), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	pRemoteText	= (char*)	VirtualAllocEx(hProcess, NULL, sizeof(sText), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	bool	bFound	= false;
	int		nSum	= ListView_GetItemCount(hView);
	for(int i = 0; i < nSum; ++i){
		memset(&xItem, 0, sizeof(xItem));
		xItem.mask			= LVIF_TEXT;
		xItem.iItem			= i;
		xItem.iSubItem		= 0;
		xItem.pszText		= pRemoteText;
		xItem.cchTextMax	= sizeof(sText);

		WriteProcessMemory(hProcess, pRemoteItem, &xItem, sizeof(xItem), NULL);
		::SendMessage(hView, LVM_GETITEM, 0, (LPARAM)pRemoteItem);
		ReadProcessMemory(hProcess, pRemoteText, sText, sizeof(sText), NULL);
		if(0 == _strcmpi(sText, psCaption)){
			if(NULL != pRect){
				memset(pRect, 0, sizeof(RECT));
				pRect->left	= LVIR_SELECTBOUNDS;
				WriteProcessMemory(hProcess, pRemoteText, pRect, sizeof(RECT), NULL);
				::SendMessage(hView, LVM_GETITEMRECT, (WPARAM)i, (LPARAM)pRemoteText);
				ReadProcessMemory(hProcess, pRemoteText, pRect, sizeof(RECT), NULL);
			}
			bFound	= true;
			break;
		}
	}

	CloseHandle(hProcess);
	VirtualFreeEx(hProcess, pRemoteItem, 0, MEM_RELEASE);
	VirtualFreeEx(hProcess, pRemoteText, 0, MEM_RELEASE);

	return	bFound;
}
