#pragma	once

void	main_procedure(HWND hWnd);
bool	handle_init_app(const char* cfg_file);
void	handle_draw(HWND hWnd, HDC hdc);
void	handle_click(HWND hWnd, int x, int y);
