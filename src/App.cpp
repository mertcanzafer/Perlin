#include "App.h"

// global variables
// !Not a very good design choice, but I'll arrange this later
static SDL_GPUBuffer* g_VertexBuffer = nullptr;
static SDL_GPUBuffer* g_IndexBuffer = nullptr;
static SDL_GPUTexture* g_Texture = nullptr;
static SDL_GPUSampler* g_Sampler = nullptr;

struct VertexData
{
	glm::vec3 position;
	struct Color {
		Uint8 r, g, b, a;
	}color;
	glm::vec2 uv; // texture coordinates, if needed

	explicit VertexData(glm::vec3 pos, Color color, glm::vec2 uvCoords)
		: position(pos), color(color), uv(uvCoords) {
	}
	VertexData() = default;
};

App::App()
	:m_Width{800u},m_Height{600u},m_BasePath{SDL_GetBasePath()},m_WindowSize{1u},
	m_Windows(m_WindowSize,nullptr),m_Device{nullptr},m_VertexShader{nullptr},m_FragmentShader{nullptr},m_Pipeline{nullptr}
	, m_Surface{nullptr}{}

App::App(const Uint32 width, const Uint32 height, const Uint32 windowSize, std::string& path, 
	std::vector<SDL_Window*>& windows, 
	SDL_GPUDevice* device, SDL_GPUShader* vShader, 
	SDL_GPUShader* fShader, 
	SDL_GPUGraphicsPipeline* pipeline, SDL_Surface* surface)
	:m_Width{width},m_Height{height},m_WindowSize{windowSize},m_BasePath{path},
	m_Windows{windows},m_Device{device},m_VertexShader{vShader},
	m_FragmentShader{fShader},m_Pipeline{pipeline}
	, m_Surface{ surface }{}

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

	std::string vShaderName = "TexturedQuad.vert";
	std::string fShaderName = "TexturedQuad.frag";

	// Set the shaders
	m_VertexShader = LoadShader(m_Device, vShaderName, 0, 0, 0, 0);

	if (!m_VertexShader) {
		throw dbg::SDL_Exception("Failed to vertex load shader");
	}

	m_FragmentShader = LoadShader(m_Device, fShaderName, 1, 0, 0, 0);

	if (!m_FragmentShader) {
		throw dbg::SDL_Exception("Failed to load fragment shader");
	}

	// Load texture data
	m_Surface = LoadImage("ravioli_inverted.bmp", 4);
	if(!m_Surface) {
		throw dbg::SDL_Exception("Failed to load image");
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
		SubmitRenderCommands();
	}
}

void App::OnCreate()
{
	std::vector<SDL_GPUColorTargetDescription> colorTargetDescriptions(1);

	colorTargetDescriptions[0].format = SDL_GetGPUSwapchainTextureFormat(m_Device, m_Windows[0]);

	SDL_GPUGraphicsPipelineTargetInfo targetInfo{};
	targetInfo.num_color_targets = 1;
	targetInfo.color_target_descriptions = colorTargetDescriptions.data();


	std::vector<SDL_GPUVertexAttribute> vertexAttributes
	{
		{0u,0u,SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,static_cast<Uint32>(offsetof(VertexData, position))}, // Position attribute
		{1u,0u,SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM,static_cast<Uint32>(offsetof(VertexData, color))}, // color attribute
		{2u,0u,SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,static_cast<Uint32>(offsetof(VertexData, uv))} // UV attribute, if needed
	};

	std::vector<SDL_GPUVertexBufferDescription> vertexBufferDescriptions
	{
		{0u,sizeof(VertexData), SDL_GPU_VERTEXINPUTRATE_VERTEX,0u}
	};

	SDL_GPUVertexInputState vertexInputState{};
	vertexInputState.vertex_attributes = vertexAttributes.data();
	vertexInputState.vertex_buffer_descriptions = vertexBufferDescriptions.data();
	vertexInputState.num_vertex_attributes = (Uint32)vertexAttributes.size();
	vertexInputState.num_vertex_buffers = (Uint32)vertexBufferDescriptions.size();


	SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.fragment_shader = m_FragmentShader;
	pipelineCreateInfo.vertex_shader = m_VertexShader;
	pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
	pipelineCreateInfo.target_info = targetInfo;
	pipelineCreateInfo.vertex_input_state = vertexInputState;

	m_Pipeline  = SDL_CreateGPUGraphicsPipeline(m_Device,&pipelineCreateInfo);
	if (!m_Pipeline) {
		throw dbg::SDL_Exception("Failed to create graphics pipeline");
	}

	// Create vertex buffer data
	// !Winding order is counter-clockwise
	std::array<VertexData, 4> vertices
	{
		// Define the vertices of a square
		VertexData{ glm::vec3(-0.5f, 0.5f, 0.0f),	VertexData::Color{ 0, 0, 255, 255},		glm::vec2(0,1)},
		VertexData{ glm::vec3(-0.5f, -0.5f, 0.0f),  VertexData::Color{ 255, 0, 0, 255},		glm::vec2(0,0)},
		VertexData{ glm::vec3(0.5f,  -0.5f, 0.0f),  VertexData::Color{ 0, 255, 0, 255},		glm::vec2(1,0)},
		VertexData{ glm::vec3(0.5f,  0.5f, 0.0f),	VertexData::Color{ 255,  255,255,255},  glm::vec2(1,1)}
	};

	std::array<Uint16, 6> indices
	{
		0u, 1u, 2u, // First triangle
		0u, 2u, 3u  // Second triangle
	};
	
	InitializeGPUResources(vertices, indices);

	// Create gpu sampler
	// Linear Clamping sampler
	SDL_GPUSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.min_filter = SDL_GPU_FILTER_LINEAR;
	samplerCreateInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
	samplerCreateInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
	samplerCreateInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	samplerCreateInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	samplerCreateInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

	SDL_GPUSampler* sampler = SDL_CreateGPUSampler(m_Device, &samplerCreateInfo);
	if(!sampler)
	{
		throw dbg::SDL_Exception("Failed to create GPU sampler");
	}
	g_Sampler = sampler;
}

