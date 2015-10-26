#include "Utility.h"

extern "C" {
#include "perlin.h"
}

#define _USE_MATH_DEFINES // for C++
#include <math.h>
#include <time.h>

#include <algorithm>
#include <array>
#include <vector>


using namespace vmath;
using std::string;

namespace {

   struct ProgramHandles {
      GLuint SinglePass;
      GLuint TwoPassRaycast;
      GLuint TwoPassIntervals;
      GLuint SkySphere;
      GLuint TestProgram;
   } Programs;

   ITrackball* Trackball;
   GLuint CubeCenterVbo;
   Matrix4 ProjectionMatrix;
   Matrix4 ModelviewMatrix;
   Matrix4 ViewMatrix;
   Matrix4 ModelviewProjection;
   Point3 EyePosition;
   GLuint CloudTexture;
   SurfacePod IntervalsFbo[2];
   bool SinglePass = true;
   float FieldOfView = 0.7f;

   // Cloud variables
   auto Absorption = 1.0f;

   // Common to sky and cloud.
   auto LightDir = -Vector3(0.0f, 0.0f, 1.0f);
   auto SunBrightness = 20.0f;

   void LoadUniforms() {
      SetUniform("ModelviewProjection", ModelviewProjection);
      SetUniform("Modelview", ModelviewMatrix);
      SetUniform("ViewMatrix", ViewMatrix);
      SetUniform("ProjectionMatrix", ProjectionMatrix);
      SetUniform("RayStartPoints", 1);
      SetUniform("RayStopPoints", 2);
      SetUniform("EyePosition", EyePosition);
      SetUniform("LightPosition", LightDir * 100);

      SetUniform("LightIntensity", Vector3(SunBrightness * .75f));
      

      Vector4 rayOrigin(transpose(ModelviewMatrix) * EyePosition);
      SetUniform("RayOrigin", rayOrigin.getXYZ());

      float focalLength = 1.0f / std::tan(FieldOfView / 2);
      SetUniform("FocalLength", focalLength);

      PezConfig cfg = PezGetConfig();
      SetUniform("WindowSize", float(cfg.Width), float(cfg.Height));
      SetUniform("Absorption", Absorption);
   }


   GLuint NewTexture() {
      GLuint handle;
      glGenTextures(1, &handle);
      return handle;
   }


   void CreatePyroclasticVolume(GLuint handle, int n, float r) {
      const auto seed = static_cast<unsigned int>(time(nullptr));
      PezDebugString("Starting seed: %u\n", seed);
      srand(seed);
      PerlinInit();

      glBindTexture(GL_TEXTURE_3D, handle);
      PezCheckCondition(GL_NO_ERROR == glGetError(), "Unable to bind texture handle");
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      auto v = std::vector<unsigned char>(n*n*n);
      unsigned char *ptr = v.data();

      float frequency = 3.0f / n;
      float center = n / 2.0f + 0.5f;

      for(int x = 0; x < n; ++x) {
         for(int y = 0; y < n; ++y) {
            for(int z = 0; z < n; ++z) {
               float dx = center - x;
               float dy = center - y;
               float dz = center - z;

               float off = fabsf((float)PerlinNoise3D(
                  x*frequency,
                  y*frequency,
                  z*frequency,
                  5,
                  6, 3));

               float d = sqrtf(dx*dx + dy*dy + dz*dz) / (n);
               bool isFilled = (d - off) < r;
               *ptr++ = isFilled ? 255 : 0;
            }
         }
         // PezDebugString("Slice %d of %d\n", x, n);
      }

      glTexImage3D(
         GL_TEXTURE_3D, 
         0,
         GL_LUMINANCE,
         n, n, n, 0,
         GL_LUMINANCE,
         GL_UNSIGNED_BYTE,
         v.data());

      PezCheckCondition(GL_NO_ERROR == glGetError(), "Unable to set texture bits");
   }


}

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

    Programs.SkySphere  = LoadProgram("SkySphere.VS", nullptr/*"SkySphere.GS"*/, "SkySphere.FS");
    CubeCenterVbo = CreatePointVbo(0, 0, 0);
    CloudTexture = NewTexture();
    CreatePyroclasticVolume(CloudTexture, 128, 0.05f);
    IntervalsFbo[0] = CreateSurface(cfg.Width, cfg.Height);
    IntervalsFbo[1] = CreateSurface(cfg.Width, cfg.Height);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


