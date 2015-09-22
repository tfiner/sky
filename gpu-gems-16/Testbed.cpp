// Testbed driver.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//

#include "Testbed.h"
#include "GameEngine.h"

#include "tfgl/Exception.h"
#include "tfgl/Program.h"
#include "tfgl/ScopedBinder.h"
#include "tfgl/Shader.h"


#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <array>
#include <iostream>
#include <cmath>


using namespace tfgl;
using namespace tft;

namespace {


} // namespace {



Testbed::Testbed() {}
Testbed::~Testbed() {}


bool Testbed::InitImpl() {
   gameEngine_.reset(new CGameEngine);

   ::glfwMakeContextCurrent(window_);

    //program_.reset(new Program);
    //auto vs = Shader(GL_VERTEX_SHADER,      "gl.vert");
    //auto fs = Shader(GL_FRAGMENT_SHADER,    "gl.frag");

    //program_->Attach(vs);
    //program_->Attach(fs);
    //program_->Link();

    ::glClearColor(0.0f, 0.5f, 0.25f, 1.0f);
    THROW_ON_GL_ERROR();
   

    return true;
}


bool Testbed::DrawImpl() {
   ::glfwMakeContextCurrent(window_);

   glClear(GL_COLOR_BUFFER_BIT);
   THROW_ON_GL_ERROR();

   //if (gameEngine_)
   //   gameEngine_->RenderFrame(100);

    //const auto matProj  = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, -5.0f, 5.0f);
    //const auto matView  = glm::mat4();
    //const auto matModel = glm::mat4();

    //ScopedBinder<Program> prog(*program_);
    //program_->SetUniformMat4("matProj", glm::value_ptr(matProj));
    //program_->SetUniformMat4("matView", glm::value_ptr(matView));
    //program_->SetUniformMat4("matModel", glm::value_ptr(matModel));


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

