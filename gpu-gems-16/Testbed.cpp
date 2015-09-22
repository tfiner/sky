// Testbed driver.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//

#include "Testbed.h"

#include "tfgl/Exception.h"
#include "tfgl/Program.h"
#include "tfgl/ScopedBinder.h"
#include "tfgl/Shader.h"


#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GL/glew.h>

#include "GameEngine.h"

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
    //::glClearColor(0.0f, 0.5f, 0.25f, 0.0f);
    //THROW_ON_GL_ERROR();
    engine_.reset(new CGameEngine);
    return true;
}


bool Testbed::DrawImpl() {
   ::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    //glClear(GL_COLOR_BUFFER_BIT);
    //THROW_ON_GL_ERROR();
   engine_->RenderFrame(1000);
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

