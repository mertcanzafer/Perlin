# set soruce files

set(SOURCES
    main.cpp
    Exception.cpp
)

set(HEADERS
    Exception.h
)

# Create your game executable target and put everything to executable
add_executable(PerlinSDL ${SOURCES} ${HEADERS})

# Link to the actual SDL3 library
target_link_libraries(PerlinSDL PRIVATE SDL3::SDL3)