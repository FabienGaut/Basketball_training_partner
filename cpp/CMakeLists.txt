cmake_minimum_required(VERSION 3.10)
project(BasketBallDetection)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)

# ONNX Runtime
set(ONNXRUNTIME_ROOT "${CMAKE_SOURCE_DIR}/../deps/onnxruntime-linux-x64-1.17.0")
set(ONNXRUNTIME_INCLUDE_DIR "${ONNXRUNTIME_ROOT}/include")
set(ONNXRUNTIME_LIB_DIR "${ONNXRUNTIME_ROOT}/lib")

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${ONNXRUNTIME_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)

# Link directories
link_directories(${ONNXRUNTIME_LIB_DIR})

# Source files
set(SOURCES
    src/main.cpp
    src/Config.cpp
    src/Utils.cpp
    src/YOLODetector.cpp
    src/Capture.cpp
)

# Executable
add_executable(detectBasketBallPlayer ${SOURCES})

# Link libraries
target_link_libraries(detectBasketBallPlayer
    ${OpenCV_LIBS}
    onnxruntime
)

# Set RPATH for runtime
set_target_properties(detectBasketBallPlayer PROPERTIES
    BUILD_RPATH "${ONNXRUNTIME_LIB_DIR}"
    INSTALL_RPATH "${ONNXRUNTIME_LIB_DIR}"
)

# Enable optimizations for Release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(detectBasketBallPlayer PRIVATE -O3 -march=native)
endif()

# Print versions
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "ONNX Runtime: ${ONNXRUNTIME_ROOT}")
