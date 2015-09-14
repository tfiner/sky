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

    struct Face {
        Face(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) : v0_(v0), v1_(v1), v2_(v2) {}

        glm::vec3 v0_;
        glm::vec3 v1_;
        glm::vec3 v2_;
    };

    static_assert(sizeof(Face) == sizeof(GLfloat) * 9, 
        "Platform doesn't support this directly.");

    inline void AddFace(std::vector<Face>& faces, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2 ) {
        faces.emplace_back(v0, v1, v2);
    }


    std::vector<Face> SubdivideSphere( const std::vector<Face>& src ) {
        auto subdiv = std::vector<Face>();
        subdiv.reserve(src.size() * 4);

        /*
                v2

              m2   m1
                
            v0  m0  v1

        */
        for(const auto & f : src) {
            const auto m0 = (f.v0_ + f.v1_) / 2.0f;
            const auto m1 = (f.v1_ + f.v2_) / 2.0f;
            const auto m2 = (f.v2_ + f.v0_) / 2.0f;

            subdiv.emplace_back(f.v0_,    m0,    m1);
            subdiv.emplace_back(   m0, f.v1_,    m1);
            subdiv.emplace_back(   m2,    m1, f.v2_);
            subdiv.emplace_back(   m0,    m1,    m2);
        }

        return subdiv;
    }


    // Generates an Icosphere, see:
    // http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
    std::vector<Face> GenIcoSphere() {
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

        // It would be more efficient to use indices from here
        // on, but I'm more interested in getting a sphere
        // rendered more than I am concerned with saving a few
        // K bytes.
        auto faces = std::vector<Face>();
        faces.reserve(20);

        // 5 faces around point 0
        AddFace(faces, v[ 0], v[11], v[ 5]);
        AddFace(faces, v[ 0], v[ 5], v[ 1]);
        AddFace(faces, v[ 0], v[ 1], v[ 7]);
        AddFace(faces, v[ 0], v[ 7], v[10]);
        AddFace(faces, v[ 0], v[10], v[11]);

        // 5 adjacent faces
        AddFace(faces, v[ 1], v[ 5], v[ 9]);
        AddFace(faces, v[ 5], v[11], v[ 4]);
        AddFace(faces, v[11], v[10], v[ 2]);
        AddFace(faces, v[10], v[ 7], v[ 6]);
        AddFace(faces, v[ 7], v[ 1], v[ 8]);

        // 5 faces around point 3
        AddFace(faces, v[ 3], v[ 9], v[ 4]);
        AddFace(faces, v[ 3], v[ 4], v[ 2]);
        AddFace(faces, v[ 3], v[ 2], v[ 6]);
        AddFace(faces, v[ 3], v[ 6], v[ 8]);
        AddFace(faces, v[ 3], v[ 8], v[ 9]);

        // 5 adjacent faces
        AddFace(faces, v[ 4], v[ 9], v[ 5]);
        AddFace(faces, v[ 2], v[ 4], v[11]);
        AddFace(faces, v[ 6], v[ 2], v[10]);
        AddFace(faces, v[ 8], v[ 6], v[ 7]);
        AddFace(faces, v[ 9], v[ 8], v[ 1]);  
        return faces;      
    }


} // namespace {


struct Testbed::Sphere {
    Sphere() : faces_(GenIcoSphere()) {
        faces_ = SubdivideSphere(faces_);
    }

    std::vector<Face> faces_;
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

    buf_->SetStaticData(earth_->faces_.data(), 
        earth_->faces_.size() * sizeof(Face));

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

    // ::glDrawArrays(GL_TRIANGLES, 0, earth_->faces_.size() * 3);
    ::glDrawArrays(GL_LINES, 0, earth_->faces_.size() * 3);

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

