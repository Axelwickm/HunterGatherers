cmake_minimum_required(VERSION 3.12)
project(HunterGatherers)

set(CMAKE_CXX_STANDARD 17)

set(SFML_DIR "C:/CPP_libs/SFML-2.5.1/lib/cmake/SFML")
find_package(SFML 2.5.1 COMPONENTS graphics audio REQUIRED)

add_executable(HunterGatherers main.cpp Quadtree.cpp Quadtree.h WorldObject.cpp WorldObject.h Agent.cpp Agent.h BouncingBall.cpp BouncingBall.h utils.cpp Camera.cpp Camera.h Config.h Config.cpp World.cpp World.h)
target_link_libraries(HunterGatherers sfml-graphics sfml-audio)