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

// Dark orange
uniform vec3 ShadowLight = vec3(1.0, 0.757, 0.55) * 0.5;

// Orange
// uniform vec3 AmbientLight = vec3(1.0, 0.757, 0.55);
// Blue
//uniform vec3 AmbientLight = vec3(.196, 0.557, 0.831);
uniform vec3 AmbientLight = vec3(1.0, 0.0, 0.0);

uniform int AmbientSamples = 4;
uniform float AmbientScale = 1.0;

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


float ambientSample(vec3 samplePos) {
   return texture(Density, samplePos).x /** densityFactor*/;
   // return .5;
}


vec3 ambientOcclusion(vec3 samplePos, float stepSize) {
   // Ambient occlusion, brute force
   float ambientIntensity = 1.0;
   float sampleScale = 1.0/6.0 * AmbientSamples;

   float step = stepSize;
   for(int ao = 0; ao < AmbientSamples; ++ao, step+=stepSize) {

      // Look in the surrounding 6 directions for density.
      ambientIntensity -= 
         ambientSample(samplePos + vec3(1.0, 0.0, 0.0) * step) *
         sampleScale;

      ambientIntensity -= 
         ambientSample(samplePos + vec3(-1.0, 0.0, 0.0) * step) *
         sampleScale;

      ambientIntensity -= 
         ambientSample(samplePos + vec3(0.0, 1.0, 0.0) * step) *
         sampleScale;

      ambientIntensity -= 
         ambientSample(samplePos + vec3(0.0, -1.0, 0.0) * step) *
         sampleScale;

      ambientIntensity -= 
         ambientSample(samplePos + vec3(0.0, 0.0, 1.0) * step) *
         sampleScale;

      ambientIntensity -= 
         ambientSample(samplePos + vec3(0.0, 0.0, -1.0) * step) *
         sampleScale;
   }

   // return AmbientLight * AmbientScale;
   return AmbientLight * AmbientScale * min(1.0, max(0.25, ambientIntensity));
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

   return min(1.0, max(0.10, sampledLightIntensity));
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
        //float sampledLightIntensity = 1.0;

        vec3 Li = LightIntensity * sampledLightIntensity + ShadowLight * sampledLightIntensity;
        lightFrag += Li*scaledSampleValue*density*stepSize;
        lightFrag += ambientOcclusion(samplePos, stepSize) * stepSize;
    }

    FragColor.rgb = lightFrag;
    FragColor.a = 1-scaledSampleValue;
}
