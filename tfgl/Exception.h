// Exception classes, part of a minimal OpenGL library.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//

#pragma once

#include "Types.h"

#include <exception>
#include <string>


namespace tfgl {

    class GlShaderException : public std::exception {
    public:
        GlShaderException(GLenum type, GLuint shader, std::string filename);
        virtual const char* what() const noexcept override { return what_.c_str(); }
    private:
        std::string what_;
    };

    class GlProgramException : public std::exception {
    public:
        GlProgramException(GLuint program);
        virtual const char* what() const noexcept override { return what_.c_str(); }
    private:
        std::string what_;
    };

    class GlException : public std::exception {
    public:
        GlException(std::string file, int line, GLenum error);
        virtual const char* what() const noexcept override { return what_.c_str(); }
    private:        
        std::string file_;
        std::string what_;
        int line_;
    };    

    void ThrowOnGlError(std::string file, int line);
    void LogGlErrors(std::string file, int line);

}

#define THROW_ON_GL_ERROR() ThrowOnGlError(__FILE__, __LINE__);
#define LOG_GL_ERRORS() LogGlErrors(__FILE__, __LINE__);
