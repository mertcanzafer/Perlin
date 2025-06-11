@echo off
echo Initializing submodules...
git submodule update --init --recursive

cd ..
echo Switched to repo root: %CD%

echo Creating build directory...
mkdir build
cd build

echo Running CMake...
cmake ..

echo Done! You can now build the project.
pause
