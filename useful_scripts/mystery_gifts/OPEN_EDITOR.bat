@echo off
cd /d "%~dp0"
where py >nul 2>nul
if %errorlevel%==0 (
  py editor_mystery_gift.py
) else (
  python editor_mystery_gift.py
)
if errorlevel 1 pause
