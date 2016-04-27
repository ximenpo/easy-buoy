#pragma	once


		extern	bool	win_is_64bits_system();
			extern	void	handle_start_64bits_app();
		extern	void	handle_init_app();
		extern	void	main_procedure(HWND hWnd);
			extern	void	handle_draw(HWND hWnd, HDC hdc);
			extern	void	handle_click(HWND hWnd, int x, int y);