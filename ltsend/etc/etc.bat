IF NOT "%1"=="--git-build" GOTO AFTER_GIT_BUILD;

..\buildscripts\signtool.exe sign ..\builds\bin\client\ltsend.exe
IF %ERRORLEVEL% NEQ 0 (exit 1)

:AFTER_GIT_BUILD

"C:\\Program Files\\Git\\bin\\sh.exe" ./etc/etc.sh
IF %ERRORLEVEL% NEQ 0 (exit 1)
