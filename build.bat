@echo off
setlocal

set BUILD_DIR=build
set BINARY=%BUILD_DIR%\bin\CodeRunner.exe

if "%1"=="" goto build
if "%1"=="build" goto build
if "%1"=="run" goto run
if "%1"=="clean" goto clean
if "%1"=="rebuild" goto rebuild
goto usage

:build
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"
cmake ..
cmake --build .
cd ..
echo Build complete: %BINARY%
goto end

:run
if not exist "%BINARY%" (
    echo Binary not found, building first...
    call "%~f0" build
)
echo Running CodeRunner...
"%BINARY%"
goto end

:clean
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
echo Build directory cleaned.
goto end

:rebuild
call "%~f0" clean
call "%~f0" build
goto end

:usage
echo Usage: build.bat [build^|run^|clean^|rebuild]
echo   build   - Configure and compile (default)
echo   run     - Build if needed, then launch
echo   clean   - Remove build directory
echo   rebuild - Clean + build
goto end

:end
endlocal
