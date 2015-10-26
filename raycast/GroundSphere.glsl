-- VS

in vec4 Position;

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

   // Get the ray from the camera to the vertex, and its length (which is the far point of the ray passing through the atmosphere)
   vec3 v3Pos = Position.xyz;
   vec3 v3Ray = v3Pos - v3CameraPos;
   float fFar = length(v3Ray);
   v3Ray /= fFar;

   // Calculate the ray's starting position, then calculate its scattering offset
   vec3 v3Start = v3CameraPos;
   float fDepth = exp((fInnerRadius - fCameraHeight) / fScaleDepth);

   float fCameraAngle = dot(-v3Ray, v3Pos) / length(v3Pos);
   float fCameraScale = scale(fCameraAngle);
   float fCameraOffset = fDepth*fCameraScale;

   float fLightAngle = dot(v3LightDir, v3Pos) / length(v3Pos);
   float fLightScale = scale(fLightAngle);

   float fTemp = (fLightScale + fCameraScale);

   // Initialize the scattering loop variables
   float fSamples = NumSamples;
   float fSampleLength = fFar / fSamples;
   float fScaledLength = fSampleLength * fScale;
   vec3 v3SampleRay = v3Ray * fSampleLength;
   vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;

   // Now loop through the sample rays
   vec3 v3FrontColor = vec3(0.0);
   vec3 v3Attenuate;
   for(int i = 0; i<NumSamples; i++)
   {
      float fHeight = length(v3SamplePoint);
      float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
      float fScatter = fDepth*fTemp - fCameraOffset;
      v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
//      vec3 v3Attenuate = vec3(1);
      v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
      v3SamplePoint += v3SampleRay;
   }

   firstColor.rgb = v3FrontColor * (v3InvWavelength * fKrESun + fKmESun);
   secondColor.rgb = v3Attenuate;
}


--FS

in vec3 firstColor;
in vec3 secondColor;

out vec4 FragColor;

const vec3 Red = vec3(100.0, 0.0, 0.0);

void main()
{
   FragColor.rgb = firstColor; // +0.25 * secondColor;
   // FragColor = Red.rgbr;
   FragColor.a = 1.0;
}


--GS