void App::InitializeGPUResources(const std::array<struct VertexData, 4>& vertices,const std::array<Uint16, 6>& indices)
{
	/*
	* - We may replace bufferCreateInfos with SDL3 Properties when creating buffers.
	* - The application currently uses a single thread for rendering
	* - In the future, We may use multiple threads for different tasks like rendering, physics, etc.
	* - Therefore, we need to be careful about thread safe functions.
	* - SDL properties are reccommended for thread safety.
	*/
	
	// Create the vertex buffer
	SDL_GPUBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	bufferCreateInfo.size = sizeof(vertices);

	SDL_GPUBuffer* vertexBuffer = SDL_CreateGPUBuffer(m_Device, &bufferCreateInfo);
	if (!vertexBuffer)
		throw dbg::SDL_Exception("Couldn't create GPU vertex buffer.");
	g_VertexBuffer = vertexBuffer;

	// Set the vertex buffer name
	SDL_SetGPUBufferName(m_Device, vertexBuffer, "Perlin vertex buffer.");

	// Create the index buffer
	SDL_GPUBufferCreateInfo indexBufferCreateInfo{};
	indexBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
	indexBufferCreateInfo.size = sizeof(indices);

	SDL_GPUBuffer* indexBuffer = SDL_CreateGPUBuffer(m_Device, &indexBufferCreateInfo);
	if (!indexBuffer)
		throw dbg::SDL_Exception("Couldn't create GPU index buffer.");
	g_IndexBuffer = indexBuffer;

	// Set the Index buffer name
	SDL_SetGPUBufferName(m_Device, indexBuffer, "Perlin index buffer.");

	// Create a texture object
	SDL_GPUTextureCreateInfo textureCreateInfo{};
	textureCreateInfo.type = SDL_GPU_TEXTURETYPE_2D;
	textureCreateInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	textureCreateInfo.width = m_Surface->w;
	textureCreateInfo.height = m_Surface->h;
	textureCreateInfo.layer_count_or_depth = 1u;
	textureCreateInfo.num_levels = 1u;
	textureCreateInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

	SDL_GPUTexture* texture = SDL_CreateGPUTexture(m_Device, &textureCreateInfo);
	if (!texture)
		throw dbg::SDL_Exception("Failed to create GPU texture");
	g_Texture = texture;

	// Set the texture name
	SDL_SetGPUTextureName(m_Device, texture, "Ravioli texture");

	// To get data into the vertex and index buffer, we have to use a transfer buffer
	SDL_GPUTransferBufferCreateInfo transferBuffCreateInfo{};
	transferBuffCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	transferBuffCreateInfo.size = sizeof(vertices) + sizeof(indices);

	// Create the transfer buffer
	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(m_Device, &transferBuffCreateInfo);
	void* transferData = SDL_MapGPUTransferBuffer(m_Device, transferBuffer, false);

	if (!transferData)
		throw dbg::SDL_Exception("Transfering data went wrong!");

	// Copy the vertex and index buffer
	SDL_memcpy(transferData, vertices.data(), sizeof(vertices));

	void* IndicesOffset = static_cast<void*>(static_cast<std::byte*>(transferData) + sizeof(vertices));

	SDL_memcpy(IndicesOffset, indices.data(), sizeof(indices));

	SDL_UnmapGPUTransferBuffer(m_Device, transferBuffer);

	// Create another transfer buffer for the texture
	SDL_GPUTransferBufferCreateInfo textureTransferBuffCreateInfo{};
	textureTransferBuffCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	textureTransferBuffCreateInfo.size = (m_Surface->w * m_Surface->h) * 4; // Assuming 4 channels (RGBA)

	SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(m_Device, &textureTransferBuffCreateInfo);
	if (!textureTransferBuffer)
		throw dbg::SDL_Exception("Failed to create GPU transfer buffer for texture");

	void* textureTransferData = SDL_MapGPUTransferBuffer(m_Device, textureTransferBuffer, false);
	if (!textureTransferData)
		throw dbg::SDL_Exception("Failed to map GPU transfer buffer for texture");
	// Copy the texture data into the transfer buffer
	SDL_memcpy(textureTransferData, m_Surface->pixels, (static_cast<size_t>(m_Surface->w) * m_Surface->h) * 4);
	SDL_UnmapGPUTransferBuffer(m_Device, textureTransferBuffer);

	// Upload the transfer data to the vertex buffer
	SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(m_Device);
	if (!uploadCmdBuf)
		throw dbg::SDL_Exception("Failed to acquire GPU command buffer for upload");
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);
	if (!copyPass)
		throw dbg::SDL_Exception("Failed to begin GPU copy pass");

	SDL_GPUTransferBufferLocation vTransferLocation{};
	vTransferLocation.transfer_buffer = transferBuffer,
		vTransferLocation.offset = 0u;

	SDL_GPUTransferBufferLocation iTransferLocation{};
	iTransferLocation.transfer_buffer = transferBuffer,
		iTransferLocation.offset = sizeof(vertices);

	// Buffer region for vertex buffer
	SDL_GPUBufferRegion bufferRegion{};
	bufferRegion.buffer = vertexBuffer;
	bufferRegion.offset = 0u;
	bufferRegion.size = sizeof(vertices);

	SDL_GPUBufferRegion indexBufferRegion{};
	indexBufferRegion.buffer = indexBuffer;
	indexBufferRegion.offset = 0u;
	indexBufferRegion.size = sizeof(indices);

	SDL_UploadToGPUBuffer(copyPass, &vTransferLocation, &bufferRegion, false);
	SDL_UploadToGPUBuffer(copyPass, &iTransferLocation, &indexBufferRegion, false);

	// Now upload the texture data
	SDL_GPUTextureTransferInfo textureTransferInfo{};
	textureTransferInfo.offset = 0; /* Zeros out the rest */
	textureTransferInfo.transfer_buffer = textureTransferBuffer;

	SDL_GPUTextureRegion textureRegion{};
	textureRegion.texture = texture;
	textureRegion.w = m_Surface->w;
	textureRegion.h = m_Surface->h;
	textureRegion.d = 1u; // 2D texture, depth is 1

	SDL_UploadToGPUTexture(copyPass, &textureTransferInfo, &textureRegion, false);

	SDL_EndGPUCopyPass(copyPass);

	if (!SDL_SubmitGPUCommandBuffer(uploadCmdBuf))
		throw dbg::SDL_Exception("Failed to submit GPU command buffer for upload");

	SDL_DestroySurface(m_Surface);
	SDL_ReleaseGPUTransferBuffer(m_Device, transferBuffer);
	SDL_ReleaseGPUTransferBuffer(m_Device, textureTransferBuffer);
}

