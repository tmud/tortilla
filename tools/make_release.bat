@echo off

set modules=modules\system.dll
set plugins=plugins\prompt.dll plugins\jmc3import.dll plugins\historyfilter.lua plugins\autowrap.lua plugins\statusbar.lua plugins\reconnect.lua plugins\colorgamecmd.lua

set /P ver="Enter version number: "
set filename="tortilla_%ver%.zip"
set sdk="sdk_%ver%.zip"

del *.zip > nul
copy ..\Release\tortilla.exe .\ > nul
copy ..\Release\lua.dll .\ > nul
copy ..\Release\api.dll .\ > nul
copy ..\mudclient\changelog.txt .\ > nul

7za.exe a -tzip %filename% tortilla.exe lua.dll api.dll changelog.txt gamedata\* -xr!.gitignore
del tortilla.exe lua.dll api.dll changelog.txt > nul
cd ..
tools\7za.exe a -r -tzip tools\%filename% help\* %plugins% %modules% -xr!.gitignore
tools\7za.exe a -tzip tools\%sdk% sdk\* -xr!.gitignore
cd tools
mkdir tortilla
7za.exe x -tzip -otortilla %filename%
del %filename%
7za.exe a -tzip %filename% tortilla\*
rd /s /q tortilla
echo Finished.
pause
