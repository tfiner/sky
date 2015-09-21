// Shader class, part of a minimal OpenGL library.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//

#include "Shader.h"
#include "Exception.h"

#include <GL/glew.h>

#include <filesystem>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>


using namespace tfgl;
namespace fs = std::tr2::sys;


namespace {


    // Takes an OpenGL enumeration for the shader type, and
    // the string contents of the shader.  Returns the OpenGL
    // identifier for the shader.  Throws on error.
   tfgl::GLuint LoadShaderFile(  tfgl::GLenum shaderType,
                            const std::string& filename, 
                            const std::string& contents) {

        const auto shader = ::glCreateShader(shaderType);
        THROW_ON_GL_ERROR()

        const char* shaderPtr = contents.c_str();
        ::glShaderSource(shader, 1, &shaderPtr, nullptr);
        THROW_ON_GL_ERROR()

        ::glCompileShader(shader);

        auto status = GLint(GL_FALSE);
        ::glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE)
            throw GlShaderException(shaderType, shader, filename);

        return shader;
    }

    // PragmaInclude and LoadFile have a circular dependency:
    std::string LoadFile(const std::string& filename);


    // So that I can include shader files in one another!
    void PragmaInclude(
        std::ostream& os, 
        const fs::path& path, 
        const std::string& filename,
        const std::string& pragmaLine, 
        int lineNum) {

        std::cout << "Found pragma include on line " << lineNum << "...\n";

        std::stringstream incParms;
        incParms << pragmaLine;

        auto ignore         = std::string();
        auto incFilename    = std::string();
        if (incParms >> ignore >> ignore >> incFilename) {
            std::cout << "include file: " << incFilename << "\n";

            auto incPath = fs::path(incFilename);
            if (!fs::exists(incPath)) {
                auto oldPath = path;
                auto newPath = oldPath;
                newPath.remove_filename();
                newPath += "/";
                newPath += incPath;
                incFilename = newPath.string();
                std::cout << "trying file: " << incFilename << "\n";
            }

            auto included = LoadFile(incFilename);
            if (included.empty()) {
                std::cerr << "Empty include file " << incFilename << "!\n";
                throw GlException(filename, lineNum, 0);                        
            }

            // Included with a nice comment for OpengLG debuggers.
            os  << "// included file begin: " << incFilename << "\n"
                << included << "\n"
                << "// included file end " << incFilename << "\n";

        } else {
            std::cerr << "Failed to parse pragma include!\n";
            throw GlException(filename, lineNum, 0);
        }
    }


    std::string LoadFile(const std::string& filename) {
        const auto path = fs::path(filename);

        std::cout << "Loading: " << filename << "\n";

        std::ifstream ifs(filename.c_str());
        std::ostringstream ss;

        auto lineNum = 1;
        auto line = std::string();
        while(std::getline(ifs, line)) {
            // std::cout << line << "\n";
            if (line.find("#pragma include") != std::string::npos) {
                PragmaInclude(ss, path, filename, line, lineNum);
            } else {
                ss << line << "\n";
            }

            ++lineNum;
        }

        std::cout << "Loaded: " << filename << "\n";
        return ss.str();
    }


} // namespace


// Private implementation structure that is wrapped in a shared_ptr to 
// handle reference counting.
struct Shader::ShaderOwner {
    ShaderOwner(GLuint id) : id_(id) {}

    ~ShaderOwner() { 
        // Note: no throwing in destructors.
        if (id_)
            ::glDeleteShader(id_);
    }
    GLuint      id_;
};


Shader::Shader(tfgl::GLenum type, const std::string& filename) : 
    type_(type),
    filename_(filename){

    owner_ = std::make_shared<ShaderOwner>(LoadShaderFile( type_, filename, LoadFile(filename)));
}

tfgl::GLuint Shader::GetId() const {
    return owner_ ? owner_->id_ : 0; 
}


#if 0


GLFWwindow* tfgl::InitGL() {
    if (!glfwInit())
      throw std::runtime_error("Failed initialize GLFW.");

    glfwSetErrorCallback(OnError);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "OpenGL", NULL, NULL);
    if(!window)
      throw std::runtime_error("Failed to create GLFW window.");

    glfwMakeContextCurrent(window);

    printf("OpenGL version supported by this platform (%s): \n", glGetString(GL_VERSION));

    glewExperimental = GL_TRUE;
    auto glewStatus = glewInit();
    if (GLEW_OK != glewStatus) {
        std::ostringstream ss;
        ss << "Glew error: " << glewGetErrorString(glewStatus) << "\n";
        throw std::runtime_error( ss.str() );
    }

    // This has the side benefit of clearing all the preexisting errors, glew
    // http://www.opengl.org/wiki/OpenGL_Loading_Library
    glGetError();
    LOG_GL_ERRORS();

    return window;
}


GLuint tfgl::InitShaders() {
    auto program = glCreateProgram();
    THROW_ON_GL_ERROR()
    
    auto fs = LoadShaderFile(GL_FRAGMENT_SHADER);
    auto vs = LoadShaderFile(GL_VERTEX_SHADER);

    glAttachShader(program, vs);
    THROW_ON_GL_ERROR()

    glAttachShader(program, fs);
    THROW_ON_GL_ERROR()

    glLinkProgram(program);
    THROW_ON_GL_ERROR()

    auto status = GLint(0);
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
        throw GlProgramException(program);

    return program;
}

#endif
