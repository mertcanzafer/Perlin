#pragma once
#include <iostream>
#include <SDL3\SDL.h>

namespace dbg {

    class SDL_Exception : public  std::runtime_error
    {
    public:
        explicit SDL_Exception(const std::string& message);
    };

}