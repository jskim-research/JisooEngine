@echo off
setlocal

for %%I in ("%~dp0.") do set "ROOT_DIR=%%~fI"
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
    echo Usage: RunTests.bat [Debug^|Release]
    exit /b 1
)

where cmake >nul 2>&1
if errorlevel 1 (
    echo [Error] CMake was not found in PATH.
    exit /b 1
)

where ctest >nul 2>&1
if errorlevel 1 (
    echo [Error] CTest was not found in PATH.
    exit /b 1
)

if not exist "%SOLUTION_PATH%" (
    echo Visual Studio project files were not found. Generating them now...
    call "%ROOT_DIR%\GenerateProjectFiles.bat" -DBUILD_TESTING=ON
    if errorlevel 1 exit /b 1
)

echo Building test targets ^(%CONFIGURATION%^) ...
cmake --build "%BUILD_DIR%" --config "%CONFIGURATION%" --target JisooObjectSystemTests
if errorlevel 1 (
    echo [Error] Test build failed.
    exit /b 1
)

echo.
echo Running tests ^(%CONFIGURATION%^) ...
ctest --test-dir "%BUILD_DIR%" -C "%CONFIGURATION%" --output-on-failure
if errorlevel 1 (
    echo [Error] Tests failed.
    exit /b 1
)

echo.
echo All tests passed.

endlocal
