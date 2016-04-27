#pragma	once

bool	win_is_64bits_system();
HWND	win_get_desktop_SysListView();
bool	win_get_desktop_icon_rect(const char* psName, RECT* pRect);

bool	win_load_resource_data(HMODULE hModule, const char* res_name, const char* res_type, void*& res_data, size_t& res_size);
bool	win_load_file_data(const char* file_name, void* res_data, size_t& res_size);

bool	win_get_bitmap_size(HBITMAP hbmp, long& nWidth, long& nHeight);


std::string	win_get_root_path();
bool		win_create_shortcut(const char* szPath, const char* szWorkingPath, const char* szLink);