void App::SubmitRenderCommands()
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
		// BG color
		colorTargetInfos[0].clear_color.r = 0.3f;
		colorTargetInfos[0].clear_color.g = 0.4f;
		colorTargetInfos[0].clear_color.b = 0.5f;
		colorTargetInfos[0].clear_color.a = 1.0f;

		colorTargetInfos[0].load_op = SDL_GPU_LOADOP_CLEAR;
		colorTargetInfos[0].store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, colorTargetInfos.data(),
			(Uint32)colorTargetInfos.size(), nullptr);

		SDL_BindGPUGraphicsPipeline(renderPass, m_Pipeline);

		std::vector<SDL_GPUBufferBinding> bufferBindins
		{
			{g_VertexBuffer,0u},
		};

		SDL_GPUTextureSamplerBinding textureSamplerBinding
		{
			g_Texture,g_Sampler
		};

		SDL_GPUBufferBinding indexBufferBinding{ g_IndexBuffer, 0u };

		SDL_BindGPUVertexBuffers(renderPass, 0u, bufferBindins.data(), (Uint32)bufferBindins.size());
		SDL_BindGPUIndexBuffer(renderPass, &indexBufferBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

		SDL_BindGPUFragmentSamplers(renderPass, 0u, &textureSamplerBinding, 1u);
		SDL_DrawGPUIndexedPrimitives(renderPass, 6u, 1u, 0u, 0u, 0u);

		SDL_EndGPURenderPass(renderPass);
	}

	if (!SDL_SubmitGPUCommandBuffer(commandBuffer))
	{
		throw dbg::SDL_Exception("Failed to submit GPU command buffer");
	}
}

