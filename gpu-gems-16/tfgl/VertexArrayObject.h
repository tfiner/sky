// Buffer class, part of a minimal OpenGL library.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//

#pragma once

#include "Types.h"
#include "Exception.h"

#include <cassert>
#include <memory>


namespace tfgl {


    // This wraps all the OpenGL operations for a VAO.
    // It also owns the OpenGL buffer via reference counting.
    // It is safe and cheap to make copies of objects of this class.
    class VertexArrayObject {
    public:
        VertexArrayObject();

        GLuint GetId() const;

        void Bind() const;
        void Unbind() const;

    private:
        // Holds the OpenGL handle to the buffer.
        // This is reference counted, and is released when the 
        // reference count goes to 0. 
        struct Owner;
        std::shared_ptr<Owner> owner_;
    };


    // Usage:
    //      Generate VAO
    //      BindVAO
    //      Generate VBO's
    //      BindVBO's
    //      Specify vertex attributes
    //      Sets the data.
    //
    // Usage of the VAO is then inside the draw loop:
    //      BindVAO
    //      Draw
    //      UnbindVAO
    // References:
    // https://www.opengl.org/wiki/Vertex_Specification
    // https://www.opengl.org/wiki/Generic_Vertex_Attribute_-_examples
    // http://gamedev.stackexchange.com/a/99238

}

