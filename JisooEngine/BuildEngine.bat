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
    echo Usage: BuildEngine.bat [Debug^|Release]
    exit /b 1
)

if not exist "%SOLUTION_PATH%" (
    echo Visual Studio project files were not found. Generating them now...
    call "%ROOT_DIR%\GenerateProjectFiles.bat"
    if errorlevel 1 exit /b 1
)

echo Building JisooEngine modules ^(%CONFIGURATION%^)...
cmake --build "%BUILD_DIR%" --config "%CONFIGURATION%" --target JisooEngineEditor
if errorlevel 1 (
    echo [Error] Engine build failed.
    exit /b 1
)

echo.
echo Engine modules built successfully.
echo Libraries: %ROOT_DIR%\JisooEngine\Intermediate\Libraries\Win64\%CONFIGURATION%

endlocal
