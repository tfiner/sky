// Program class, part of a minimal OpenGL library.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//

#pragma once

#include "Types.h"

#include <memory>
#include <vector>

namespace tfgl {


    class Shader;


    // This class owns all OpenGL resources related to it: the Shaders 
    // given to it and the unique program loaded into an OpenGL context.  
    class Program {
    public:
        Program();
        ~Program();

        // Attaches a shader, throws on error.
        void Attach(const Shader& s);

        // After adding all the Shaders, Link, throws on error.
        void Link() const;

        // I know this is weird, but it is consistent vs. "use".
        // TODO: Better terminology?
        void Bind() const;
        void Unbind() const;


        GLuint GetId() const { return id_; }

        void SetUniform(const std::string& name, float f) const;
        void SetUniform(const std::string& name, float f1, float f2, float f3) const;

        void SetUniformMat4(const std::string& name, const float* f) const;

    private:
        GLuint id_;

        // Pimpl of shaders.
        std::vector<std::unique_ptr<Shader>> shaders_;


        GLuint GetUniform(const std::string& name) const;
    };


}

