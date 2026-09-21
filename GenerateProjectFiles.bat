@echo off
setlocal

for %%I in ("%~dp0.") do set "ROOT_DIR=%%~fI"
set "BUILD_DIR=%ROOT_DIR%\Intermediate\Build\VS2022-x64"

where cmake >nul 2>&1
if errorlevel 1 (
    echo [Error] CMake was not found in PATH.
    exit /b 1
)

cmake -S "%ROOT_DIR%" -B "%BUILD_DIR%" -G "Visual Studio 17 2022" -A x64 %*
if errorlevel 1 (
    echo [Error] Failed to generate Visual Studio project files.
    exit /b 1
)

echo.
echo Project generation completed.
echo Solution: %BUILD_DIR%\JisooEngine.sln

endlocal
