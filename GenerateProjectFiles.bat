@echo off
setlocal

for %%I in ("%~dp0.") do set "ROOT_DIR=%%~fI"
set "PROJECT_FILES_DIR=%ROOT_DIR%\Intermediate\ProjectFiles"

where cmake >nul 2>&1
if errorlevel 1 (
    echo [Error] CMake was not found in PATH.
    exit /b 1
)

cmake -S "%ROOT_DIR%" -B "%PROJECT_FILES_DIR%" -A x64 %*
if errorlevel 1 (
    echo [Error] Failed to generate Visual Studio project files.
    exit /b 1
)

echo.
echo Project generation completed.
echo Solution: %PROJECT_FILES_DIR%\JisooEngine.sln

endlocal
