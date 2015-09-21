// Program class, part of a minimal OpenGL library.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//

#include "Program.h"
#include "Shader.h"
#include "Exception.h"

#include <GL/glew.h>

#include <cassert>


using namespace tfgl;


Program::Program() : id_(0) {
    id_ = ::glCreateProgram();
    THROW_ON_GL_ERROR();
}


Program::~Program() {
    // Note: no throwing in destructors.

    for(const auto& s: shaders_) {
        if (auto sid = s->GetId()) {
            ::glDetachShader(id_, sid);
        }
    }

    assert(id_);
    ::glDeleteProgram(id_);
}


void Program::Attach(const Shader& s) {
    shaders_.push_back(std::unique_ptr<Shader>(new Shader(s)));

    ::glAttachShader(id_, s.GetId());
    THROW_ON_GL_ERROR();
}


void Program::Link() const {
    glLinkProgram(id_);
    THROW_ON_GL_ERROR();

    auto status = GLint(0);
    glGetProgramiv(id_, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
        throw GlProgramException(id_);
}


void Program::Bind() const {
    assert(id_);
    glUseProgram(id_);
    THROW_ON_GL_ERROR();
}


void Program::Unbind() const {
    glUseProgram(0);
    THROW_ON_GL_ERROR();
}


void Program::SetUniform(const std::string& name, float f) const {
    ::glUniform1f(GetUniform(name), f);
    THROW_ON_GL_ERROR();
}


void Program::SetUniform(const std::string& name, float f1, float f2, float f3) const {
    ::glUniform3f(GetUniform(name), f1, f2, f3);
    THROW_ON_GL_ERROR();
}


void Program::SetUniformMat4(const std::string& name, const float* f) const {
    ::glUniformMatrix4fv(GetUniform(name), 1, GL_FALSE, f);
    THROW_ON_GL_ERROR();
}


tfgl::GLuint Program::GetUniform(const std::string& name) const {
    if (auto loc = ::glGetUniformLocation(id_, name.c_str()))
        return loc;

    THROW_ON_GL_ERROR();
    return 0;
}
