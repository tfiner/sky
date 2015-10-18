#include "Utility.h"

extern "C" {
#include "perlin.h"
}

#define _USE_MATH_DEFINES // for C++
#include <math.h>


using namespace vmath;
using std::string;

struct ProgramHandles {
    GLuint SinglePass;
    GLuint TwoPassRaycast;
    GLuint TwoPassIntervals;
    GLuint SkySphere;
};

static ProgramHandles Programs;
static GLuint CreatePyroclasticVolume(int n, float r);
static ITrackball* Trackball;
static GLuint CubeCenterVbo;
static Matrix4 ProjectionMatrix;
static Matrix4 ModelviewMatrix;
static Matrix4 ViewMatrix;
static Matrix4 ModelviewProjection;
static Point3 EyePosition;
static GLuint CloudTexture;
static SurfacePod IntervalsFbo[2];
static bool SinglePass = true;
static float FieldOfView = 0.7f;

PezConfig PezGetConfig()
{
    PezConfig config;
    config.Title = "Raycast";
    config.Width = 800;
    config.Height = 800;
    config.Multisampling = 0;
    config.VerticalSync = 0;
    return config;
}

void PezInitialize()
{
    PezConfig cfg = PezGetConfig();

    Trackball = CreateTrackball(cfg.Width * 1.0f, cfg.Height * 1.0f, cfg.Width * 0.5f);
    Programs.SinglePass = LoadProgram("SinglePass.VS", "SinglePass.GS", "SinglePass.FS");
    Programs.TwoPassIntervals = LoadProgram("TwoPass.VS", "TwoPass.Cube", "TwoPass.Intervals");
    Programs.TwoPassRaycast = LoadProgram("TwoPass.VS", "TwoPass.Fullscreen", "TwoPass.Raycast");

    Programs.SkySphere  = LoadProgram("SkySphere.VS", nullptr, "SkySphere.FS");
    CubeCenterVbo = CreatePointVbo(0, 0, 0);
    CloudTexture = CreatePyroclasticVolume(128, 0.025f);
    IntervalsFbo[0] = CreateSurface(cfg.Width, cfg.Height);
    IntervalsFbo[1] = CreateSurface(cfg.Width, cfg.Height);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static void LoadUniforms()
{
    SetUniform("ModelviewProjection", ModelviewProjection);
    SetUniform("Modelview", ModelviewMatrix);
    SetUniform("ViewMatrix", ViewMatrix);
    SetUniform("ProjectionMatrix", ProjectionMatrix);
    SetUniform("RayStartPoints", 1);
    SetUniform("RayStopPoints", 2);
    SetUniform("EyePosition", EyePosition);

    Vector4 rayOrigin(transpose(ModelviewMatrix) * EyePosition);
    SetUniform("RayOrigin", rayOrigin.getXYZ());

    float focalLength = 1.0f / std::tan(FieldOfView / 2);
    SetUniform("FocalLength", focalLength);

    PezConfig cfg = PezGetConfig();
    SetUniform("WindowSize", float(cfg.Width), float(cfg.Height));
}


// Sky simulation parameters:
auto LightDir        = normalize(Vector3(0.0f, -400.0f, 1000.0f));
auto NumSamples      = 3;		                  // Number of sample rays to use in integral equation

// Planetary constants
const auto EarthRadius        = 4.0f;
const auto AtmosphereRadius   = EarthRadius * 1.025f;
const auto AtmosphereScale    = 1.0f / (AtmosphereRadius - EarthRadius);

// Rayleigh / Mie Scattering constants.
const auto RayleighKr         = 0.0025f;		            // Rayleigh scattering constant
const auto RayleighKr4PI      = RayleighKr * 4.0f * M_PI;
const auto RayleighScaleDepth = 0.25f;

const auto MieKm           = 0.0010f;		            // Mie scattering constant
const auto MieKm4PI        = MieKm * 4.0f * M_PI;
const auto MieG            = -0.990f;		            // The Mie phase asymmetry factor
const auto MieScaleDepth   = 0.1f;

const auto SunBrightness   = 20.0f;

// Color wavelength scales
// User defined literal nanometers.
float operator "" _nm(long double);

auto constexpr Red_nm   = 650.0f;
auto constexpr Green_nm = 570.0f;
auto constexpr Blue_nm  = 475.0f;

auto WaveLength = Vector3(
   Red_nm / 1000.0f,
   Green_nm / 1000.0f,
   Blue_nm / 1000.0f
);

auto WaveLength4 = Vector3(
   powf(WaveLength[0], 4.0f),
   powf(WaveLength[1], 4.0f),
   powf(WaveLength[2], 4.0f)
);


// Take the camera position from the other code, and 
// put it in the same relative position.
auto CameraHeight = EarthRadius + 0.01f;
auto CameraPos = normalize(Vector3(6.946963f, 6.913678f, 2.205330f)) * CameraHeight;


void SkyRender() {
   ::glUseProgram(Programs.SkySphere);
   PezCheckCondition(GL_NO_ERROR == glGetError(), "Unable to use sky sphere");

   DumpUniforms();

   SetUniform("ModelviewProjection", ModelviewProjection);

   SetUniform("v3CameraPos",     CameraPos);
   SetUniform("fCameraHeight",   CameraHeight);

   SetUniform("v3LightDir",   LightDir);
   SetUniform("NumSamples",   NumSamples);
   SetUniform("fInnerRadius", EarthRadius);
   
   SetUniform("fKrESun",               RayleighKr * SunBrightness);
   SetUniform("fKr4PI",                RayleighKr4PI);
   SetUniform("fScaleDepth",           RayleighScaleDepth);
   SetUniform("fScaleOverScaleDepth",  AtmosphereScale / RayleighScaleDepth);

   SetUniform("fKmESun",   MieKm * SunBrightness);
   SetUniform("fKm4PI",    MieKm4PI);
   SetUniform("g",         MieG);
   SetUniform("g2",        MieG * MieG);

   SetUniform("v3InvWavelength", WaveLength4); 

   //    ::glDisable(GL_CULL_FACE);

   ::glFrontFace(GL_CW);
   ::glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

   auto pSphere = ::gluNewQuadric();
   ::gluSphere(pSphere, AtmosphereRadius, 100, 100);
   ::gluDeleteQuadric(pSphere);
}


void PezRender()
{  
    glBindBuffer(GL_ARRAY_BUFFER, CubeCenterVbo);
    glVertexAttribPointer(SlotPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(SlotPosition);

    glBindTexture(GL_TEXTURE_3D, CloudTexture);

    if (SinglePass)
    {
        glClearColor(0.2f, 0.2f, 0.2f, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        SkyRender();

        ::glFrontFace(GL_CCW);
        ::glPolygonMode(GL_FRONT, GL_FILL);
        ::glEnable(GL_CULL_FACE);
        ::glCullFace(GL_BACK);

        glUseProgram(Programs.SinglePass);
        LoadUniforms();
        glDrawArrays(GL_POINTS, 0, 1);
    }
    else
    {
        glUseProgram(Programs.TwoPassIntervals);
        LoadUniforms();
        glClearColor(0, 0, 0, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, IntervalsFbo[0].FboHandle);
        glClear(GL_COLOR_BUFFER_BIT);
        glCullFace(GL_FRONT);
        glDrawArrays(GL_POINTS, 0, 1);
        glBindFramebuffer(GL_FRAMEBUFFER, IntervalsFbo[1].FboHandle);
        glClear(GL_COLOR_BUFFER_BIT);
        glCullFace(GL_BACK);
        glDrawArrays(GL_POINTS, 0, 1);

        glUseProgram(Programs.TwoPassRaycast);
        LoadUniforms();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, IntervalsFbo[0].ColorTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, IntervalsFbo[1].ColorTexture);
        glClearColor(0.2f, 0.2f, 0.2f, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_POINTS, 0, 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }


    ::glBindBuffer(GL_ARRAY_BUFFER, 0);
    ::glBindTexture(GL_TEXTURE_3D, 0);
}

void PezUpdate(unsigned int microseconds)
{
    float dt = microseconds * 0.000001f;
    
    Trackball->Update(microseconds);

    EyePosition = Point3(0, 0, 5 + Trackball->GetZoom());
    Vector3 up(0, 1, 0); 
    Point3 target(0);

    ViewMatrix = Matrix4::lookAt(EyePosition, target, up);

    Matrix4 modelMatrix(transpose(Trackball->GetRotation()), Vector3(0));
    ModelviewMatrix = ViewMatrix * modelMatrix;

    float n = .050f;
    float f = 20.0f;

    ProjectionMatrix = Matrix4::perspective(FieldOfView, 1, n, f);
    ModelviewProjection = ProjectionMatrix * ModelviewMatrix;
}

void PezHandleMouse(int x, int y, int action)
{
    if (action & PEZ_DOWN)
        Trackball->MouseDown(x, y);
    else if (action & PEZ_UP)
        Trackball->MouseUp(x, y);
    else if (action & PEZ_MOVE)
        Trackball->MouseMove(x, y);
    else if (action & PEZ_DOUBLECLICK)
        Trackball->ReturnHome();
    else if (action & PEZ_ZOOM_IN)
       Trackball->MouseZoomIn();
    else if(action & PEZ_ZOOM_OUT)
       Trackball->MouseZoomOut();
}

void PezHandleKey(char c)
{
    if (c == ' ') SinglePass = !SinglePass;
    if (c == '1') FieldOfView += 0.05f;
    if (c == '2') FieldOfView -= 0.05f;
}

static GLuint CreatePyroclasticVolume(int n, float r)
{
    GLuint handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_3D, handle);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    unsigned char *data = new unsigned char[n*n*n];
    unsigned char *ptr = data;

    float frequency = 3.0f / n;
    float center = n / 2.0f + 0.5f;

    for(int x=0; x < n; ++x) {
        for (int y=0; y < n; ++y) {
            for (int z=0; z < n; ++z) {
                float dx = center-x;
                float dy = center-y;
                float dz = center-z;

                float off = fabsf((float) PerlinNoise3D(
                    x*frequency,
                    y*frequency,
                    z*frequency,
                    5,
                    6, 3));

                float d = sqrtf(dx*dx+dy*dy+dz*dz)/(n);
                bool isFilled = (d-off) < r;
                *ptr++ = isFilled ? 255 : 0;
            }
        }
        PezDebugString("Slice %d of %d\n", x, n);
    }

    glTexImage3D(GL_TEXTURE_3D, 0,
                 GL_LUMINANCE,
                 n, n, n, 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 data);

    delete[] data;
    return handle;
}
