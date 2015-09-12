#include "gl_helpers.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <iostream>
#include <cmath>


using namespace tfgl;


GLuint vertexBufferObject;
GLuint vertexArrayObject;


const float vertexData[] = {
    0.0, 0.0, 0.0,
    1.0, 0.0, 0.0,
    1.0, 1.0, 0.0,

    0.0, 0.0, 0.0,
    1.0, 1.0, 0.0,
    0.0, 1.0, 0.0
};

void InitTutorial() {
    glGenBuffers(1, &vertexBufferObject);
    THROW_ON_GL_ERROR()

    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    THROW_ON_GL_ERROR()
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
    THROW_ON_GL_ERROR()

    glGenVertexArrays(1, &vertexArrayObject);
    THROW_ON_GL_ERROR()

    // Needed for VertexAttribArray 
    glBindVertexArray(vertexArrayObject);
    THROW_ON_GL_ERROR()
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    THROW_ON_GL_ERROR()

    // glEnable(GL_CULL_FACE);
    // THROW_ON_GL_ERROR()

    // glCullFace(GL_BACK);
    // THROW_ON_GL_ERROR()

    // glFrontFace(GL_CW);    
    // THROW_ON_GL_ERROR()
}


void RunTutorial(int argc, char** argv) {
    ProcessArgs(argc, argv);
    auto window = InitGL();

    InitTutorial();
    auto program = InitShaders();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    THROW_ON_GL_ERROR()

    auto winWidth = 0, winHeight = 0;
    glfwGetWindowSize(window, &winWidth, &winHeight);

    const auto matProjObj   = glGetUniformLocation( program, "matProj" );
    THROW_ON_GL_ERROR()

    const auto matViewObj   = glGetUniformLocation( program, "matView" );
    THROW_ON_GL_ERROR()

    const auto matModelObj  = glGetUniformLocation( program, "matModel" );
    THROW_ON_GL_ERROR()

    const auto matProj      = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    for (auto j = 0u; j < 4; ++j)
        for (auto i = 0u; i < 4; ++i)
            std::cout << j << ":" << i << " " << matProj[j][i] << "\n";

    const auto matView      = glm::mat4();
    const auto matModel     = glm::mat4();

    auto drawFunc = [program, matProjObj, &matProj, matViewObj, &matView, matModelObj, &matModel]() -> void {
        glClear(GL_COLOR_BUFFER_BIT);
        THROW_ON_GL_ERROR()

        glUseProgram(program);
        THROW_ON_GL_ERROR()

        glUniformMatrix4fv(matProjObj, 1, GL_FALSE, glm::value_ptr(matProj));
        THROW_ON_GL_ERROR()

        glUniformMatrix4fv(matViewObj, 1, GL_FALSE, glm::value_ptr(matView));
        THROW_ON_GL_ERROR()

        glUniformMatrix4fv(matModelObj, 1, GL_FALSE, glm::value_ptr(matModel));
        THROW_ON_GL_ERROR()

        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
        THROW_ON_GL_ERROR()

        glEnableVertexAttribArray(0);
        THROW_ON_GL_ERROR()

        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
        THROW_ON_GL_ERROR()

        glDrawArrays(GL_TRIANGLES, 0, sizeof(vertexData) / sizeof(vertexData[0]));
        THROW_ON_GL_ERROR()

        glDisableVertexAttribArray(0);
        THROW_ON_GL_ERROR()

        glUseProgram(0);
        THROW_ON_GL_ERROR()
    };

    Loop(window, drawFunc);
}


int main(int argc, char** argv) {
    try {
        RunTutorial(argc, argv);
    } catch(std::exception& e) {
        std::cerr << e.what() << "\n";
    }
    return 0;
}