// Sky simulation parameters:
auto NumSamples   = 20;		                  // Number of sample rays to use in integral equation
auto Scale        = 4.0f;

// Planetary constants
const auto EarthRadius        = 10.0f;
const auto AtmosphereRadius   = EarthRadius * 1.025f;
const auto AtmosphereScale    = 1.0f / (AtmosphereRadius - EarthRadius);

// Rayleigh / Mie Scattering constants.
auto RayleighKr         = 0.0025f;		            // Rayleigh scattering constant
auto RayleighKr4PI      = static_cast<float>(RayleighKr * 4.0f * M_PI);
auto RayleighScaleDepth = 0.40f;

auto MieKm                 = 0.0010f;		            // Mie scattering constant
auto MieKm4PI              = static_cast<float>(MieKm * 4.0f * M_PI);
const auto MieG            = -0.990f;		            // The Mie phase asymmetry factor
const auto MieScaleDepth   = 0.1f;

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

auto InvWaveLength4 = Vector3(
   1.0f / powf(WaveLength[0], 4.0f),
   1.0f / powf(WaveLength[1], 4.0f),
   1.0f / powf(WaveLength[2], 4.0f)
);


// Take the camera position from the other code, and 
// put it in the same relative position.
auto CameraPos = normalize(Vector3(0.0f, 1.0f, 0.0f)) * 9.75;/*Vector3(6.94696283f, 6.91367817f, 2.20532990f)*/;
//auto CameraPos = normalize(Vector3(0.0f, 1.0f, 0.0f)) * 9.9;/*Vector3(6.94696283f, 6.91367817f, 2.20532990f)*/;
auto CameraHeight = EarthRadius * 1.01f;

void SkyRender() {
   ::glUseProgram(Programs.SkySphere);
   PezCheckCondition(GL_NO_ERROR == glGetError(), "Unable to use sky sphere");

   SetUniform("ModelviewProjection", ModelviewProjection);

   SetUniform("v3CameraPos",     CameraPos);
   SetUniform("fCameraHeight",   CameraHeight);

   SetUniform("v3LightDir",   LightDir);
   SetUniform("NumSamples",   NumSamples);
   SetUniform("fInnerRadius", EarthRadius);
   SetUniform("fScale",       Scale);
   
   SetUniform("fKrESun",               RayleighKr * SunBrightness);
   SetUniform("fKr4PI",                RayleighKr4PI);
   SetUniform("fScaleDepth",           RayleighScaleDepth);
   SetUniform("fScaleOverScaleDepth",  AtmosphereScale / RayleighScaleDepth);

   SetUniform("fKmESun",   MieKm * SunBrightness);
   SetUniform("fKm4PI",    MieKm4PI);
   SetUniform("g",         MieG);
   SetUniform("g2",        MieG * MieG);

   SetUniform("v3InvWavelength", InvWaveLength4 );

//   DumpUniforms();

   //    ::glDisable(GL_CULL_FACE);

   ::glFrontFace(GL_CW);
   ::glPolygonMode(GL_FRONT, GL_FILL/*GL_LINE*/);
   //::glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

   auto pSphere = ::gluNewQuadric();
   ::gluSphere(pSphere, EarthRadius*Scale, 150, 150);
   ::gluDeleteQuadric(pSphere);
   //glDrawArrays(GL_POINTS, 0, 1);
}

   auto pSphere = ::gluNewQuadric();
   ::gluSphere(pSphere, AtmosphereRadius*4, 150, 150);
   ::gluDeleteQuadric(pSphere);
   //glDrawArrays(GL_POINTS, 0, 1);
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
    Point3 target(0,0,0.0);

    ViewMatrix = Matrix4::lookAt(EyePosition, target, up);

    Matrix4 modelMatrix(transpose(Trackball->GetRotation()), Vector3(0));
    ModelviewMatrix = ViewMatrix * modelMatrix;

    float n = .050f;
    float f = AtmosphereRadius * Scale * 1.5f;

    ProjectionMatrix = Matrix4::perspective(FieldOfView, 1, n, f);
    ModelviewProjection = ProjectionMatrix * ModelviewMatrix;
}

