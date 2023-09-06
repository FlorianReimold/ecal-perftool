cmake_minimum_required(VERSION 3.0)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

project(ecal_perftool)

find_package(eCAL REQUIRED)

set(source_files
  src/main.cpp
  src/publisher.cpp
  src/publisher.h
  src/publisher_statistics.h
  src/subscriber.cpp
  src/subscriber.h
  src/subscriber_statistics.h
)

add_executable(${PROJECT_NAME} ${source_files})

target_link_libraries(${PROJECT_NAME}
  eCAL::core
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}"
    FILES
        ${source_files}
)