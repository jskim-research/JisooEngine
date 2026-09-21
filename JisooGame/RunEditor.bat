@echo off
setlocal

for %%I in ("%~dp0..") do set "ROOT_DIR=%%~fI"
set "BUILD_DIR=%ROOT_DIR%\Build\VS2022-x64"
set "SOLUTION_PATH=%BUILD_DIR%\JisooEngine.sln"
set "CONFIGURATION=%~1"

if "%CONFIGURATION%"=="" set "CONFIGURATION=Debug"

if /I "%CONFIGURATION%"=="Debug" (
    set "CONFIGURATION=Debug"
) else if /I "%CONFIGURATION%"=="Release" (
    set "CONFIGURATION=Release"
) else (
    echo [Error] Unsupported configuration: %CONFIGURATION%
    echo Usage: RunEditor.bat [Debug^|Release]
    exit /b 1
)

if not exist "%SOLUTION_PATH%" (
    echo Visual Studio project files were not found. Generating them now...
    call "%ROOT_DIR%\GenerateProjectFiles.bat"
    if errorlevel 1 exit /b 1
)

echo Building JisooGameEditor ^(%CONFIGURATION%^)...
cmake --build "%BUILD_DIR%" --config "%CONFIGURATION%" --target JisooGameEditor
if errorlevel 1 (
    echo [Error] Editor build failed.
    exit /b 1
)

set "EDITOR_PATH=%ROOT_DIR%\JisooGame\Binaries\Win64\%CONFIGURATION%\JisooGameEditor.exe"
if not exist "%EDITOR_PATH%" (
    echo [Error] Editor executable was not found: %EDITOR_PATH%
    exit /b 1
)

echo Starting JisooGameEditor...
pushd "%ROOT_DIR%"
start "" "%EDITOR_PATH%"
set "START_RESULT=%ERRORLEVEL%"
popd

if not "%START_RESULT%"=="0" (
    echo [Error] Failed to start JisooGameEditor.
    exit /b 1
)

endlocal
