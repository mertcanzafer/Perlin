# set soruce files

set(SOURCES
    main.cpp
    Exception.cpp
)

set(HEADERS
    Exception.h
)

# Create your game executable target and put everything to executable
add_executable(NoisyEffect ${SOURCES} ${HEADERS})

# Link to the actual SDL3 library
target_link_libraries(NoisyEffect PRIVATE SDL3::SDL3)