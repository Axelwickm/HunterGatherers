cmake_minimum_required(VERSION 3.12)
project(HunterGatherers)

set(CMAKE_CXX_STANDARD 17)

set(SFML_DIR "C:/CPP_libs/SFML-2.5.1/lib/cmake/SFML")
find_package(SFML 2.5.1 COMPONENTS graphics audio REQUIRED)

find_package(OpenCL REQUIRED)

add_executable(HunterGatherers main.cpp Quadtree.cpp Quadtree.h WorldObject.cpp WorldObject.h Agent.cpp Agent.h BouncingBall.cpp BouncingBall.h utils.cpp Camera.cpp Camera.h Config.h Config.cpp World.cpp World.h OpenCL_Wrapper.cpp OpenCL_Wrapper.h Gene.cpp Gene.h)
target_link_libraries(HunterGatherers sfml-graphics sfml-audio)
target_link_libraries(HunterGatherers OpenCL::OpenCL)

#file(COPY "cl" DESTINATION "${CMAKE_BINARY_DIR}")
#add_custom_command(TARGET HunterGatherers PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/cl $<TARGET_FILE_DIR:HunterGatherers>/cl)


