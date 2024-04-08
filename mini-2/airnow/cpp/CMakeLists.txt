# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.27.6)

# Project's name
project(AirNow)

# Specify the directory for the build files
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/cmake-build)

# Automatically find all .cpp files in the current directory to build
file(GLOB SOURCES "src/*.cpp" "src/*.h")

# Define the executable with the project name and include all source files
add_executable(${PROJECT_NAME} ${SOURCES})

# Specify C++ standard, for example, C++15
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Specify the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