void App::Clean()
{
	if (m_VertexShader) {
		SDL_ReleaseGPUShader(m_Device, m_VertexShader);
		m_VertexShader = nullptr;
	}

	if (m_FragmentShader) {
		SDL_ReleaseGPUShader(m_Device, m_FragmentShader);
		m_FragmentShader = nullptr;
	}

	if (m_Pipeline) {
		SDL_ReleaseGPUGraphicsPipeline(m_Device, m_Pipeline);
		m_Pipeline = nullptr;
	}

	if (g_VertexBuffer) {
		SDL_ReleaseGPUBuffer(m_Device, g_VertexBuffer);
		g_VertexBuffer = nullptr;
	}

	if(g_IndexBuffer) {
		SDL_ReleaseGPUBuffer(m_Device, g_IndexBuffer);
		g_IndexBuffer = nullptr;
	}
	if (g_Texture) {
		SDL_ReleaseGPUTexture(m_Device, g_Texture);
		g_Texture = nullptr;
	}

	if(g_Sampler) {
		SDL_ReleaseGPUSampler(m_Device, g_Sampler);
		g_Sampler = nullptr;
	}

	if (m_Windows[0]) {
		SDL_ReleaseWindowFromGPUDevice(m_Device, m_Windows[0]);
		SDL_DestroyWindow(m_Windows[0]);
		m_Windows[0] = nullptr;
	}

	if (m_Device) {
		SDL_DestroyGPUDevice(m_Device);
		m_Device = nullptr;
	}
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


SDL_Surface* App::LoadImage(const std::string& fileName, int desirecChannels)
{
	std::string fullPath = std::format("{}Content/Images/{}", m_BasePath, fileName);

	SDL_Surface* result{ nullptr };
	SDL_PixelFormat pixelFormat{};

	std::string fileFormat = fullPath.substr(fullPath.find_last_of('.') + 1);
	std::clog << "Image format: " << fileFormat << std::endl;

	if(fileFormat == "bmp")
		result = SDL_LoadBMP(fullPath.c_str());
	/*else if(fileFormat == "png")
		result = SDL_LoadPNG(fullPath.c_str());
	else if(fileFormat == "jpg" || fileFormat == "jpeg")
		result = SDL_LoadJPG(fullPath.c_str());
	else if(fileFormat == "tga")
		result = SDL_LoadTGA(fullPath.c_str());
	else if(fileFormat == "webp")
		result = SDL_LoadWEBP(fullPath.c_str());*/
	else
	{
		throw dbg::SDL_Exception("Unsupported image format: " + fileFormat);
	}

	if(!result)
	{
		throw dbg::SDL_Exception("Failed to load image: " + fullPath);
	}

	if (desirecChannels == 4)
	{
		pixelFormat = SDL_PIXELFORMAT_ABGR8888;
	}
	else {
		SDL_assert(!"Unexpected desiredChannels");
		SDL_DestroySurface(result);
		return nullptr;
	}

	if(result->format != pixelFormat)
	{
		SDL_Surface* convertedSurface = SDL_ConvertSurface(result, pixelFormat);
		if (!convertedSurface)
		{
			SDL_DestroySurface(result);
			throw dbg::SDL_Exception("Failed to convert surface to desired pixel format");
		}
		SDL_DestroySurface(result);
		result = convertedSurface;
	}
	return result;
}

App::~App()
{
	Clean();
}