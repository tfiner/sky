-- VS

in vec4 Position;
out vec4 vPosition;
uniform mat4 ModelviewProjection;

void main()
{
    gl_Position = ModelviewProjection * Position;
    vPosition = Position;
}

-- GS

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
    faces[0] = ivec4(0,1,3,2); faces[1] = ivec4(5,4,6,7);
    faces[2] = ivec4(4,5,0,1); faces[3] = ivec4(3,2,7,6);
    faces[4] = ivec4(0,3,4,7); faces[5] = ivec4(2,1,6,5);

    vec4 P = vPosition[0];
    vec4 I = vec4(1,0,0,0);
    vec4 J = vec4(0,1,0,0);
    vec4 K = vec4(0,0,1,0);

    objCube[0] = P+K+I+J; objCube[1] = P+K+I-J;
    objCube[2] = P+K-I-J; objCube[3] = P+K-I+J;
    objCube[4] = P-K+I+J; objCube[5] = P-K+I-J;
    objCube[6] = P-K-I-J; objCube[7] = P-K-I+J;

    // Transform the corners of the box:
    for (int vert = 0; vert < 8; vert++)
        ndcCube[vert] = ModelviewProjection * objCube[vert];

    // Emit the six faces:
    for (int face = 0; face < 6; face++)
        emit_face(face);
}

-- FS

out vec4 FragColor;

uniform sampler3D Density;
uniform vec3 LightPosition = vec3(0.25, 1.0, 3.0);
uniform vec3 LightIntensity = vec3(1.0);
uniform float Absorption = 1.0;
uniform mat4 Modelview;
uniform float FocalLength;
uniform vec2 WindowSize;
uniform vec3 RayOrigin;

uniform vec3 AmbientLight = vec3(1.0, 0.757, 0.55);
uniform float AmbientScale = 0.5;
const int AmbientRays = 6;
const int AmbientSteps = 12;

uniform int numSamples = 128;
uniform int numLightSamples = 16;

const float maxDist = sqrt(3.0);
const float densityFactor = 5;

struct Ray {
    vec3 Origin;
    vec3 Dir;
};

struct AABB {
    vec3 Min;
    vec3 Max;
};

bool IntersectBox(Ray r, AABB aabb, out float t0, out float t1)
{
    vec3 invR = 1.0 / r.Dir;
    vec3 tbot = invR * (aabb.Min-r.Origin);
    vec3 ttop = invR * (aabb.Max-r.Origin);
    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);
    vec2 t = max(tmin.xx, tmin.yz);
    t0 = max(t.x, t.y);
    t = min(tmax.xx, tmax.yz);
    t1 = min(t.x, t.y);
    return t0 <= t1;
}

// Originally by Stefan Gustavson, it has been upgraded and lives at:
// https://github.com/ashima/webgl-noise
float rand(vec2 co){
   return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}


vec3 ambientOcclusion(vec3 samplePos) {
   // Ambient occlusion, brute force
   float ambientIntensity = 1.0;
   float ambientScale = 1.0/(AmbientRays * AmbientSteps);
   for(int ao = 0; ao < AmbientRays; ++ao) {
      // Use the position as a seed to generate a random vector.
      vec3 aoDir = normalize(vec3(rand(samplePos.xy), rand(samplePos.xz), rand(samplePos.yz)));
      vec3 aoPos = samplePos + aoDir;

      for(int s = 0; s < AmbientSteps; ++s) {
         float cloudDensity = texture(Density, aoPos).x * densityFactor;
         ambientIntensity -= Absorption*cloudDensity*ambientScale;
         aoPos += aoDir;
      }
   }
   return AmbientLight * AmbientScale * ambientIntensity;
}

float shadowAttenuation(vec3 samplePos, vec3 lightDir, float stepSize){
   vec3 shadowSamplePos = samplePos + lightDir;

   // Shadow rays
   float sampledLightIntensity = 1.0;
   for(int s = 0; s < numLightSamples; ++s) {
      float cloudDensity = texture(Density, shadowSamplePos).x * densityFactor;
      sampledLightIntensity *= 1.0 - Absorption*stepSize*cloudDensity;
      if(sampledLightIntensity <= 0.01)
         shadowSamplePos += lightDir;
   }

   return sampledLightIntensity;
}


void main()
{
    float stepSize = maxDist / float(numSamples);
    float lightScale = maxDist / float(numLightSamples);

    vec3 rayDirection;
    rayDirection.xy = 2.0 * gl_FragCoord.xy / WindowSize - 1.0;
    rayDirection.z = -FocalLength;
    rayDirection = (vec4(rayDirection, 0) * Modelview).xyz;

    Ray eye = Ray( RayOrigin, normalize(rayDirection) );
    AABB aabb = AABB(vec3(-1.0), vec3(+1.0));

    float tnear, tfar;
    IntersectBox(eye, aabb, tnear, tfar);
    if (tnear < 0.0) tnear = 0.0;

    vec3 rayStart = eye.Origin + eye.Dir * tnear;
    vec3 rayStop = eye.Origin + eye.Dir * tfar;
    rayStart = 0.5 * (rayStart + 1.0);
    rayStop = 0.5 * (rayStop + 1.0);

    vec3 samplePos = rayStart;
    vec3 step = normalize(rayStop-rayStart) * stepSize;
    float travel = distance(rayStop, rayStart);
    float scaledSampleValue = 1.0;
    vec3 lightFrag = vec3(0.0);

    for (int i=0; i < numSamples && travel > 0.0; ++i, samplePos += step, travel -= stepSize) {
        float density = texture(Density, samplePos).x * densityFactor;
        if (density <= 0.0)
            continue;

        scaledSampleValue *= 1.0-density*stepSize*Absorption;
        if (scaledSampleValue <= 0.01)
            break;

        vec3 lightDir = normalize(LightPosition - samplePos)*lightScale;
        float sampledLightIntensity = shadowAttenuation(samplePos, lightDir, stepSize);

        vec3 Li = LightIntensity*(sampledLightIntensity + ambientOcclusion(samplePos));

        lightFrag += Li*scaledSampleValue*density*stepSize;
    }

    FragColor.rgb = lightFrag;
    FragColor.a = 1-scaledSampleValue;
}
