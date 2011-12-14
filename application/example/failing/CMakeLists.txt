cmake_minimum_required(VERSION 2.6.2)
project(app_example_failing)

set(CMAKE_MODULE_PATH
  "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}"
  )

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${${PROJECT_NAME}_SOURCE_DIR})
include_directories(${CMAKE_INCLUDE_PATH})

# generate the xml document

# generate the c files

# compile all c files
