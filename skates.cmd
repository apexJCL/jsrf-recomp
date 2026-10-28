@echo off
rem skates: the project CLI (skates.py). Python 3.9 or newer.
where py >nul 2>nul
if %errorlevel%==0 (
    py -3 "%~dp0skates.py" %*
) else (
    python "%~dp0skates.py" %*
)
exit /b %errorlevel%
