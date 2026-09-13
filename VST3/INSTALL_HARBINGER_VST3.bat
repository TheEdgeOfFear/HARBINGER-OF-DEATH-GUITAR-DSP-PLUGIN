@echo off
setlocal
echo =========================================================================
echo  HARBINGER OF DEATH - VST3 System Installer / Updater
echo  THE EDGE OF FEAR
echo =========================================================================
echo.

net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Administrator privileges required to update C:\Program Files\Common Files\VST3.
    echo Requesting elevation...
    powershell -NoProfile -ExecutionPolicy Bypass -Command "Start-Process cmd -ArgumentList '/c \"\"%~f0\"\"' -Verb RunAs"
    exit /b
)

echo Cleaning old VST3 files from C:\Program Files\Common Files\VST3\HARBINGER OF DEATH.vst3...
if exist "C:\Program Files\Common Files\VST3\HARBINGER OF DEATH.vst3" (
    rd /s /q "C:\Program Files\Common Files\VST3\HARBINGER OF DEATH.vst3"
)

echo Deploying latest HARBINGER OF DEATH.vst3...
robocopy "%~dp0HARBINGER OF DEATH.vst3" "C:\Program Files\Common Files\VST3\HARBINGER OF DEATH.vst3" /E /IS /IT /NFL /NDL

if exist "C:\Program Files\Common Files\VST3\HARBINGER OF DEATH.vst3\Contents\x86_64-win\HARBINGER OF DEATH.vst3" (
    echo.
    echo [SUCCESS] HARBINGER OF DEATH.vst3 successfully updated in C:\Program Files\Common Files\VST3!
    echo Rescan plugins in your DAW or restart your DAW now.
) else (
    echo.
    echo [WARNING] Please check that HARBINGER OF DEATH.vst3 exists next to this script.
)

echo.
pause
