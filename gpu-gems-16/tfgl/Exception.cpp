// Exception classes, part of a minimal OpenGL library.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//

#include "Exception.h"

#include <GL/glew.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


using namespace tfgl;


namespace {


    std::string GlErrorToString(tfgl::GLenum errNum) {
        std::string msg;

        switch(errNum) {
        case GL_NO_ERROR:
            msg = "No error has been recorded. The value of this symbolic constant is guaranteed to be 0.";
        break;

        case GL_INVALID_ENUM:
            msg = "An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.";
        break;

        case GL_INVALID_VALUE:
            msg = "A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.";
        break;

        case GL_INVALID_OPERATION:
            msg = "The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.";
        break;

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            msg = "The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag.";
        break;

        case GL_OUT_OF_MEMORY:
            msg = "There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
        break;

        case GL_STACK_UNDERFLOW:
            msg = "An attempt has been made to perform an operation that would cause an internal stack to underflow.";
        break;

        case GL_STACK_OVERFLOW:
            msg = "An attempt has been made to perform an operation that would cause an internal stack to overflow.";
        break;

        default:
            assert(!"Unknown enum.");
        }

        return msg;
    }


    std::string GetGlErrors(std::string file, int line, tfgl::GLenum error) {
        std::ostringstream os;

        os  << file.c_str() << "(" << line << "):" << " OpenGL error: ";
        auto const oldError = error;
        while(error != GL_NO_ERROR) {
            os << "(" << error << ") " << GlErrorToString(error) << "\n";
            error = glGetError();
            if(error == oldError)
               break;
        }
        
        return os.str();
    }


    // Returns a line from a source file, along with a number of surrounding lines.
    std::string GetErrorLineText(std::string filename, int lineNum) {
        std::cerr << "filename: " << filename.c_str() << " " << lineNum << "\n"; 
        std::ifstream ifs(filename.c_str());
        std::ostringstream os;
        int line = 1;
        std::string lineText;
        while( std::getline(ifs, lineText) ) {
            if (lineNum == line) {
                os << lineText;
                break;
            }
            line++;
        }
        return os.str();
    } 


    // Parses GL error text like 
    // ERROR: 0:5: '' :  extension 'fooBar' is not supported
    // into the line and column number of the error.
    std::pair<int,int> ParseError(const std::string& errStr) {
        std::stringstream ss;
        ss << errStr.c_str();

        std::string junk;
        ss >> junk;
        if (junk != "ERROR:")
            return std::make_pair(-1, -1);

        int col(-1);
        ss >> col;
        if (col == -1)
            return std::make_pair(-1, -1);

        char c = '\0';
        ss >> c;
        if (c != ':')
            return std::make_pair(-1, -1);

        int line(-1);
        ss >> line;
        if (line == -1)
            return std::make_pair(-1, -1);

        std::cerr << "error: " << junk << " col: " << col << " line: " << line << "\n";

        return std::make_pair(line, col);
    }


} // namespace


GlShaderException::GlShaderException(tfgl::GLenum type, tfgl::GLuint shader, std::string filename) {
    GLint infoLogLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

    std::vector<GLchar> buf(infoLogLength+1, '\0');
    glGetShaderInfoLog(shader, buf.size(), &infoLogLength, buf.data());

    auto glError = std::string(static_cast<const char*>(buf.data()));
    auto lineCol = ParseError(glError);

    std::ostringstream os;
    os  << "Shader error in " 
        << filename << " type: " << type << " number: " << shader << "\n";
    if (infoLogLength)
        os << glError.c_str() << "\n";
    else
        os << "None.\n";        

    if(lineCol.first >= 0) {
        auto colPtr = std::string(lineCol.second, ' ');
        colPtr += '^';

        auto errLine = GetErrorLineText(filename, lineCol.first);
        os  << errLine << "\n"
            << colPtr << "\n";
    }

    what_ = os.str();
}


GlProgramException::GlProgramException(tfgl::GLuint program) {
    GLint infoLogLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

    std::vector<GLchar> buf(infoLogLength+1, '\0');
    glGetProgramInfoLog(program, buf.size(), &infoLogLength, buf.data());

    std::ostringstream os;
    os << "Program error:\n";
    if (infoLogLength)
        os << static_cast<const char*>(buf.data()) << "\n";
    else
        os << "None.\n";        
    what_ = os.str();
}


GlException::GlException(std::string file, int line, tfgl::GLenum error) : file_(file), line_(line) {
    what_ = GetGlErrors(file, line, error);
}


void tfgl::ThrowOnGlError(std::string file, int line) {
    auto status = glGetError();
    if (GL_NO_ERROR != status) 
        throw GlException(file, line, status);
}


void tfgl::LogGlErrors(std::string file, int line) {
    auto status = glGetError();
    if (GL_NO_ERROR != status) 
        std::cerr << GetGlErrors(file, line, status);    
}

