project(testbed)
cmake_minimum_required(VERSION 2.8.5)

message( STATUS "CMake detected OS '${CMAKE_SYSTEM_NAME}'" )
message( STATUS "Build type - ${CMAKE_BUILD_TYPE}")

# put our custom modules into cmake's search path
# set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

find_package(OpenGL)
find_package(GLEW)
find_package(GLFW)
FIND_PACKAGE(Boost 1.55 COMPONENTS program_options filesystem system REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../tfgl
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
)

set(SRCS 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Testbed.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/../tfgl/App.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../tfgl/Buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../tfgl/Exception.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../tfgl/Shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../tfgl/Program.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../tfgl/VertexArrayObject.cpp
)

add_executable(testbed ${SRCS} )
target_link_libraries(testbed 
    ${GLEW_LIBRARIES} 
    glfw3 
    ${GLFW_LIBRARIES} 
    ${OPENGL_LIBRARIES}
    ${Boost_LIBRARIES}
)

