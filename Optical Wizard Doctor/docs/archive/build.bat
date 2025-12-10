@echo off
REM Build script for Windows
REM Usage: build.bat [Debug|Release]

setlocal enabledelayedexpansion

REM Default build type
set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

REM Validate build type
if not "%BUILD_TYPE%"=="Debug" if not "%BUILD_TYPE%"=="Release" (
    echo Error: Build type must be 'Debug' or 'Release'
    exit /b 1
)

echo Building OWD in %BUILD_TYPE% mode...

REM Create build directory
set BUILD_DIR=build
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM Configure with CMake
cmake -B "%BUILD_DIR%" ^
    -S . ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE%

REM Build
cmake --build "%BUILD_DIR%" --config %BUILD_TYPE%

echo.
echo Build complete! Executable: %BUILD_DIR%\bin\owd.exe
echo.
echo To run: %BUILD_DIR%\bin\owd.exe

endlocal

