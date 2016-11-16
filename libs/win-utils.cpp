#include	"stdafx.h"

#include	<string>
#include	<fstream>

#include	<CommCtrl.h>

#include	<Shlwapi.h>
#include	<ShlObj.h>

#pragma	comment(lib, "Shlwapi.lib")

//
//	判断是否64位系统
//
bool	win_is_64bits_system(){
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
//	根据类名获取子窗口，同 FindWindowEx
//
HWND	win_find_child_by_class(HWND hWnd, const char* sClass){
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

HWND	win_find_desktop_SHELLDLL_DefView(const char* sClass) {
	HWND	hView	= NULL;
	HWND	hWnd	= win_find_child_by_class(GetDesktopWindow(), sClass);
	while(NULL != hWnd)
	{
		hView	= win_find_child_by_class(hWnd, "SHELLDLL_DefView");
		if(NULL != hView){
			break;
		}

		do{
			char	buf[MAX_PATH]	= {};
			hWnd	= GetWindow(hWnd, GW_HWNDNEXT);
			GetClassName(hWnd, buf, sizeof(buf) - 1);
			if(0 == _strcmpi(buf, "WorkerW")){
				break;
			}
		}while(hWnd != NULL);
	}

	return	hView;
}

//
//	获取桌面SysListView窗口句柄
//
HWND	win_get_desktop_SysListView(){
	HWND	xView	= win_find_desktop_SHELLDLL_DefView("WorkerW");
	if(NULL == xView){
		xView	= win_find_desktop_SHELLDLL_DefView("Progman");
	}

	if(NULL != xView){
		xView	=	win_find_child_by_class(xView, "SysListView32");
	}

	return	xView;
}

//
//	获取桌面图标矩形
//
bool	win_get_desktop_icon_rect(const char* psCaption, RECT* pRect){
	HANDLE	hProcess	= NULL;
	HWND	hView		= win_get_desktop_SysListView();
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

	pRemoteItem	= (LVITEM*)	VirtualAllocEx(hProcess, NULL, sizeof(xItem) + 1, MEM_COMMIT, PAGE_READWRITE);
	pRemoteText	= (char*)	VirtualAllocEx(hProcess, NULL, sizeof(sText) + 1, MEM_COMMIT, PAGE_READWRITE);

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

	VirtualFreeEx(hProcess, pRemoteItem, 0, MEM_RELEASE);
	VirtualFreeEx(hProcess, pRemoteText, 0, MEM_RELEASE);
	CloseHandle(hProcess);

	return	bFound;
}

//
//	获取资源内容
//
bool	win_load_resource_data(
	HMODULE		hModule,
	const char*	res_name,
	const char*	res_type,
	void*&		res_data,
	size_t&		res_size
	){
		HRSRC hRsrc		= FindResourceA(hModule, res_name, res_type);
		if(NULL == hRsrc)	return false;

		DWORD dwSize	= SizeofResource(hModule, hRsrc);
		if(0 == dwSize)		return false;

		HGLOBAL hGlobal	= LoadResource(hModule, hRsrc);
		if(NULL == hGlobal)	return false;

		LPVOID pBuffer	= LockResource(hGlobal);
		if(NULL == pBuffer)	return false;

		res_data	=	pBuffer;
		res_size	=	dwSize;

		return	true;
}

//
//	获取文件内容(必须确保空间足够大)
//
bool	win_load_file_data(
	const char*	file_name,
	void*		res_data,
	size_t&		res_size
	){
		std::ifstream	ifs(file_name, std::ios::binary);
		if(!ifs){
			return	false;
		}

		ifs.seekg(0, std::ios::end);
		size_t	file_size	= size_t(ifs.tellg());
		if(NULL == res_data){
			res_size	= file_size;
			return	true;
		}

		if(res_size < file_size){
			return	false;
		}

		res_size	= file_size;
		ifs.seekg(0, std::ios::beg);
		ifs.read((char*)res_data, res_size);

		return	true;
}

//
//	获取HBITMAP大小
//
bool	win_get_bitmap_size(HBITMAP hbmp, long& nWidth, long& nHeight){
	BITMAP bmp;
	if(NULL == hbmp || !::GetObject(hbmp, sizeof(BITMAP), &bmp)){
		return false;
	}

	nWidth	= bmp.bmWidth;
	nHeight	= bmp.bmHeight;
	return true;
}

//
//	获取当前应用根目录（最后带分隔符/）
//
std::string	win_get_root_path(){
	char szPath[MAX_PATH] = {0};
	GetModuleFileName(GetModuleHandle(NULL), szPath, MAX_PATH);
	PathRemoveFileSpec(szPath);
	PathAddBackslash(szPath);
	return	std::string(szPath);
}

//
//	创建快捷方式 
//
bool	win_create_shortcut(const char* szPath, const char* szWorkingPath, const char* szLink){ 
	HRESULT hres ; 
	IShellLink * psl ; 
	IPersistFile* ppf ; 
	WCHAR wsz[MAX_PATH]={L""}; 
	////初始化COM 
	CoInitialize (NULL); 
	//创建一个IShellLink实例 
	hres = CoCreateInstance( CLSID_ShellLink, NULL,CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl);
	if( FAILED( hres)){ 
		CoUninitialize (); 
		return false ; 
	} 

	//设置目标应用程序 
	psl->SetPath( szPath);
	psl->SetWorkingDirectory(szWorkingPath); 

	//从IShellLink获取其IPersistFile接口 
	//用于保存快捷方式的数据文件 (*.lnk) 
	hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf) ; 
	if( FAILED( hres)){ 
		CoUninitialize (); 
		return false ; 
	} 

	// 确保数据文件名为ANSI格式 
	MultiByteToWideChar( CP_ACP, 0, szLink, -1, wsz, MAX_PATH) ; 

	//调用IPersistFile::Save 
	//保存快捷方式的数据文件 (*.lnk) 
	hres = ppf->Save(wsz, STGM_READWRITE); 
	//释放IPersistFile和IShellLink接口 
	ppf->Release() ; 
	psl->Release() ; 
	CoUninitialize();

	return true; 
}

std::string	win_get_shortcut_realpath(const char* szLink){ 
	wchar_t	wszLink[MAX_PATH]	= {};
	::MultiByteToWideChar(CP_ACP, NULL, szLink, int(strlen(szLink)), wszLink, MAX_PATH);

	char	buf[MAX_PATH]	= {};

	IShellLink   *pShellLink;
	HRESULT hRes;

	::CoInitialize(NULL); 
	hRes = CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void **)&pShellLink); 
	if(SUCCEEDED(hRes)) 
	{ 
		IPersistFile   *ppf; 
		hRes = pShellLink->QueryInterface(IID_IPersistFile,(void **)&ppf); 
		if(SUCCEEDED(hRes))
		{
			hRes = ppf->Load(wszLink, TRUE); 
			if(SUCCEEDED(hRes)) 
			{ 
				pShellLink->GetPath(buf, MAX_PATH, NULL, 0); 
			} 
			ppf->Release(); 
		} 
		pShellLink->Release(); 
	} 
	::CoUninitialize(); 

	return	std::string(buf);
}