void PezHandleMouse(int x, int y, int action) {
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


void PezHandleKey(char c, int flags) {
   switch(c) {
   case ' ':
      SinglePass = !SinglePass;
      break;

   case 'C':
      CreatePyroclasticVolume(CloudTexture, 128, 0.05f);
      break;

   case 'D':
      Scale += (flags & PEZ_SHIFT) ? -0.1f : 0.1f;
      Scale = (std::max)(Scale, 0.0f);
      PezDebugString("Scale: %3.2f\n", Scale);
      break;

   case 'H':
      CameraHeight += (flags & PEZ_SHIFT) ? -0.005f : 0.005f;
      PezDebugString("CameraHeight: %3.2f\n", CameraHeight);
      break;

   case 'S':
      SunBrightness += (flags & PEZ_SHIFT) ? -1.0f : 1.0f;
      SunBrightness = (std::max)(SunBrightness, 0.0f);
      PezDebugString("SunBrightness: %3.2f\n", SunBrightness);
      break;

   case 'N':
      NumSamples += (flags & PEZ_SHIFT) ? -1 : 1;
      NumSamples = (std::max)(NumSamples, 2);
      PezDebugString("NumSamples: %d\n", NumSamples);
      break;

   case 'M':
      MieKm += (flags & PEZ_SHIFT) ? -0.0001f : 0.0001f;
      MieKm = (std::max)(MieKm, 0.0f);
      MieKm4PI = static_cast<float>(MieKm * 4.0f * M_PI);
      PezDebugString("MieKm: %0.4f\n", MieKm);
      break;

   case 'R':
      if(flags & PEZ_CTRL) {
         RayleighScaleDepth += (flags & PEZ_SHIFT) ? -0.01f : 0.01f;
         RayleighScaleDepth = (std::max)(RayleighScaleDepth, 0.0f);
         PezDebugString("RayleighScaleDepth: %0.4f\n", RayleighScaleDepth);

      } else {
         RayleighKr += (flags & PEZ_SHIFT) ? -0.0001f : 0.0001f;
         RayleighKr = (std::max)(RayleighKr, 0.0f);
         RayleighKr4PI = static_cast<float>(RayleighKr * 4.0f * M_PI);
         PezDebugString("RayleighKr: %0.4f\n", RayleighKr);
      }
      break;


   case 'A':
      Absorption += (flags & PEZ_SHIFT) ? -0.1f : 0.1f;
      Absorption = (std::max)(Absorption, 0.0f);
      PezDebugString("Absorption: %0.4f\n", Absorption);
      break;

   case'1':
      FieldOfView += 0.05f;
      break;

   case'2':
      FieldOfView -= 0.05f;
      break;

   case '&':
      Trackball->PanUp();
      PezDebugString("pan up\n");
      break;

   case '(':
      Trackball->PanDown();
      PezDebugString("pan down\n");
      break;

   case '%':
      Trackball->PanLeft();
      PezDebugString("pan left\n");
      break;

   case '\'':
      Trackball->PanRight();
      PezDebugString("pan right\n");
      break;

   case '-':
      Trackball->RollLeft();
      PezDebugString("roll left\n");
      break;

   case '!':
      Trackball->RollRight();
      PezDebugString("roll right\n");
      break;

   default:
      if (isprint(c))
         PezDebugString("Key: %c 0x%02x\n", c, flags);
   }

    
}

