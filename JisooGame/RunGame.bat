@echo off
setlocal

for %%I in ("%~dp0..") do set "ROOT_DIR=%%~fI"
set "CONFIGURATION=%~1"

if "%CONFIGURATION%"=="" set "CONFIGURATION=Debug"

if /I "%CONFIGURATION%"=="Debug" (
    set "CONFIGURATION=Debug"
) else if /I "%CONFIGURATION%"=="Release" (
    set "CONFIGURATION=Release"
) else (
    echo [Error] Unsupported configuration: %CONFIGURATION%
    echo Usage: RunGame.bat [Debug^|Release]
    exit /b 1
)

call "%~dp0BuildGame.bat" "%CONFIGURATION%"
if errorlevel 1 exit /b 1

set "GAME_PATH=%ROOT_DIR%\JisooGame\Binaries\Win64\%CONFIGURATION%\JisooGame.exe"
if not exist "%GAME_PATH%" (
    echo [Error] Game executable was not found: %GAME_PATH%
    exit /b 1
)

echo Starting JisooGame...
pushd "%ROOT_DIR%"
start "" "%GAME_PATH%"
set "START_RESULT=%ERRORLEVEL%"
popd

if not "%START_RESULT%"=="0" (
    echo [Error] Failed to start JisooGame.
    exit /b 1
)

endlocal
