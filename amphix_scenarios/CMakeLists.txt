cmake_minimum_required(VERSION 3.8)
project(amphix_scenarios)

find_package(ament_cmake REQUIRED)

install(
  DIRECTORY launch scenarios data
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
