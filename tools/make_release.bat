@echo off

set windbg=0

set modules=modules\system.dll modules\modules.lua modules\rnd.dll modules\modules.txt
set plugins=plugins\prompt.dll plugins\jmc3import.dll plugins\historyfilter.lua plugins\autowrap.lua plugins\statusbar.lua plugins\reconnect.lua plugins\colorgamecmd.lua
set plugins2=plugins\tray.dll

set /P ver="Enter version number: "
set filename="tortilla_%ver%.zip"
set sdk="sdk_%ver%.zip"

del *.zip > nul

set prod=Release
set dbg=
if %windbg% == 1 set prod=Debug
if %windbg% == 1 copy windbg\dbghelp.dll .\ >nul
if %windbg% == 1 set dbg=dbghelp.dll

copy ..\%prod%\tortilla.exe .\ > nul
copy ..\%prod%\lua.dll .\ > nul
copy ..\%prod%\api.dll .\ > nul
copy ..\mudclient\changelog.txt .\ > nul

7za.exe a -tzip %filename% tortilla.exe lua.dll api.dll changelog.txt %dbg% gamedata\* -xr!.gitignore
del tortilla.exe lua.dll api.dll changelog.txt %dbg% > nul
cd ..
tools\7za.exe a -r -tzip tools\%filename% help\* %plugins% %plugins2% %modules% -xr!.gitignore
tools\7za.exe a -tzip tools\%sdk% sdk\* -xr!.gitignore
cd tools
mkdir tortilla
7za.exe x -tzip -otortilla %filename%
del %filename%
7za.exe a -tzip %filename% tortilla\*
rd /s /q tortilla
echo Finished.
pause
