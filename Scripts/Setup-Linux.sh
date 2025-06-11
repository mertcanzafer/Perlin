#!/bin/bash

cd "$(dirname "$0")"

cd ..

echo "Initializing submodules..."
git submodule update --init --recursive

echo "Creating build directory in repo root..."
mkdir -p build
cd build

echo "Running CMake..."
cmake ..

echo "Setup complete. You can now build the project."
