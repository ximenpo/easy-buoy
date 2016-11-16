#pragma	once

bool	main_procedure(HWND hWnd);
bool	handle_init_app(const char* search_paths, const char* cfg_files);
void	handle_draw(HWND hWnd, HDC hdc);
void	handle_click(HWND hWnd, int x, int y);
