#!/bin/bash

echo "Initializing submodules..."
git submodule update --init --recursive

echo "Creating build directory..."
mkdir -p build
cd build

echo "Running CMake..."
cmake ..

echo "Setup complete. You can now build the project."
