-- VS

in vec4 Position;

// out vec4 vPosition;
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

const float fSamples = 2.0;


float scale(float fCos)
{
   float x = 1.0 - fCos;
   return fScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
}

void main()
{
   // Set the output for rendering the fragments.
   gl_Position = ModelviewProjection * Position;
   // vPosition = Position;

   // Get the ray from the camera to the vertex, and its length (which is the far point of the ray passing through the atmosphere)
   vec3 v3Pos = Position.xyz;
   vec3 v3Ray = v3Pos - v3CameraPos;
   float fFar = length(v3Ray);
   v3Ray /= fFar;

   // Calculate the ray's starting position, then calculate its scattering offset
   vec3 v3Start = v3CameraPos;
   float fHeight = length(v3Start);
   float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fCameraHeight));
   float fStartAngle = dot(v3Ray, v3Start) / fHeight;
   float fStartOffset = fDepth*scale(fStartAngle);

   // Initialize the scattering loop variables
   //gl_FrontColor = vec4(0.0, 0.0, 0.0, 0.0);
   float fSampleLength = fFar / fSamples;
   float fScaledLength = fSampleLength * fScale;
   vec3 v3SampleRay = v3Ray * fSampleLength;
   vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

   // Now loop through the sample rays
   vec3 v3FrontColor = vec3(0.0, 0.0, 0.0);
   for(int i = 0; i<NumSamples; i++)
   {
      float fHeight = length(v3SamplePoint);
      float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
      float fLightAngle = dot(v3LightDir, v3SamplePoint) / fHeight;
      float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
      float fScatter = (fStartOffset + fDepth*(scale(fLightAngle) - scale(fCameraAngle)));
      vec3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
      v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
      v3SamplePoint += v3SampleRay;
   }

   // Finally, scale the Mie and Rayleigh colors and set up the varying variables for the pixel shader
   v3Direction       = v3CameraPos - v3Pos;
   firstColor.rgb    = v3FrontColor * (v3InvWavelength * fKrESun);
   secondColor.rgb   = v3FrontColor * fKmESun;
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
   FragColor = color.rgbr;

   // FragColor = Red.rgbr;

   // FragColor = normalize(Red);
   FragColor.a = 1.0;

   // FragColor = vec4(1.0, 0.0, 0.0, 1.0);
   // FragColor.a = firstColor.b;


   //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
