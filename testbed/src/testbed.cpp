// Testbed driver.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//

#include "Testbed.h"
#include "Buffer.h"
#include "Program.h"
#include "ScopedBinder.h"
#include "Shader.h"
#include "VertexArrayObject.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GL/glew.h>


#include <array>
#include <iostream>
#include <cmath>


using namespace tfgl;
using namespace tft;

namespace {

    const std::array<float,18> vertexData{
        0.0, 0.0, 0.0,
        1.0, 0.0, 0.0,
        1.0, 1.0, 0.0,

        0.0, 0.0, 0.0,
        1.0, 1.0, 0.0,
        0.0, 1.0, 0.0
    };

}


Testbed::~Testbed() {}


bool Testbed::InitImpl() {
    program_.reset(new Program);
    auto vs = Shader(GL_VERTEX_SHADER,      "gl.vert");
    auto fs = Shader(GL_FRAGMENT_SHADER,    "gl.frag");

    program_->Attach(vs);
    program_->Attach(fs);
    program_->Link();


    vao_.reset(new VertexArrayObject);

    // The VAO binds all the changes from here on. 
    ScopedBinder<VertexArrayObject> bindVao(*vao_);

    // The nature of VAO's is that the data is held by the VAO state.
    // The Buffer object can safely be unbound and deleted (because it
    // is reference counted on the OpenGL side).  As long as the VAO
    // has not been deleted, the associated buffer stays alive.
    buf_.reset(new Buffer(GL_ARRAY_BUFFER));
    ScopedBinder<Buffer> bindBuf(*buf_);

    buf_->SetStaticData(vertexData.data(), 
        vertexData.size() * sizeof(float));

    ::glEnableVertexAttribArray(0);
    THROW_ON_GL_ERROR();
    
    // Tell the VAO about the format of vertexData:
    ::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    THROW_ON_GL_ERROR();
    
    ::glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    THROW_ON_GL_ERROR();

    return true;
}


bool Testbed::DrawImpl() {
    glClear(GL_COLOR_BUFFER_BIT);
    THROW_ON_GL_ERROR();

    const auto matProj  = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
    const auto matView  = glm::mat4();
    const auto matModel = glm::mat4();

    ScopedBinder<Program> prog(*program_);
    program_->SetUniformMat4("matProj", glm::value_ptr(matProj));
    program_->SetUniformMat4("matView", glm::value_ptr(matView));
    program_->SetUniformMat4("matModel", glm::value_ptr(matModel));

    ScopedBinder<VertexArrayObject> vao(*vao_);

    ::glDrawArrays(GL_TRIANGLES, 0, 6);
    THROW_ON_GL_ERROR();

    return true;
}


int main(int argc, char** argv) {
    try {
        tft::Testbed app;
        app.Run(argc, argv);
    } catch(std::exception& e) {
        std::cerr << e.what() << "\n";
    }
    return 0;
}

