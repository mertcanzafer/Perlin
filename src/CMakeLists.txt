# set source files

set(SOURCES
    main.cpp
    Exception.cpp
    App.cpp
    ThreadPool.cpp
)

set(HEADERS
    Exception.h
    App.h
    ThreadPool.h
)

# Create your game executable target and put everything to executable
add_executable(PerlinSDL ${SOURCES} ${HEADERS})

# Link to the actual SDL3 library
target_link_libraries(PerlinSDL PRIVATE SDL3::SDL3)

# Link to glm headers
target_link_libraries(PerlinSDL PRIVATE glm::glm)

# Link to sdl_image
target_link_libraries(PerlinSDL PRIVATE SDL3_image::SDL3_image)

# Copy the content folder to output dir.
add_custom_command(TARGET PerlinSDL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/Content"
    "$<TARGET_FILE_DIR:PerlinSDL>/Content"
)