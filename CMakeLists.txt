cmake_minimum_required(VERSION 3.12)
project(HunterGatherers)

set(CMAKE_CXX_STANDARD 17)

add_definitions(-DCL_TARGET_OPENCL_VERSION=300)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")

find_package(SFML 2.5.1 COMPONENTS graphics audio REQUIRED)
find_package(OpenCL REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE PROJECT_SOURCES "src/*.cpp")
file(GLOB_RECURSE PROJECT_HEADERS "include/*.h")

list(FILTER PROJECT_SOURCES EXCLUDE REGEX ".*Test_PerlinNoise.cpp$")

add_executable(HunterGatherers ${PROJECT_SOURCES} ${PROJECT_HEADERS})
target_link_libraries(HunterGatherers sfml-graphics sfml-audio)
target_link_libraries(HunterGatherers OpenCL::OpenCL)

file(COPY "resources" DESTINATION "${CMAKE_BINARY_DIR}")
file(COPY Config.json DESTINATION "${CMAKE_BINARY_DIR}")

