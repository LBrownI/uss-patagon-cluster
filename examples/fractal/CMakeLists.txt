cmake_minimum_required(VERSION 3.10)
project(FractalGenerator)

# Genera un compile_commands.json para que el LSP pueda detectar la libreria
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCES
    main.cpp
)
find_package(OpenCV REQUIRED)
if (NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found. Please set OpenCV_DIR.")
endif()

include_directories(${CMAKE_SOURCE_DIR}/dependencies
    ${OpenCV_INCLUDE_DIRS})

add_executable(fractal_generator ${SOURCES})
target_link_libraries(fractal_generator ${OpenCV_LIBS})
