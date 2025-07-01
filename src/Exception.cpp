#include "Exception.h"

namespace dbg {
	SDL_Exception::SDL_Exception(const std::string& message) : std::runtime_error(message + "\n" + SDL_GetError()){}
	
	void reportAssertionFailure(const char* expression, const char* file, int line)
	{
		std::cerr << "Assertion failed: " << expression
			<< " in file " << file <<
			" at line " << line << std::endl;
	}
}