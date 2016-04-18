@echo off
@pushd	%~dp0

@COPY	/Y	Release\windesk-buoy.x86.exe
@COPY	/Y	x64\Release\windesk-buoy.amd64.exe

@popd
@echo	on