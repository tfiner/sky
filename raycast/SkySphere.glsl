-- VS

in vec4 Position;
out vec4 vPosition;
uniform mat4 ModelviewProjection;

void main()
{
   gl_Position = ModelviewProjection * Position;
   vPosition = Position;
}

-- FS

out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
