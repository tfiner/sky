// App class, part of a minimal OpenGL library.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//

#include "App.h"
#include "Exception.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <boost/program_options.hpp>

#include <iostream>


using namespace tfgl;
namespace bpo = boost::program_options;


namespace {

    void OnKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            ::glfwSetWindowShouldClose(window, GL_TRUE);
    }


    // TODO: maybe throw.
    void OnError(int error, const char* description) {
        std::cerr << error << ": " << description << "\n";
    }


}


App::App() : 
    screenWidth_(0), 
    screenHeight_(0),
    window_(nullptr),
    settings_(new bpo::variables_map) {}


void App::Run(int argc, char** argv) {
    if (!Init(argc, argv))
        return;

    while(!::glfwWindowShouldClose(window_)) {
        if (!Draw())
            break;

        ::glfwSwapBuffers(window_);
        ::glfwPollEvents();
    }        
}

bool App::Init(int argc, char** argv) {
    bpo::options_description desc("Options");

    desc.add_options()
        ("help,h",                                  "You are reading it now.")
        ("version,v",                               "Version of this.")
        ("vshader,V",   bpo::value<std::string>(),  "vertex shader")
        ("fshader,F",   bpo::value<std::string>(),  "fragment shader")
        ("width,W",     bpo::value<int>(),          "screen width")
        ("height,H",    bpo::value<int>(),          "screen height")
    ;

    bpo::store(bpo::parse_command_line(argc, argv, desc), *settings_);
    bpo::notify(*settings_);

    if (settings_->count("help")) {
        std::cout << desc << "\n";
        return false;
    }

    if (settings_->count("version")) {
        std::cout << GetVersion() << "\n";
        return false;
    }

    if (settings_->count("width")) {
        screenWidth_ = (*settings_)["width"].as<int>();
        std::cout << "Width: " << screenWidth_ << ".\n";
    }

    if (settings_->count("height")) {
        screenHeight_ = (*settings_)["height"].as<int>();
        std::cout << "Height: " << screenHeight_ << ".\n";
    }

    InitGL();
    return InitImpl();
}

void App::InitGL() {
    if (!::glfwInit())
      throw std::runtime_error("Failed initialize GLFW.");

    ::glfwSetErrorCallback(OnError);

    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    ::glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    ::glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    ::glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    ::glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    ::glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    window_ = ::glfwCreateWindow(screenWidth_, screenHeight_, "OpenGL", NULL, NULL);
    if(!window_)
      throw std::runtime_error("Failed to create GLFW window.");

    ::glfwMakeContextCurrent(window_);
    ::glfwSetKeyCallback(window_, OnKey);

    std::cout   << "OpenGL version supported by this platform "
                << "(" << ::glGetString(GL_VERSION) << "): \n";

    glewExperimental = GL_TRUE;
    auto glewStatus = ::glewInit();
    if (GLEW_OK != glewStatus) {
        std::ostringstream ss;
        ss << "Glew error: " << ::glewGetErrorString(glewStatus) << "\n";
        throw std::runtime_error( ss.str() );
    }

    // This has the side benefit of clearing all the preexisting errors, glew
    // http://www.opengl.org/wiki/OpenGL_Loading_Library
    ::glGetError();
    LOG_GL_ERRORS();
}
