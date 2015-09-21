// Buffer class, part of a minimal OpenGL library.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//

#include "Buffer.h"
#include "Exception.h"

#include <GL/glew.h>


using namespace tfgl;


namespace { 

    GLuint GetNewBufferId() {
        auto bo = GLuint(0);
        ::glGenBuffers(1, &bo);
        THROW_ON_GL_ERROR();
        return bo;        
    }

}


struct Buffer::BufferOwner {
    BufferOwner(GLuint id) : id_(id) {
        assert(id);
    }

    ~BufferOwner() {
        if (id_)
            ::glDeleteBuffers(1, &id_);
    }

    GLuint id_;
};


Buffer::Buffer(GLenum type) : 
    type_(type),
    owner_(std::make_shared<BufferOwner>(GetNewBufferId())) {}


void Buffer::Bind() const {
    ::glBindBuffer(type_, GetId());
    THROW_ON_GL_ERROR();
}

void Buffer::Unbind() const {
    ::glBindBuffer(type_, 0);
}


GLuint Buffer::GetId() const {
    assert(owner_);
    if (owner_) {
        assert(owner_->id_);
        return owner_->id_;
    }
    return 0;
}

void Buffer::SetStaticData(const void* data, size_t sizeInBytes) const {
    ::glBufferData(type_, sizeInBytes, data, GL_STATIC_DRAW);
    THROW_ON_GL_ERROR();
}


std::unique_ptr<Buffer> tfgl::MakeVBO(const void* data, size_t sizeInBytes) {
    std::unique_ptr<Buffer> buf(new Buffer(GL_ARRAY_BUFFER));
    buf->Bind();
    buf->SetStaticData(data, sizeInBytes);
    return buf;
}
