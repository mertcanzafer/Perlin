#include "App.h"

App::App()
	:m_Width{800u},m_Height{600u},m_BasePath{SDL_GetBasePath()},m_WindowSize{1u},
	m_Windows(m_WindowSize,nullptr),m_Device{nullptr},m_VertexShader{nullptr},m_FragmentShader{nullptr},m_Pipeline{nullptr}
{

}

App::App(const Uint32 width, const Uint32 height, const Uint32 windowSize, std::string& path, 
	std::vector<SDL_Window*>& windows, 
	SDL_GPUDevice* device, SDL_GPUShader* vShader, 
	SDL_GPUShader* fShader, 
	SDL_GPUGraphicsPipeline* pipeline)
	:m_Width{width},m_Height{height},m_WindowSize{windowSize},m_BasePath{path},
	m_Windows{windows},m_Device{device},m_VertexShader{vShader},
	m_FragmentShader{fShader},m_Pipeline{pipeline}
{

}

void App::InitSDL()
{
	if (m_BasePath.empty())
		InitializeAssetLoader();

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		throw dbg::SDL_Exception("Failed to initialize SDL");
	}

	 m_Windows[0] = SDL_CreateWindow(
		"SDL Window", m_Width, m_Height, SDL_WINDOW_RESIZABLE);
	if (!m_Windows[0]) {
		throw dbg::SDL_Exception("Failed to create window");
	}

	m_Device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV |
		SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL
		, true, nullptr);

	if (!SDL_ClaimWindowForGPUDevice(m_Device, m_Windows[0]))
	{
		throw dbg::SDL_Exception("Failed to claim window for GPU device");
	}

	// Set the shaders
	m_VertexShader = LoadShader(m_Device, std::string("RawTriangle.vert"), 0, 0, 0, 0);

	if (!m_VertexShader) {
		throw dbg::SDL_Exception("Failed to load shader");
	}

	m_FragmentShader = LoadShader(m_Device, std::string("SolidColor.frag"), 0, 0, 0, 0);

	if (!m_FragmentShader) {
		throw dbg::SDL_Exception("Failed to load fragment shader");
	}

}

void App::Render()
{
	SDL_ShowWindow(m_Windows[0]);

	SDL_Event event;
	bool running = true;
	// !Game loop
	while (running) {
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
		AllocateBuffers();
	}
}

void App::OnCreate()
{
	std::vector<SDL_GPUColorTargetDescription> colorTargetDescriptions(1);

	colorTargetDescriptions[0].format = SDL_GetGPUSwapchainTextureFormat(m_Device, m_Windows[0]);

	SDL_GPUGraphicsPipelineTargetInfo targetInfo{};
	targetInfo.num_color_targets = 1;
	targetInfo.color_target_descriptions = colorTargetDescriptions.data();


	SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.fragment_shader = m_FragmentShader;
	pipelineCreateInfo.vertex_shader = m_VertexShader;
	pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
	pipelineCreateInfo.target_info = targetInfo;

	m_Pipeline  = SDL_CreateGPUGraphicsPipeline(m_Device,&pipelineCreateInfo);
	if (!m_Pipeline) {
		throw dbg::SDL_Exception("Failed to create graphics pipeline");
	}
}

void App::Clean()
{
	SDL_ReleaseWindowFromGPUDevice(m_Device, m_Windows[0]);
	SDL_DestroyWindow(m_Windows[0]);

	SDL_ReleaseGPUShader(m_Device, m_VertexShader);
	SDL_ReleaseGPUShader(m_Device, m_FragmentShader);
	SDL_ReleaseGPUGraphicsPipeline(m_Device, m_Pipeline);
	SDL_DestroyGPUDevice(m_Device);
	SDL_Quit();
}

