cmake_minimum_required(VERSION 3.16)
project(CacheBlockingDemo VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform detection
if(WIN32)
    message(STATUS "Building for Windows")
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
    add_definitions(-DNOMINMAX)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
elseif(UNIX AND NOT APPLE)
    message(STATUS "Building for Linux")
    add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
endif()

# Vulkan headers (no linking required - we use runtime loading via volk)
find_path(VULKAN_HEADERS_DIR vulkan/vulkan.h
    HINTS
        $ENV{VULKAN_SDK}/Include
        /usr/include
        /usr/local/include
)
if(VULKAN_HEADERS_DIR)
    message(STATUS "Vulkan headers found: ${VULKAN_HEADERS_DIR}")
else()
    message(STATUS "Vulkan headers not found in standard locations, will use system default")
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(src/third_party/glfw)
add_subdirectory(src/third_party/glm)

# Volk (Vulkan meta-loader for runtime loading)
set(VOLK_SOURCES
    src/third_party/volk/volk.c
    src/third_party/volk/volk.h
)

# Core source files
set(CORE_SOURCES
    src/core/vulkan_context.cpp
    src/core/vulkan_context.h
    src/core/vulkan_utils.cpp
    src/core/vulkan_utils.h
)

# Example base
set(EXAMPLE_BASE_SOURCES
    src/examples/example_base.h
)

# Motion blur example sources
set(MOTION_BLUR_SOURCES
    src/examples/motion_blur/motion_blur_example.cpp
    src/examples/motion_blur/motion_blur_example.h
)

# All sources
set(SOURCES
    src/main.cpp
    ${VOLK_SOURCES}
    ${CORE_SOURCES}
    ${EXAMPLE_BASE_SOURCES}
    ${MOTION_BLUR_SOURCES}
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/third_party/volk
)
if(VULKAN_HEADERS_DIR)
    target_include_directories(${PROJECT_NAME} PRIVATE ${VULKAN_HEADERS_DIR})
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glm::glm
)

# Platform-specific libraries for dynamic loading
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE dl)
endif()

# Shader compilation
find_program(GLSLC glslc HINTS $ENV{VULKAN_SDK}/Bin $ENV{VULKAN_SDK}/bin /usr/bin)
if(NOT GLSLC)
    message(WARNING "glslc not found. Shaders must be compiled manually.")
else()
    message(STATUS "glslc found: ${GLSLC}")

    # Shader files
    set(SHADER_SOURCES
        shaders/gbuffer.vert
        shaders/gbuffer.frag
        shaders/motion_apply.vert
        shaders/motion_apply.frag
        shaders/blur_vertical.vert
        shaders/blur_vertical.frag
        shaders/blur_horizontal.vert
        shaders/blur_horizontal.frag
        shaders/final_apply.vert
        shaders/final_apply.frag
    )

    # Output directory for compiled shaders
    set(SHADER_OUTPUT_DIR ${CMAKE_BINARY_DIR}/shaders)
    file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

    # Compile each shader
    foreach(SHADER ${SHADER_SOURCES})
        get_filename_component(SHADER_NAME ${SHADER} NAME)
        set(SHADER_SPV ${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv)
        add_custom_command(
            OUTPUT ${SHADER_SPV}
            COMMAND ${GLSLC} ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER} -o ${SHADER_SPV}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}
            COMMENT "Compiling ${SHADER_NAME}"
        )
        list(APPEND SHADER_SPVS ${SHADER_SPV})
    endforeach()

    add_custom_target(shaders DEPENDS ${SHADER_SPVS})
    add_dependencies(${PROJECT_NAME} shaders)
endif()

# Copy shaders to build directory (for runtime)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SHADER_OUTPUT_DIR}
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
    COMMENT "Copying compiled shaders to output directory"
)

# Set output directories
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
