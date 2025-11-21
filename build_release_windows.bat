@echo off
setlocal

if not exist "release" mkdir release
if exist "build" (
    echo Cleaning build directory...
    rmdir /s /q build
)

echo Configuring CMake...
cmake -B build -S .
if %errorlevel% neq 0 (
    echo CMake configuration failed.
    exit /b %errorlevel%
)

echo Building...
cmake --build build --config Release
if %errorlevel% neq 0 (
    echo Build failed.
    exit /b %errorlevel%
)

echo Packaging...
if not exist "build\package" mkdir build\package

if exist "build\VST3OpenValhalla_artefacts\Release\VST3" (
    xcopy "build\VST3OpenValhalla_artefacts\Release\VST3" "build\package\VST3\" /E /I /Y
) else (
    echo Error: VST3 artifact not found!
    exit /b 1
)

if exist "build\VST3OpenValhalla_artefacts\Release\Standalone" (
    xcopy "build\VST3OpenValhalla_artefacts\Release\Standalone" "build\package\Standalone\" /E /I /Y
) else (
    echo Warning: Standalone artifact not found.
)

echo Zipping...
if exist "release\Open_Valhalla_VST3_Windows.zip" del "release\Open_Valhalla_VST3_Windows.zip"
powershell -command "Compress-Archive -Path 'build\package\*' -DestinationPath 'release\Open_Valhalla_VST3_Windows.zip' -Force"

echo Build complete: release\Open_Valhalla_VST3_Windows.zip
endlocal
