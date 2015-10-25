-- VS

in vec4 Position;

out vec4 vPosition;
out vec3 v3Direction;
out vec3 firstColor;
out vec3 secondColor;

uniform mat4 ModelviewProjection;

// From GPU Gems 2, Chapter 16
// Author Sean O'Neil
uniform vec3 v3CameraPos;		// The camera's current position
uniform float fCameraHeight;	// The camera's current height

uniform vec3 v3LightDir;		// The direction vector to the light source
uniform int NumSamples;

uniform float fKrESun;			// Kr * ESun
uniform float fKmESun;			// Km * ESun
uniform float fKr4PI;			// Kr * 4 * PI
uniform float fKm4PI;			// Km * 4 * PI

uniform vec3 v3InvWavelength;	// 1 / pow(wavelength, 4) for the red, green, and blue channels

uniform float fInnerRadius;		// The inner (planetary) radius


uniform float fScale;			      // 1 / (fOuterRadius - fInnerRadius)
uniform float fScaleDepth;		      // The scale depth (i.e. the altitude at which the atmosphere's average density is found)
uniform float fScaleOverScaleDepth;	// fScale / fScaleDepth

float scale(float fCos)
{
   float x = 1.0 - fCos;
   return fScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
}

void main()
{
   // Set the output for rendering the fragments.
   gl_Position = ModelviewProjection * Position;
   vPosition = Position;

   // Get the ray from the camera to the vertex, and its length (which is the far point of the ray passing through the atmosphere)
   vec3 v3Pos = Position.xyz;
   vec3 v3Ray = v3Pos - v3CameraPos;
   float fFar = length(v3Ray);
   v3Ray /= fFar;

   // Calculate the ray's starting position, then calculate its scattering offset
   vec3 v3Start = v3CameraPos;
   float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fCameraHeight));
   float fStartAngle = dot(v3Ray, v3Start) / fCameraHeight;
   float fStartOffset = fDepth*scale(fStartAngle);

   // Initialize the scattering loop variables
   float fSamples = NumSamples;
   float fSampleLength = fFar / fSamples;
   float fScaledLength = fSampleLength * fScale;
   vec3 v3SampleRay = v3Ray * fSampleLength;
   vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

   // Now loop through the sample rays
   vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);
   for(int i = 0; i<NumSamples; i++)
   {
      float fHeight  = length(v3SamplePoint);
      float fDepth   = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));

      float fLightAngle    = dot(v3LightDir, v3SamplePoint) / fHeight;
      float fCameraAngle   = dot(v3Ray, v3SamplePoint) / fHeight;

      float fScatter    = (fStartOffset + fDepth*(scale(fLightAngle) - scale(fCameraAngle)));
      vec3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
      // vec3 v3Attenuate = vec3(1);

      v3FrontColor   += v3Attenuate * (fDepth * fScaledLength);
      v3SamplePoint  += v3SampleRay;
   }

   // Finally, scale the Mie and Rayleigh colors and set up the varying variables for the pixel shader
   v3Direction       = v3CameraPos - v3Pos;

   firstColor.rgb = v3FrontColor * (v3InvWavelength * fKrESun); // vec3(0, 1, 0); // 
   secondColor.rgb = v3FrontColor * fKmESun;
}


--FS

in vec3 v3Direction;
in vec3 firstColor;
in vec3 secondColor;

out vec4 FragColor;

uniform vec3 v3LightDir;
uniform float g;
uniform float g2;

const vec3 Red = vec3(100.0, 0.0, 0.0);

void main()
{
   float fCos = dot(v3LightDir, v3Direction) / length(v3Direction);
   float fMiePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos*fCos) / pow(1.0 + g2 - 2.0*g*fCos, 1.5);
   vec3 color = firstColor + fMiePhase * secondColor;
   FragColor = color.rgbb;
   // FragColor = Red.rgbr; // color.rgbb;
   FragColor.a = 1.0;
}


--GS

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

in vec4 vPosition[1];

uniform mat4 ModelviewProjection;
uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 Modelview;

vec4 objCube[8]; // Object space coordinate of cube corner
vec4 ndcCube[8]; // Normalized device coordinate of cube corner
ivec4 faces[6];  // Vertex indices of the cube faces

void emit_vert(int vert)
{
   gl_Position = ndcCube[vert];
   EmitVertex();
}

void emit_face(int face)
{
   emit_vert(faces[face][1]); emit_vert(faces[face][0]);
   emit_vert(faces[face][3]); emit_vert(faces[face][2]);
   EndPrimitive();
}

void main()
{
   faces[0] = ivec4(0, 1, 3, 2); faces[1] = ivec4(5, 4, 6, 7);
   faces[2] = ivec4(4, 5, 0, 1); faces[3] = ivec4(3, 2, 7, 6);
   faces[4] = ivec4(0, 3, 4, 7); faces[5] = ivec4(2, 1, 6, 5);

   vec4 P = vPosition[0];
   vec4 I = vec4(1, 0, 0, 0);
   vec4 J = vec4(0, 1, 0, 0);
   vec4 K = vec4(0, 0, 1, 0);

   objCube[0] = P + K + I + J; objCube[1] = P + K + I - J;
   objCube[2] = P + K - I - J; objCube[3] = P + K - I + J;
   objCube[4] = P - K + I + J; objCube[5] = P - K + I - J;
   objCube[6] = P - K - I - J; objCube[7] = P - K - I + J;

   // Transform the corners of the box:
   for(int vert = 0; vert < 8; vert++)
      ndcCube[vert] = ModelviewProjection * objCube[vert];

   // Emit the six faces:
   for(int face = 0; face < 6; face++)
      emit_face(face);
}