void		win_run_shortcut(const char* szLink, const char* szWorkdir){
	wchar_t	wszLink[MAX_PATH]	= {};
	::MultiByteToWideChar(CP_ACP, NULL, szLink, int(strlen(szLink)), wszLink, MAX_PATH);


	IShellLink   *pShellLink;
	HRESULT hRes;

	::CoInitialize(NULL); 
	hRes = CoCreateInstance(CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,IID_IShellLink,(void **)&pShellLink); 
	if(SUCCEEDED(hRes)) 
	{ 
		IPersistFile   *ppf; 
		hRes = pShellLink->QueryInterface(IID_IPersistFile,(void **)&ppf); 
		if(SUCCEEDED(hRes))
		{
			hRes = ppf->Load(wszLink, TRUE); 
			if(SUCCEEDED(hRes)) 
			{ 
				int		show	= SW_SHOW;
				char	path[MAX_PATH]	= {};
				char	workdir[MAX_PATH]	= {};
				pShellLink->GetShowCmd(&show);
				pShellLink->GetPath(path, MAX_PATH, NULL, 0); 
				pShellLink->GetWorkingDirectory(workdir, MAX_PATH); 
				if(path[0] != 0){
					ShellExecute(NULL, "open", path, NULL, workdir, show);
				}
			} 
			ppf->Release(); 
		} 
		pShellLink->Release(); 
	} 
	::CoUninitialize(); 
}
