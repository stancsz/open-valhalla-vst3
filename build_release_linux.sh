#!/bin/bash
set -e

# Ensure release directory exists
mkdir -p release

# Clean build directory
if [ -d "build" ]; then
    echo "Cleaning build directory..."
    rm -rf build
fi

echo "Configuring CMake..."
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

echo "Building..."
cmake --build build --config Release

echo "Packaging..."
mkdir -p build/package

# Copy VST3
if [ -d "build/FDNR_artefacts/Release/VST3" ]; then
    cp -r "build/FDNR_artefacts/Release/VST3" build/package/
elif [ -d "build/FDNR_artefacts/VST3" ]; then
    cp -r "build/FDNR_artefacts/VST3" build/package/
else
    echo "Error: VST3 artifact not found!"
    exit 1
fi

# Copy Standalone
if [ -d "build/FDNR_artefacts/Release/Standalone" ]; then
    cp -r "build/FDNR_artefacts/Release/Standalone" build/package/
elif [ -d "build/FDNR_artefacts/Standalone" ]; then
    cp -r "build/FDNR_artefacts/Standalone" build/package/
else
    echo "Warning: Standalone artifact not found."
fi

# Zip
cd build/package
zip -r ../../release/FDNR_VST3_Linux.zip .
cd ../..

echo "Build complete: release/FDNR_VST3_Linux.zip"
