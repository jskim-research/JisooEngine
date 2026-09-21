@echo off
setlocal

for %%I in ("%~dp0.") do set "ROOT_DIR=%%~fI"
set "SOLUTION_PATH=%ROOT_DIR%\Build\VS2022-x64\JisooEngine.sln"

if not exist "%SOLUTION_PATH%" (
    echo Visual Studio project files were not found. Generating them now...
    call "%ROOT_DIR%\GenerateProjectFiles.bat"
    if errorlevel 1 exit /b 1
)

echo Opening JisooEngine in Visual Studio...
start "" "%SOLUTION_PATH%"
if errorlevel 1 (
    echo [Error] Failed to open Visual Studio.
    exit /b 1
)

endlocal
