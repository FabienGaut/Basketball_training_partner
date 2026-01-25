cmake_minimum_required(VERSION 3.10)
project(BasketBallDetection)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

set(ONNXRUNTIME_ROOT "" CACHE PATH "Path to ONNX Runtime")

set(ONNXRUNTIME_INCLUDE_DIR ${ONNXRUNTIME_ROOT}/include)
set(ONNXRUNTIME_LIB_DIR ${ONNXRUNTIME_ROOT}/lib)

set(SOURCES
    src/main.cpp
    src/Config.cpp
    src/Utils.cpp
    src/YOLODetector.cpp
    src/Capture.cpp
)

add_executable(detectBasketBallPlayer ${SOURCES})

target_include_directories(detectBasketBallPlayer PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${ONNXRUNTIME_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_link_directories(detectBasketBallPlayer PRIVATE
    ${ONNXRUNTIME_LIB_DIR}
)

target_link_libraries(detectBasketBallPlayer PRIVATE
    ${OpenCV_LIBS}
    onnxruntime
)

set_target_properties(detectBasketBallPlayer PROPERTIES
    BUILD_RPATH ${ONNXRUNTIME_LIB_DIR}
    INSTALL_RPATH ${ONNXRUNTIME_LIB_DIR}
)

target_compile_options(detectBasketBallPlayer PRIVATE
    $<$<CONFIG:Release>:-O3 -march=native>
)

message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "ONNX Runtime root: ${ONNXRUNTIME_ROOT}")
