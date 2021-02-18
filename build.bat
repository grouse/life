@ECHO off

set start_time=%time%

SET ROOT=%~dp0
SET BUILD_DIR=%ROOT%\build

REM if NOT "%Platform%" == "X64" IF NOT "%Platform%" == "x64" (call "%ROOT%\setup_cl_x64.bat")

SET DEFINITIONS=-DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS

SET WARNINGS=-Wall -Wextra -Wno-missing-braces
SET FLAGS=%DEFINITIONS% %WARNINGS% -gcodeview -g -fno-exceptions -fno-rtti -std=c++17 -fno-color-diagnostics

if not exist %BUILD_DIR% mkdir %BUILD_DIR%

SET RAYLIB_DIR=%ROOT%\external\raylib

SET INCLUDE_DIR=-I%RAYLIB_DIR%\include
SET LIBS=-l%RAYLIB_DIR%\lib\raylib.lib -lgdi32.lib -luser32.lib

PUSHD %BUILD_DIR%
@ECHO on
clang++ -O0 %FLAGS% %INCLUDE_DIR% %LIBS% -o life.exe %ROOT%\life.cpp
@ECHO off
POPD

set end_time=%time%

set options="tokens=1-4 delims=:.,"
for /f %options% %%a in ("%start_time%") do set start_h=%%a&set /a start_m=100%%b %% 100&set /a start_s=100%%c %% 100&set /a start_ms=100%%d %% 100
for /f %options% %%a in ("%end_time%") do set end_h=%%a&set /a end_m=100%%b %% 100&set /a end_s=100%%c %% 100&set /a end_ms=100%%d %% 100

set /a hours=%end_h%-%start_h%
set /a mins=%end_m%-%start_m%
set /a secs=%end_s%-%start_s%
set /a ms=%end_ms%-%start_ms%
if %ms% lss 0 set /a secs = %secs% - 1 & set /a ms = 100%ms%
if %secs% lss 0 set /a mins = %mins% - 1 & set /a secs = 60%secs%
if %mins% lss 0 set /a hours = %hours% - 1 & set /a mins = 60%mins%
if %hours% lss 0 set /a hours = 24%hours%
if 1%ms% lss 100 set ms=0%ms%

set /a totalsecs = %hours%*3600 + %mins%*60 + %secs%
echo compilation took %totalsecs%.%ms% seconds
