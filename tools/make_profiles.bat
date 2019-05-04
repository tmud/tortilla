@echo off

md profiles
xcopy ..\resources\profiles\*.* profiles\ /E /Y
cd profiles
..\7za.exe a -mcu -tzip ..\profiles.pak *
cd ..
rd profiles /s /q
move profiles.pak ..\resources

pause