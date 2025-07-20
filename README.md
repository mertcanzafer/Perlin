# Perlin
## **Recommendations:**
- Keep in mind that I will be using graphics pipeline in this project. Therefore, make sure that you have one of the SDKs that SDL3 uses for its rendering backend. Vulkan (SDK) or DirectX (SDK).
- Download Vulkan SDK: (https://www.lunarg.com/vulkan-sdk/)
- Download DirectX SDK: (https://www.microsoft.com/en-us/download/details.aspx?id=6812)
  
**Note: If you do not work on Windows, then there is no problem. You can have Vulkan SDK that works on both Windows and Linux, a cross platform API**

## ⚠️ **Requirements:**
Downloading NASM compiler: SDL_image depends on third-party codecs to decode image/video formats.
### Step 1: Download NASM
  
Go to the official site:
👉 https://www.nasm.us/pub/nasm/releasebuilds/

Choose the latest version → click the Windows or Linux installer link:
### Step 2: Run the installer
- For windows, Double-click the .exe installer
- Install the NASM to a specific directory.
### Step 3: Add NASM to your system PATH
- Use system environment variables to add NASM.
- Verify that you installed NASM
### Step 4: Open a new terminal and run
``` bash
nasm -v
```
If NASM is correctly installed, then you are ready to build the project using CMAKE


## How to Build

1. Clone the repository
 ```bash
       git clone --recurse-submodules https://github.com/mertcanzafer/Perlin.git
```
2. Use CMAKE for building the project. Create a build folder in root dir.
   ```bash
        mkdir build
        cd build
   ```
3. Inside the build folder, type cmake to build your project
   ```basg
     cmake ..
   ```
4. In build folder you can open the project if it's done building
5. Run the project.

### Using Binaries

#### Windows Setup
1. Clone the repository
 ```bash
 git clone --recurse-submodules https://github.com/mertcanzafer/Perlin.git
```
2. Navigate to Srcipts folder
3. Using terminal run batch file
    ```basg
     Setup-Windows.bat
   ```
4. In build folder you can open the project if it's done building
5. Run the project.

#### Linux Setup
1. Clone the repository
 ```bash
 git clone --recurse-submodules https://github.com/mertcanzafer/Perlin.git
```
2. Navigate to Srcipts folder
3. Using terminal run sh file
    ```basg
     chmod +x Scripts/Setup-Linux.sh
     ./Scripts/Setup-Linux.sh
   ```
4. In build folder you can open the project if it's done building
5. Run the project.
