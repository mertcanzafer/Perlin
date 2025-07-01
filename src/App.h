#include <iostream>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>

#include "Exception.h"
#include <array>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include <glm\vec3.hpp>

class App
{
public:
    App();
    App(
        const Uint32 width, const Uint32 height, const Uint32 windowSize,
        std::string& path, std::vector<SDL_Window*>& windows,
        SDL_GPUDevice* device, SDL_GPUShader* vShader, SDL_GPUShader* fShader, SDL_GPUGraphicsPipeline* pipeline
    );

    App(const App &) = delete;
    App &operator=(const App &) = delete;

    void InitSDL();
    void OnCreate(); // Will Create the pipline
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

    // Private Attributes
    void Clean();
    SDL_GPUShader* LoadShader(SDL_GPUDevice* device,
        std::string& shaderFileName,
        Uint32 sampleCount,
        Uint32 uniformBufferCount,
        Uint32 storageBufferCount,
        Uint32 storageTextureCount);
    
    void InitializeAssetLoader();
    void AllocateBuffers();
};