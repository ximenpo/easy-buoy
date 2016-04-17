#pragma	once

bool	is_64bits_windows();
HWND	get_desktop_SysListView_HWND();
bool	get_desktop_icon_RECT(const char* psName, RECT* pRect);

bool	fetch_resource_data(
    HMODULE		hModule,
    const char*	res_name,
    const char*	res_type,
	void*&		res_data,
	size_t&		res_size
	);

bool get_bitmap_size(HBITMAP hbmp, long& nWidth, long& nHeight);


std::string	get_app_root_path();
