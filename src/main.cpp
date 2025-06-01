#include <iostream>
#include <SDL3/SDL_gpu.h>
#include "Exception.h"

int main(int argc, char* args[])
{
    const unsigned int width = 800u;
	const unsigned int height = 600u;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		throw dbg::SDL_Exception("Failed to initialize SDL");
    }

	SDL_Window* window = SDL_CreateWindow(
        "SDL Window",width, height, 0);
    if(!window) {
        throw dbg::SDL_Exception("Failed to create window");
	}

	auto* gpuDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV |
        SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL 
        , true, nullptr);

    if (!SDL_ClaimWindowForGPUDevice(gpuDevice, window))
    {
		throw dbg::SDL_Exception("Failed to claim window for GPU device");
    }

	SDL_ShowWindow(window);

    SDL_Event event;
	bool running = true;
    
	// !Game loop
    while(running) {
		// !Handle events
        while (SDL_PollEvent(&event)) {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
				running = false;
                break;
            default: break;
            }
        }
	}


	SDL_DestroyWindow(window);
	SDL_Quit();

    return 0;
}