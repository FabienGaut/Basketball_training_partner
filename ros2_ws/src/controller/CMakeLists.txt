cmake_minimum_required(VERSION 3.10)
project(controller)

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
find_package(geometry_msgs REQUIRED)

# =====================
# Subscriber node
# =====================
add_executable(subscriber_node src/subscriber_node.cpp)

ament_target_dependencies(subscriber_node
  rclcpp
  geometry_msgs
)

install(TARGETS
  subscriber_node
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
