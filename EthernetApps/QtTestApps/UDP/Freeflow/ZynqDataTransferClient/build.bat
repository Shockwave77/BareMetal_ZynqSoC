@echo off
echo Building ZynqDataTransferClient...

REM Clean previous build
if exist build rmdir /s /q build
mkdir build
cd build

REM Try to find Qt installation
set QT_PATH=
for /d %%i in ("C:\Qt\6.*") do (
    for /d %%j in ("%%i\mingw_64") do (
        if exist "%%j\bin\qmake.exe" set QT_PATH=%%j
    )
)

if "%QT_PATH%"=="" (
    echo Qt installation not found in C:\Qt\
    echo Please install Qt or set QT_PATH manually
    echo.
    echo Using Qt Creator is recommended:
    echo 1. Open Qt Creator
    echo 2. Open ZynqDataTransferClient.pro
    echo 3. Configure and build
    pause
    exit /b 1
)

echo Found Qt at: %QT_PATH%

REM Configure with CMake
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="%QT_PATH%" ..

if %errorlevel% neq 0 (
    echo CMake configuration failed
    echo Try using Qt Creator instead
    pause
    exit /b 1
)

REM Build the project
cmake --build .

if %errorlevel% neq 0 (
    echo Build failed
    pause
    exit /b 1
)

echo Build successful!
echo Executable: %cd%\Debug\ZynqDataTransferClient.exe
pause
