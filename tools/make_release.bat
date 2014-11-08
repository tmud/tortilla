@echo off

set modules=modules\system.dll
set plugins=plugins\prompt.dll plugins\jmc3import.dll plugins\historyfilter.lua plugins\autowrap.lua

set /P ver="Enter version number: "
set filename="tortilla_%ver%.zip"
set sdk="sdk_%ver%.zip"

del *.zip > nul
copy ..\Release\tortilla.exe .\ > nul
copy ..\Release\lua.dll .\ > nul
copy ..\Release\api.dll .\ > nul
copy ..\mudclient\changelog.txt .\ > nul

7za.exe a -tzip %filename% tortilla.exe lua.dll api.dll changelog.txt gamedata\*
del tortilla.exe lua.dll api.dll changelog.txt > nul
cd ..
tools\7za.exe a -r -tzip tools\%filename% help\* %plugins% %modules%
tools\7za.exe a -tzip tools\%sdk% sdk\*
cd tools
echo Finished.
pause
