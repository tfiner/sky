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

    // const std::array<float,18> vertexData{
    //     0.0, 0.0, 0.0,
    //     1.0, 0.0, 0.0,
    //     1.0, 1.0, 0.0,

    //     0.0, 0.0, 0.0,
    //     1.0, 1.0, 0.0,
    //     0.0, 1.0, 0.0
    // };

    inline void AddVertex(std::vector<glm::vec3>& v, float v0, float v1, float v2) {
        v.emplace_back(v0, v1, v2);
    }

    inline void AddFace(std::vector<float>& faces, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2 ) {
        faces.emplace_back(v0[0]);
        faces.emplace_back(v0[1]);
        faces.emplace_back(v0[2]);

        faces.emplace_back(v1[0]);
        faces.emplace_back(v1[1]);
        faces.emplace_back(v1[2]);

        faces.emplace_back(v2[0]);
        faces.emplace_back(v2[1]);
        faces.emplace_back(v2[2]);
    }

}


struct Testbed::Sphere {
    // Generates an Icosphere, see:
    // http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
    Sphere() {
        auto v = std::vector<glm::vec3>();
        v.reserve(12 * 3);

        const auto t = (1.0f + sqrt(5.0f)) / 2.0f;

        AddVertex(v, -1.0f,     t,  0.0f);
        AddVertex(v,  1.0f,     t,  0.0f);
        AddVertex(v, -1.0f,    -t,  0.0f);
        AddVertex(v,  1.0f,    -t,  0.0f);

        AddVertex(v,  0.0f, -1.0f,     t);
        AddVertex(v,  0.0f,  1.0f,     t);
        AddVertex(v,  0.0f, -1.0f,    -t);
        AddVertex(v,  0.0f,  1.0f,    -t);

        AddVertex(v,     t,  0.0f, -1.0f);
        AddVertex(v,     t,  0.0f,  1.0f);
        AddVertex(v,    -t,  0.0f, -1.0f);
        AddVertex(v,    -t,  0.0f,  1.0f);

        // Instead of using indexing, like the original 
        // author, I'm just going to make a new vector
        // of faces I'm going to fill.
        verts_.reserve(60);

        // 5 faces around point 0
        AddFace(verts_, v[ 0], v[11], v[ 5]);
        AddFace(verts_, v[ 0], v[ 5], v[ 1]);
        AddFace(verts_, v[ 0], v[ 1], v[ 7]);
        AddFace(verts_, v[ 0], v[ 7], v[10]);
        AddFace(verts_, v[ 0], v[10], v[11]);

        // 5 adjacent faces
        AddFace(verts_, v[ 1], v[ 5], v[ 9]);
        AddFace(verts_, v[ 5], v[11], v[ 4]);
        AddFace(verts_, v[11], v[10], v[ 2]);
        AddFace(verts_, v[10], v[ 7], v[ 6]);
        AddFace(verts_, v[ 7], v[ 1], v[ 8]);

        // 5 faces around point 3
        AddFace(verts_, v[ 3], v[ 9], v[ 4]);
        AddFace(verts_, v[ 3], v[ 4], v[ 2]);
        AddFace(verts_, v[ 3], v[ 2], v[ 6]);
        AddFace(verts_, v[ 3], v[ 6], v[ 8]);
        AddFace(verts_, v[ 3], v[ 8], v[ 9]);

        // 5 adjacent faces
        AddFace(verts_, v[ 4], v[ 9], v[ 5]);
        AddFace(verts_, v[ 2], v[ 4], v[11]);
        AddFace(verts_, v[ 6], v[ 2], v[10]);
        AddFace(verts_, v[ 8], v[ 6], v[ 7]);
        AddFace(verts_, v[ 9], v[ 8], v[ 1]);
    }

    std::vector<float> verts_;
};


Testbed::Testbed() : earth_(new Sphere) {}


Testbed::~Testbed() {}


void Testbed::InitVAO() {
    vao_.reset(new VertexArrayObject);

    // The VAO binds all the changes from here on. 
    ScopedBinder<VertexArrayObject> bindVao(*vao_);

    // The nature of VAO's is that the data is held by the VAO state.
    buf_.reset(new Buffer(GL_ARRAY_BUFFER));
    ScopedBinder<Buffer> bindBuf(*buf_);

    buf_->SetStaticData(earth_->verts_.data(), 
        earth_->verts_.size() * sizeof(float));

    ::glEnableVertexAttribArray(0);
    THROW_ON_GL_ERROR();
    
    // Tell the VAO about the format of vertexData:
    ::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    THROW_ON_GL_ERROR();
}


bool Testbed::InitImpl() {
    program_.reset(new Program);
    auto vs = Shader(GL_VERTEX_SHADER,      "gl.vert");
    auto fs = Shader(GL_FRAGMENT_SHADER,    "gl.frag");

    program_->Attach(vs);
    program_->Attach(fs);
    program_->Link();

    InitVAO();

    ::glClearColor(0.0f, 0.5f, 0.25f, 0.0f);
    THROW_ON_GL_ERROR();

    return true;
}


bool Testbed::DrawImpl() {
    glClear(GL_COLOR_BUFFER_BIT);
    THROW_ON_GL_ERROR();

    const auto matProj  = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, -5.0f, 5.0f);
    const auto matView  = glm::mat4();
    const auto matModel = glm::mat4();

    ScopedBinder<Program> prog(*program_);
    program_->SetUniformMat4("matProj", glm::value_ptr(matProj));
    program_->SetUniformMat4("matView", glm::value_ptr(matView));
    program_->SetUniformMat4("matModel", glm::value_ptr(matModel));

    ScopedBinder<VertexArrayObject> vao(*vao_);

    ::glDrawArrays(GL_TRIANGLES, 0, earth_->verts_.size()/3);

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

