#include <iostream>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

#include "Exception.h"
#include <array>
#include <string>
#include <vector>
#include <fstream>

SDL_GPUShader* LoadShader(SDL_GPUDevice* device,
    std::string& shaderFileName,
    Uint32 sampleCount,
    Uint32 uniformBufferCount,
    Uint32 storageBufferCount,
    Uint32 storageTextureCount);


const char* BasePath = nullptr;

void InitializeAssetLoader();

int main(int argc, char* args[])
{
    const unsigned int width = 800u;
	const unsigned int height = 600u;
	InitializeAssetLoader();

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

	SDL_GPUShader* vertexShader = LoadShader(gpuDevice, std::string("RawTriangle.vert"), 0, 0, 0, 0);

    if (!vertexShader) {
		throw dbg::SDL_Exception("Failed to load shader");
    }

	SDL_GPUShader* fragmentShader = LoadShader(gpuDevice, std::string("SolidColor.frag"), 0, 0, 0, 0);

    if(!fragmentShader) {
        throw dbg::SDL_Exception("Failed to load fragment shader");
	}

    //! Create the pipelines
    
    std::vector<SDL_GPUColorTargetDescription> colorTargetDescriptions(1);

    colorTargetDescriptions[0].format = SDL_GetGPUSwapchainTextureFormat(gpuDevice, window);

    SDL_GPUGraphicsPipelineTargetInfo targetInfo{};
    targetInfo.num_color_targets = 1;
    targetInfo.color_target_descriptions = colorTargetDescriptions.data();
   

	SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.fragment_shader = fragmentShader;
	pipelineCreateInfo.vertex_shader = vertexShader;
	pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
    pipelineCreateInfo.target_info = targetInfo;

    SDL_GPUGraphicsPipeline* pipeline{ SDL_CreateGPUGraphicsPipeline(gpuDevice,&pipelineCreateInfo)};
	if (!pipeline) {
        throw dbg::SDL_Exception("Failed to create graphics pipeline");
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

            SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
            SDL_DrawGPUPrimitives(renderPass, 3u, 1u, 0u, 0u);

			SDL_EndGPURenderPass(renderPass);
        }

        if (!SDL_SubmitGPUCommandBuffer(commandBuffer))
        {
			throw dbg::SDL_Exception("Failed to submit GPU command buffer");
        }
	}

	SDL_ReleaseWindowFromGPUDevice(gpuDevice, window);
    SDL_DestroyWindow(window);

	SDL_ReleaseGPUShader(gpuDevice,vertexShader);
	SDL_ReleaseGPUShader(gpuDevice,fragmentShader);
    SDL_ReleaseGPUGraphicsPipeline(gpuDevice, pipeline);
    SDL_DestroyGPUDevice(gpuDevice);
	SDL_Quit();

    return 0;
}

SDL_GPUShader* LoadShader
(
    SDL_GPUDevice* device, std::string& shaderFileName, Uint32 sampleCount, Uint32 uniformBufferCount, 
    Uint32 storageBufferCount, Uint32 storageTextureCount)
{
    // Auto-detect the shader stage from the file name for convenience
    SDL_GPUShaderStage stage{};
    if(SDL_strstr(shaderFileName.c_str(), ".vert"))
    {
        stage = SDL_GPU_SHADERSTAGE_VERTEX;
    }
    else if(SDL_strstr(shaderFileName.c_str(), ".frag"))
    {
        stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    }
    else
    {
        throw dbg::SDL_Exception("Unsupported shader file extension");
	}

	char fullPath[256] = { 0 };
	SDL_GPUShaderFormat backendFormat = SDL_GetGPUShaderFormats(device);
    SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;

	const char* entrypoint = nullptr;

    if(backendFormat & SDL_GPU_SHADERFORMAT_SPIRV)
    {
		SDL_snprintf(fullPath, sizeof(fullPath), "%sContent/Shaders/Compiled/SPIRV/%s.spv", BasePath, shaderFileName.c_str());
        format = SDL_GPU_SHADERFORMAT_SPIRV;
        entrypoint = "main";
    }
    else if (backendFormat & SDL_GPU_SHADERFORMAT_DXIL)
    {
        SDL_snprintf(fullPath, sizeof(fullPath), "%sContent/Shaders/Compiled/DXIL/%s.dxil", BasePath, shaderFileName.c_str());
        format = SDL_GPU_SHADERFORMAT_DXIL;
        entrypoint = "main";
    }
    else if (backendFormat & SDL_GPU_SHADERFORMAT_MSL)
    {
        SDL_snprintf(fullPath, sizeof(fullPath), "%sContent/Shaders/Compiled/MSL/%s.msl", BasePath, shaderFileName.c_str());
        format = SDL_GPU_SHADERFORMAT_MSL;
        entrypoint = "main0";
    }
    else
    {
		SDL_Log("Unsupported shader format for device: %s", SDL_GetGPUDeviceDriver(device));
		return nullptr;
    }

    std::ifstream shaderFile{ fullPath,std::ios::binary | std::ios::ate };
    if (!shaderFile)
    {
		SDL_Log("Failed to open shader file: %s", fullPath);
        return nullptr;
    }

    std::streamsize codeSize = shaderFile.tellg();
    if (codeSize <= 0)
    {
        SDL_Log("Shader file is empty or could not determine size: %s", fullPath);
		return nullptr;
    }

	shaderFile.seekg(0, std::ios::beg);
	std::vector<char> code(codeSize);
    if (!shaderFile.read(code.data(), codeSize))
    {
		SDL_Log("Failed to read shader file: %s", fullPath);
		return nullptr;
    }

	SDL_GPUShaderCreateInfo shaderCreateInfo{};

    shaderCreateInfo.code = (Uint8*)code.data();
	shaderCreateInfo.code_size = codeSize;
    shaderCreateInfo.entrypoint = entrypoint;
	shaderCreateInfo.format = format;
	shaderCreateInfo.stage = stage;
    shaderCreateInfo.num_samplers = sampleCount;
	shaderCreateInfo.num_uniform_buffers = uniformBufferCount;
	shaderCreateInfo.num_storage_buffers = storageBufferCount;
	shaderCreateInfo.num_storage_textures = storageTextureCount;

	SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderCreateInfo);

    if (!shader)
    {
		SDL_Log("Failed to create shader from file: %s", fullPath);
		return nullptr;
    }

    return shader;
}

void InitializeAssetLoader()
{
	BasePath = SDL_GetBasePath();
}
