#version 150

out vec4 FragColor;

uniform sampler3D Density;
uniform vec3 LightPosition = vec3(0.25, 1.0, 3.0);
uniform vec3 LightIntensity = vec3(15.0);
uniform float Absorption = 1.0;
uniform mat4 Modelview;
uniform float FocalLength;
uniform vec2 WindowSize;
uniform vec3 RayOrigin;

const float maxDist = sqrt(2.0);
const int numSamples = 128;
const float stepSize = maxDist/float(numSamples);
const int numLightSamples = 32;
const float lscale = maxDist / float(numLightSamples);
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

void main()
{
    FragColor = vec4(1.0,0.0,0.0,1.0);
}

