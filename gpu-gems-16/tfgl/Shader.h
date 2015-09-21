// Shader class, part of a minimal OpenGL library.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//

#pragma once


#include "Types.h"

#include <memory>
#include <string>


namespace tfgl {

    // Responsible for loading, parsing, hanging on to the OpenGL 
    // opaque handle to a shader.  Resource ownership of the shader
    // is performed by reference counting.
    class Shader {
    public:
        Shader(GLenum type, const std::string& filename);
        GLuint GetId() const;

    private:
        GLenum      type_;
        std::string filename_;

        // This owns the shader, when it is deleted, the shader is
        // deleted from the current GL context.
        struct ShaderOwner;
        std::shared_ptr<ShaderOwner> owner_;
    };

}
