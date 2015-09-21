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


    // This wraps all the OpenGL operations for a generic buffer.
    // It also owns the OpenGL buffer via reference counting.
    // It is safe and cheap to make copies of objects of this class.
    class Buffer {
    public:
        Buffer(GLenum type);

        GLuint GetId() const;

        void Bind() const;
        void Unbind() const;
        
        void SetStaticData(const void* data, size_t sizeInBytes) const;

    private:
        GLenum type_;

        // Holds the OpenGL handle to the buffer.
        // This is reference counted, and is released when the 
        // reference count goes to 0. 
        struct BufferOwner;
        std::shared_ptr<BufferOwner> owner_;
    };


    // Creates and returns a buffer of a an OpenGL type
    // (GL_ARRAY_BUFFER, etc.)
    std::unique_ptr<Buffer> MakeVBO(const void* data, size_t sizeInBytes);
}

