// Shader class, part of a minimal OpenGL library.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//

#pragma once


#include "Types.h"


#include <string>

namespace tfgl {
    // TODO: If these end up being used everywhere, move these into a common header.
    using GLuint = unsigned int;

    // Responsible for loading, parsing, hanging on to the OpenGL 
    // opaque handle to a shader.
    class Shader {
    public:
    private:
        // The source file name.
        std::string filename_;
        GLuint      id_;
    };

}
