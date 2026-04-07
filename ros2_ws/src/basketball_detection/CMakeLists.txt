cmake_minimum_required(VERSION 3.10)
project(basketball_detection)

# =====================
# C++ standard
# =====================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# =====================
# ROS 2
# =====================
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)

# =====================
# OpenCV
# =====================
find_package(OpenCV REQUIRED)

# =====================
# ONNX Runtime
# =====================
set(ONNXRUNTIME_ROOT "$ENV{ONNXRUNTIME_ROOT}")
set(ONNXRUNTIME_INCLUDE_DIR "${ONNXRUNTIME_ROOT}/include")
set(ONNXRUNTIME_LIB_DIR "${ONNXRUNTIME_ROOT}/lib")

find_library(ONNX_LIB onnxruntime
  PATHS ${ONNXRUNTIME_LIB_DIR}
  NO_DEFAULT_PATH
)

# =====================
# Core library (basketball_core)
# =====================
add_library(basketball_core
  src/Config.cpp
  src/Utils.cpp
  src/YOLODetector.cpp
  src/Capture.cpp  # inclure Capture.cpp pour ROS
)

target_include_directories(basketball_core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${OpenCV_INCLUDE_DIRS}
    ${ONNXRUNTIME_INCLUDE_DIR}
)

# Linker ce qui est nécessaire pour la librairie
target_link_libraries(basketball_core
  PUBLIC
    ${OpenCV_LIBS}
    ${ONNX_LIB}
  PRIVATE
    rclcpp::rclcpp
)

ament_export_include_directories(include)

# =====================
# Publisher node
# =====================
add_executable(publisher_node src/publisher_node.cpp)

# Dépendances ROS pour le node
ament_target_dependencies(publisher_node
  rclcpp
  std_msgs
  geometry_msgs
)

# Le node utilise la librairie core
target_link_libraries(publisher_node basketball_core)

install(TARGETS
  publisher_node
  DESTINATION lib/${PROJECT_NAME}
)

# =====================
# Tests (optionnel)
# =====================
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
