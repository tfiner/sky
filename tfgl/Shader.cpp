#include "Shader.h"
#include "Exception.h"

#include <GL/glew.h>

#include <boost/filesystem.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>


using namespace tfgl;


namespace {


    // Takes an OpenGL enumeration for the shader type, and
    // the string contents of the shader.  Returns the OpenGL
    // identifier for the shader.  Throws on error.
    GLuint LoadShaderFile(  GLenum shaderType, 
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


    std::string LoadFile(const std::string& filename) {
        namespace fs = boost::filesystem;
        const auto path = fs::path(filename);

        std::cout << "Loading: " << filename << "\n";

        std::ifstream ifs(filename.c_str());
        std::ostringstream ss;

        auto lineNum = 1;
        auto line = std::string();
        while(std::getline(ifs, line)) {
            // std::cout << line << "\n";
            if (line.find("#pragma include") != std::string::npos) {
                std::cout << "Found pragma include on line " << lineNum << "...\n";

                std::stringstream incParms;
                incParms << line;

                auto ignore = std::string();
                auto incFilename = std::string();
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

                    ss  << "// included file begin: " << incFilename << "\n"
                        << included << "\n"
                        << "// included file end " << incFilename << "\n";

                } else {
                    std::cerr << "Failed to parse pragma include!\n";
                    throw GlException(filename, lineNum, 0);
                }

            } else {
                ss << line << "\n";
            }

            ++lineNum;
        }

        std::cout << "Loaded: " << filename << "\n";
        return ss.str();
    }


} // namespace

#if 0
    if (vm.count("vshader")) {
        auto filename = vm["vshader"].as<std::string>();
        std::cout << "Vertex shader: " << filename << ".\n";
        auto shaderVal = std::make_pair(filename, LoadFile(filename));

        std::cout << shaderVal.first << " begin\n";
        DumpStringLineByLine(shaderVal.second);
        std::cout << shaderVal.first << " end\n";

        shaders_.insert(std::make_pair(GL_VERTEX_SHADER, shaderVal));

    } else {
        std::cerr << "Vertex shader was not set.\n";
        exit(-1);
    }


    if (vm.count("fshader")) {
        auto filename = vm["fshader"].as<std::string>();        
        std::cout << "Fragment shader: " << filename << ".\n";
        auto shaderVal = std::make_pair(filename, LoadFile(filename));

        std::cout << shaderVal.first << " begin\n";
        DumpStringLineByLine(shaderVal.second);
        std::cout << shaderVal.first << " end\n";


        shaders_.insert(std::make_pair(GL_FRAGMENT_SHADER, shaderVal));

    } else {
        std::cerr << "Fragment shader was not set.\n";
        exit(-1);
    }    
}



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
