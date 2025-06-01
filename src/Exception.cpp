#include "Exception.h"

namespace dbg {
	SDL_Exception::SDL_Exception(const std::string& message) : std::runtime_error(message + "\n" + SDL_GetError()){}
}