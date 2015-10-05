project(testbed)
cmake_minimum_required(VERSION 2.8.5)

message( STATUS "CMake detected OS '${CMAKE_SYSTEM_NAME}'" )
message( STATUS "Build type - ${CMAKE_BUILD_TYPE}")

# put our custom modules into cmake's search path
# set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

find_package(OpenGL)
find_package(GLEW)
find_package(GLFW)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../tfgl
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
)

set(TFGL_SRC 
    ${CMAKE_CURRENT_SOURCE_DIR}/../tfgl/Exception.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../tfgl/Shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../tfgl/Program.cpp
)

set(GPU_GEMS_SRC 
    ${CMAKE_CURRENT_SOURCE_DIR}/GLUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Master.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGLApp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PixelBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SkySimulation.cpp
)

add_executable(testbed ${TFGL_SRC} ${GPU_GEMS_SRC})
target_link_libraries(testbed 
    ${GLEW_LIBRARIES} 
    glfw3 
    ${GLFW_LIBRARIES} 
    ${OPENGL_LIBRARIES}
)