SDL_GPUShader* App::LoadShader
(SDL_GPUDevice* device, 
	std::string& shaderFileName, 
	Uint32 sampleCount, 
	Uint32 uniformBufferCount, 
	Uint32 storageBufferCount, 
	Uint32 storageTextureCount)
{
	// Auto-detect the shader stage from the file name for convenience
	SDL_GPUShaderStage stage{};
	if (SDL_strstr(shaderFileName.c_str(), ".vert"))
	{
		stage = SDL_GPU_SHADERSTAGE_VERTEX;
	}
	else if (SDL_strstr(shaderFileName.c_str(), ".frag"))
	{
		stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
	}
	else
	{
		throw dbg::SDL_Exception("Unsupported shader file extension");
	}

	std::ostringstream fs;
	std::string fullPath;
	SDL_GPUShaderFormat backendFormat = SDL_GetGPUShaderFormats(device);
	SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;

	const char* entrypoint = nullptr;

	if (backendFormat & SDL_GPU_SHADERFORMAT_SPIRV)
	{
		//SDL_snprintf(fullPath, sizeof(fullPath), "%sContent/Shaders/Compiled/SPIRV/%s.spv", m_BasePath, shaderFileName.c_str());
		fs << m_BasePath << "Content/Shaders/Compiled/SPIRV/" << shaderFileName.c_str() << ".spv";
		format = SDL_GPU_SHADERFORMAT_SPIRV;
		entrypoint = "main";
	}
	else if (backendFormat & SDL_GPU_SHADERFORMAT_DXIL)
	{
		//SDL_snprintf(fullPath, sizeof(fullPath), "%sContent/Shaders/Compiled/DXIL/%s.dxil", m_BasePath, shaderFileName.c_str());
		fs << m_BasePath << "Content/Shaders/Compiled/DXIL/" << shaderFileName.c_str() << ".dxil";
		format = SDL_GPU_SHADERFORMAT_DXIL;
		entrypoint = "main";
	}
	else if (backendFormat & SDL_GPU_SHADERFORMAT_MSL)
	{
		//SDL_snprintf(fullPath, sizeof(fullPath), "%sContent/Shaders/Compiled/MSL/%s.msl", m_BasePath, shaderFileName.c_str());
		fs << m_BasePath << "Content/Shaders/Compiled/MSL/" << shaderFileName.c_str() << ".msl";
		format = SDL_GPU_SHADERFORMAT_MSL;
		entrypoint = "main0";
	}
	else
	{
		SDL_Log("Unsupported shader format for device: %s", SDL_GetGPUDeviceDriver(device));
		return nullptr;
	}

	fullPath = fs.str();

	std::ifstream shaderFile{ fullPath,std::ios::binary | std::ios::ate };
	if (!shaderFile)
	{
		throw dbg::SDL_Exception("Failed to open shader file: " + fullPath);
		return nullptr;
	}

	std::streamsize codeSize = shaderFile.tellg();
	if (codeSize <= 0)
	{
		throw dbg::SDL_Exception("Shader file is empty or could not determine size: " + fullPath);
		return nullptr;
	}

	shaderFile.seekg(0, std::ios::beg);
	std::vector<char> code(codeSize);
	if (!shaderFile.read(code.data(), codeSize))
	{
		throw dbg::SDL_Exception("Failed to read shader file: " + fullPath);
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
		throw dbg::SDL_Exception("Failed to create shader from file: " + fullPath);
		return nullptr;
	}

	return shader;
}

void App::InitializeAssetLoader()
{
	m_BasePath = SDL_GetBasePath();
}

void App::AllocateBuffers()
{
	// Set command buffer for GPU device
	SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(m_Device);
	if (!commandBuffer) {
		throw dbg::SDL_Exception("Failed to acquire GPU command buffer");
	}

	// Create a swapchain of textures
	SDL_GPUTexture* swapchain;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, m_Windows[0], &swapchain, nullptr, nullptr))
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

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, colorTargetInfos.data(),
			(Uint32)colorTargetInfos.size(), nullptr);

		SDL_BindGPUGraphicsPipeline(renderPass, m_Pipeline);
		SDL_DrawGPUPrimitives(renderPass, 3u, 1u, 0u, 0u);

		SDL_EndGPURenderPass(renderPass);
	}

	if (!SDL_SubmitGPUCommandBuffer(commandBuffer))
	{
		throw dbg::SDL_Exception("Failed to submit GPU command buffer");
	}
}

App::~App()
{
	Clean();
}