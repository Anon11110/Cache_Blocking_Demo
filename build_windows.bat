@echo off
REM Build script for Windows
REM Requires: CMake, Visual Studio, Vulkan SDK

echo ======================================
echo Building Cache Blocking Demo (Windows)
echo ======================================

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo.
echo Configuring with CMake...
cmake -G "Visual Studio 17 2022" -A x64 ..

if errorlevel 1 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

REM Build the project
echo.
echo Building project...
cmake --build . --config Release

if errorlevel 1 (
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo ======================================
echo Build successful!
echo Executable: build\bin\Release\CacheBlockingDemo.exe
echo ======================================
pause
