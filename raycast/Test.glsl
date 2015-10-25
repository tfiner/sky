-- VS

in vec4 Position;

out vec3 firstColor;
out vec3 secondColor;

uniform mat4 ModelviewProjection;

void main()
{
   // Set the output for rendering the fragments.
   gl_Position = ModelviewProjection * Position;

   firstColor.rgb = gl_Position.rgb; //vec3(1.0, 0.0, 0.0);
   secondColor.rgb   = vec3(1.0, 1.0, 0.0);
}


--FS

in vec3 firstColor;
in vec3 secondColor;

out vec4 FragColor;

const vec3 Red = vec3(100.0, 0.0, 0.0);

void main()
{
   FragColor = firstColor.rgbr/* + secondColor.rgbr*/;
   FragColor.a = 1.0;
}
