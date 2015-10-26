-- VS

in vec4 Position;

uniform mat4 ModelviewProjection;

void main()
{
   gl_Position = ModelviewProjection * Position;
}

--FS

out vec4 FragColor;

void main()
{
   FragColor.rgba = vec4(.2, .1, .1, 1.0);
}


--GS

