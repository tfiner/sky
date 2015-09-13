#version 330
#pragma debug(on)

layout (location = 0) in vec4 position;

uniform mat4 matProj;
uniform mat4 matView;
uniform mat4 matModel;


void main() {
    gl_Position = matProj * matView * matModel * position;
}
