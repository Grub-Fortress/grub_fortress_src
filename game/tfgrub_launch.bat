@echo off
title Grub Fortress Launcher

set "EXECUTABLE=tfgrub_win64.exe"

:menu
cls
echo ===============================
echo    Grub Fortress Launcher
echo ===============================
echo.
echo 1) Default
echo 2) Devmode
echo 3) Devmode (Devtest map)
echo 4) LUX Disabled
echo 5) Insecure
echo 6) Steam
echo 7) No Staging Items
echo 8) Experimental Items
echo 9) Tools
echo.
echo Q) Quit
echo.

set /p choice=Select an option: 

if /i "%choice%"=="1" goto option1
if /i "%choice%"=="2" goto option2
if /i "%choice%"=="3" goto option3
if /i "%choice%"=="4" goto option4
if /i "%choice%"=="5" goto option5
if /i "%choice%"=="6" goto option6
if /i "%choice%"=="7" goto option7
if /i "%choice%"=="8" goto option8
if /i "%choice%"=="9" goto option9
if /i "%choice%"=="Q" exit

goto menu

:option1
start "" "%EXECUTABLE%"
goto end

:option2
start "" "%EXECUTABLE%" -dev -multirun -novid +mp_disable_respawn_times 1 +maxplayers 25
goto end

:option3
start "" "%EXECUTABLE%" -dev -multirun -novid +mp_disable_respawn_times 1 +maxplayers 25 +map devtest
goto end

:option4
start "" "%EXECUTABLE%" -nolux
goto end

:option5
start "" "%EXECUTABLE%" -novid -insecure
goto end

:option6
start "" "%EXECUTABLE%" -novid -steam
goto end

:option7
start "" "%EXECUTABLE%" -nostaging_items
goto end

:option8
start "" "%EXECUTABLE%" -experimental_items
goto end

:option9
start "" "%EXECUTABLE%" -tools -nop4 +sv_lan 1 +map devtest +mp_disable_respawn_times 1
goto end

:end
echo.
echo Press any key to return to menu...
pause >nul
goto menu
