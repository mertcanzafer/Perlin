#pragma once
#include <iostream>
#include <SDL3\SDL.h>

namespace PERLIN_UTIL
{
    namespace dbg {

        class SDL_Exception : public  std::runtime_error
        {
        public:
            explicit SDL_Exception(const std::string& message);
        };

        void reportAssertionFailure(const char* expression, const char* file, int line);

#if defined(_MSC_VER)
#define dbg_debugBreak() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#define dbg_debugBreak() __builtin_trap()
#else
#define dbg_debugBreak() ((void)0)
#endif
    }
}


#ifdef ASSERTIONS_ENABLED

#define P_ASSERT(expr)                                            \
        do {                                                          \
            if (!(expr)) {                                            \
                dbg::reportAssertionFailure(#expr, __FILE__, __LINE__); \
                dbg_debugBreak();                                     \
            }                                                         \
        } while (0)

#else
#define P_ASSERT(expr) ((void)0)
#endif
