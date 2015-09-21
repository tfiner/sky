// Buffer class, part of a minimal OpenGL library.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//

#include "VertexArrayObject.h"
#include "Buffer.h"
#include "Exception.h"
#include "ScopedBinder.h"


#include <GL/glew.h>


using namespace tfgl;


namespace { 

    GLuint GetNewId() {
        auto va = GLuint(0);
        ::glGenVertexArrays(1, &va);
        THROW_ON_GL_ERROR();
        return va;
    }

}


struct VertexArrayObject::Owner {
    Owner(GLuint id) : id_(id) {
        assert(id);
    }

    ~Owner() {
        if (id_)
            ::glDeleteVertexArrays(1, &id_);
    }

    GLuint id_;
};


VertexArrayObject::VertexArrayObject() : 
    owner_(std::make_shared<Owner>(GetNewId())) {}


void VertexArrayObject::Unbind() const {
    ::glBindVertexArray(0);
    THROW_ON_GL_ERROR();
}


void VertexArrayObject::Bind() const {
    assert(GetId());
    ::glBindVertexArray(GetId());
    THROW_ON_GL_ERROR();
}


GLuint VertexArrayObject::GetId() const {
    assert(owner_);
    if (owner_) {
        assert(owner_->id_);
        return owner_->id_;
    }
    return 0;
}

