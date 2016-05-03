@echo off
@pushd	%~dp0

@FOR	%%F	IN (easy-buoy.ini buoy.bmp buoy.gif buoy.rgn) DO (
	@COPY	/Y	bin\%%F	Debug\
	@IF	NOT	ERRORLEVEL		0			@PAUSE
	@COPY	/Y	bin\%%F	Release\
	@IF	NOT	ERRORLEVEL		0			@PAUSE
	@COPY	/Y	bin\%%F	x64\Debug\
	@IF	NOT	ERRORLEVEL		0			@PAUSE
	@COPY	/Y	bin\%%F	x64\Release\
	@IF	NOT	ERRORLEVEL		0			@PAUSE
)

@popd
@echo	on