@echo off

set modules=modules\dummy.txt
set plugins=plugins\prompt.dll plugins\jmc3import.dll

set /P ver="Enter version number: "
set filename="tortilla_%ver%.zip"

del *.zip > nul
copy ..\Release\tortilla.exe .\ > nul
copy ..\Release\lua.dll .\ > nul
copy ..\Release\api.dll .\ > nul
copy ..\mudclient\readme.txt .\ > nul

7za.exe a -tzip %filename% tortilla.exe lua.dll api.dll readme.txt gamedata\*
del tortilla.exe lua.dll api.dll readme.txt > nul
cd ..
tools\7za.exe a -r -tzip tools\%filename% help\* %plugins% %modules%
cd tools




