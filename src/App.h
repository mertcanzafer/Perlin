#include <iostream>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

#include <SDL3_image\sdl_image.h>

#include "Exception.h"
#include <array>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include <glm\vec3.hpp>
#include <glm\vec2.hpp>

class App
{
public:
    App();
    App(
        const Uint32 width, const Uint32 height, const Uint32 windowSize,
        std::string& path, std::vector<SDL_Window*>& windows,
        SDL_GPUDevice* device, SDL_GPUShader* vShader, SDL_GPUShader* fShader, SDL_GPUGraphicsPipeline* pipeline,
		SDL_Surface* surface
    );

    App(const App &) = delete;
    App &operator=(const App &) = delete;

    void InitSDL();
    void OnCreate();
    void Render();

    ~App();

private:
    // SDL init attributes
    Uint32 m_Width, m_Height, m_WindowSize;
    std::string m_BasePath;

    // We are going to use only 1 window for now.
    std::vector<SDL_Window *> m_Windows;
    SDL_GPUDevice* m_Device;
    SDL_GPUShader* m_VertexShader;
    SDL_GPUShader* m_FragmentShader;
    SDL_GPUGraphicsPipeline* m_Pipeline;

    SDL_Surface* m_Surface;

private:
    // Private Methods
    void Clean();
    SDL_GPUShader* LoadShader(SDL_GPUDevice* device,
        std::string& shaderFileName,
        Uint32 sampleCount,
        Uint32 uniformBufferCount,
        Uint32 storageBufferCount,
        Uint32 storageTextureCount);
    
    void InitializeAssetLoader();
    void SubmitRenderCommands();
    void InitializeGPUResources(const std::array<struct VertexData,4>& vertices, const std::array<Uint16, 6>& indices);

    SDL_Surface* LoadImage(const std::string& fileName, int desirecChannels);
};