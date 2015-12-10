@echo off

set windbg=0

set /P ver="Enter version number: "
set filename="tortilla_%ver%.zip"
set sdk="sdk_%ver%.zip"

set prod=Release
if %windbg% == 1 set prod=Debug

del *.zip /q

md tortilla
cd tortilla
md gamedata
md help
md modules
md plugins
md resources
cd resources
md sound
cd ..
cd ..

md sdk

xcopy gamedata\*.* tortilla\gamedata /E /Y
xcopy ..\help\*.* tortilla\help /Y

xcopy ..\modules\system.dll tortilla\modules /Y
xcopy ..\modules\rnd.dll tortilla\modules /Y
xcopy ..\modules\bass.dll tortilla\modules /Y
xcopy ..\modules\bass_fx.dll tortilla\modules /Y
xcopy ..\modules\lbass.dll tortilla\modules /Y
xcopy ..\modules\modules.lua tortilla\modules /Y
xcopy ..\modules\soundplayer.lua tortilla\modules /Y

xcopy ..\plugins\jmc3import.dll tortilla\plugins /Y
xcopy ..\plugins\tray.dll tortilla\plugins /Y
xcopy ..\plugins\prompt.dll tortilla\plugins /Y
xcopy ..\plugins\clickpad.dll tortilla\plugins /Y
xcopy ..\plugins\sound.dll tortilla\plugins /Y
xcopy ..\plugins\send.lua tortilla\plugins /Y
xcopy ..\plugins\statusbar.lua tortilla\plugins /Y
xcopy ..\plugins\autowrap.lua tortilla\plugins /Y
xcopy ..\plugins\reconnect.lua tortilla\plugins /Y
xcopy ..\plugins\historyfilter.lua tortilla\plugins /Y
xcopy ..\plugins\colorgamecmd.lua tortilla\plugins /Y
xcopy ..\plugins\bell.lua tortilla\plugins /Y
xcopy ..\resources\clickpad\*.* tortilla\resources\clickpad\ /E /Y

xcopy ..\%prod%\tortilla.exe tortilla /Y
xcopy ..\%prod%\api.dll tortilla /Y
xcopy ..\%prod%\lua.dll tortilla /Y
if %windbg% == 1 xcopy windbg\dbghelp.dll tortilla /Y
xcopy ..\mudclient\readme.txt tortilla /Y
xcopy ..\mudclient\changelog.txt tortilla /Y

xcopy ..\sdk\*.* sdk /E /Y
xcopy ..\modules\modules.txt sdk /Y

7za.exe a -tzip %filename% tortilla
7za.exe a -tzip %sdk% sdk -xr!.gitignore

rd tortilla /s /q
rd sdk /s /q

pause