@echo off

del Logs\*.log

echo Building GameCQ...
"%DEVENV%" ..\..\GameCQ\GameCQ.sln /build Debug /out Logs\GameCQ.log
if errorlevel 1 goto :Error

goto :Done

:Error
echo Error occured...

:Done
echo Done...
pause

