@echo off
REM Run from repository root: c:\Users\stanc\github\VST3-Open-Valhalla

echo --- PATH ---
echo %PATH%
echo.

echo --- where cmake (may fail if not on PATH) ---
where cmake || echo "where cmake failed"
echo.

echo --- try common absolute paths for cmake.exe ---
if exist "C:\Program Files\CMake\bin\cmake.exe" (
  set "CMAKE_EXE=C:\Program Files\CMake\bin\cmake.exe"
) else if exist "C:\Program Files (x86)\CMake\bin\cmake.exe" (
  set "CMAKE_EXE=C:\Program Files (x86)\CMake\bin\cmake.exe"
) else (
  echo "cmake not found at common locations, using PATH cmake"
  set "CMAKE_EXE=cmake"
)

"%CMAKE_EXE%" --version

echo.
echo --- Configure (Visual Studio generator, x64) ---
"%CMAKE_EXE%" -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
  echo Configure with Visual Studio generator failed.
  echo This usually means Visual Studio 2022 with C++ tools is not installed.
  echo Cleaning previous CMake cache and trying Ninja generator...
  if exist build (
    rmdir /S /Q build
  )
  "%CMAKE_EXE%" -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
)

echo.
echo --- Build Release ---
"%CMAKE_EXE%" --build build --config Release
echo.
echo --- Done ---
