/**
* Example Vertex Shader
* Sets the position of the vertex by setting gl_Position
*/


void main() {

    // This sets the position of the vertex in 3d space. The correct math is
    // provided below to take into account camera and object data.
    gl_Position = vec4( 0.0, 0.0, 0.0, 1.0 );

}