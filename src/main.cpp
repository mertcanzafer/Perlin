#include <iostream>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>
#include "Exception.h"
#include <array>
#include <string>

SDL_GPUShader* LoadShader(SDL_GPUDevice* device,
    std::string& shaderFileName,
    Uint32 sampleCount,
    Uint32 uniformBufferCount,
    Uint32 storageBufferCount,
    Uint32 storageTextureCount);


int main(int argc, char* args[])
{
    const unsigned int width = 800u;
	const unsigned int height = 600u;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		throw dbg::SDL_Exception("Failed to initialize SDL");
    }

	SDL_Window* window = SDL_CreateWindow(
        "SDL Window",width, height,SDL_WINDOW_RESIZABLE);
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

	//std::clog << "Using GPU device: " << SDL_GetGPUDeviceDriver(gpuDevice) << std::endl;

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
			case SDL_EVENT_WINDOW_RESIZED:
				// Handle window resize event
                break;
            default: break;
            }
        }
        // Set command buffer for GPU device
		SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(gpuDevice);
        if (!commandBuffer) {
            throw dbg::SDL_Exception("Failed to acquire GPU command buffer");
		}

        // Create a swapchain of textures
        SDL_GPUTexture* swapchain;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window, &swapchain, nullptr, nullptr))
        {
			throw dbg::SDL_Exception("Failed to acquire swapchain texture");
        }
       
        if (swapchain != nullptr)
        {
            std::array<SDL_GPUColorTargetInfo, 1> colorTargetInfos{};
            colorTargetInfos[0].texture = swapchain;
			colorTargetInfos[0].clear_color.r = 0.3f;
			colorTargetInfos[0].clear_color.g = 0.4f;
			colorTargetInfos[0].clear_color.b = 0.5f;
			colorTargetInfos[0].clear_color.a = 1.0f;
            colorTargetInfos[0].load_op = SDL_GPU_LOADOP_CLEAR;
            colorTargetInfos[0].store_op = SDL_GPU_STOREOP_STORE;

            SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer,colorTargetInfos.data(),
                (Uint32)colorTargetInfos.size(), nullptr);
			SDL_EndGPURenderPass(renderPass);
        }

        if (!SDL_SubmitGPUCommandBuffer(commandBuffer))
        {
			throw dbg::SDL_Exception("Failed to submit GPU command buffer");
        }
	}

	SDL_ReleaseWindowFromGPUDevice(gpuDevice, window);
    SDL_DestroyWindow(window);
    SDL_DestroyGPUDevice(gpuDevice);
	SDL_Quit();

    return 0;
}

//SDL_GPUShader* LoadShader
//(
//    SDL_GPUDevice* device, std::string& shaderFileName, Uint32 sampleCount, Uint32 uniformBufferCount, 
//    Uint32 storageBufferCount, Uint32 storageTextureCount)
//{
//    // Auto-detect the shader stage from the file name for convenience
//    SDL_GPUShaderStage stage{};
//    if(SDL_strstr(shaderFileName.c_str(), ".vert"))
//    {
//        stage = SDL_GPU_SHADERSTAGE_VERTEX;
//    }
//    else if(SDL_strstr(shaderFileName.c_str(), ".frag"))
//    {
//        stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
//    }
//    else
//    {
//        throw dbg::SDL_Exception("Unsupported shader file extension");
//	}
//
//	char fullPath[256] = { 0 };
//	SDL_GPUShaderFormat backendFormat = SDL_GetGPUShaderFormats(device);
//    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
//
//	const char* entrypoint = nullptr;
//    if(backendFormat & SDL_GPU_SHADERFORMAT_SPIRV)
//    {
//        format = SDL_GPU_SHADERFORMAT_SPIRV;
//        entrypoint = "main";
//    }
//    else if (backendFormat & SDL_GPU_SHADERFORMAT_DXIL)
//    {
//        format = SDL_GPU_SHADERFORMAT_DXIL;
//        entrypoint = "main";
//    }
//    else if (backendFormat & SDL_GPU_SHADERFORMAT_MSL)
//    {
//        format = SDL_GPU_SHADERFORMAT_MSL;
//        entrypoint = "main0";
//    }
//    else
//    {
//		throw dbg::SDL_Exception("No supported shader formats found");
//
//    return nullptr;
//}
