#pragma	once

bool	img_load(const char* file);
bool	img_fetch_size(SIZE* sz);
bool	img_render(HDC hdc, double timestamp);
void	img_destroy();

