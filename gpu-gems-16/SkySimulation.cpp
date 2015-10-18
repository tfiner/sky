/*
s_p_oneil@hotmail.com
Copyright (c) 2000, Sean O'Neil
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
* Neither the name of this project nor the names of its contributors
  may be used to endorse or promote products derived from this software
  without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include "Master.h"
#include "OpenGLApp.h"
#include "SkySimulation.h"
#include "GLUtil.h"
#include "perlin.h"

#include <tfgl/Exception.h>
#include <tfgl/ScopedBinder.h>
#include <tfgl/ScopedEnable.h>
#include <tfgl/Shader.h>
#include <tfgl/Program.h>

#include <GLFW/glfw3.h>

#include <algorithm>


namespace {
   // Camera rotation:
   const auto RadiansPerSecond = 1.0f;

   // Camera acceleration rate due to thrusters (units/s*s):
   const auto Thrust = 1.0f;

   // Camera damping effect on velocity:
   const auto Resistance = 0.1f;

   const auto PI = 3.1415927f;


   GLuint CreatePointVbo(float x, float y, float z)
   {
      float p[] = {x, y, z};
      GLuint vbo;
      glGenBuffers(1, &vbo);
      THROW_ON_GL_ERROR();

      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      THROW_ON_GL_ERROR();

      glBufferData(GL_ARRAY_BUFFER, sizeof(p), &p[0], GL_STATIC_DRAW);
      THROW_ON_GL_ERROR();

      return vbo;
   }


   static GLuint CreatePyroclasticVolume(int n, float r) {
      GLuint handle;
      glGenTextures(1, &handle);
      THROW_ON_GL_ERROR();

      glBindTexture(GL_TEXTURE_3D, handle);
      THROW_ON_GL_ERROR();

      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      THROW_ON_GL_ERROR();

      unsigned char *data = new unsigned char[n*n*n];
      unsigned char *ptr = data;

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
      }

      glTexImage3D(GL_TEXTURE_3D, 0,
         GL_LUMINANCE,
         n, n, n, 0,
         GL_LUMINANCE,
         GL_UNSIGNED_BYTE,
         data);

      THROW_ON_GL_ERROR();

      delete[] data;
      return handle;
   }


   std::unique_ptr<SkySimulation::Surface> CreateSurface(GLsizei width, GLsizei height){
      GLuint fboHandle;
      glGenFramebuffers(1, &fboHandle);
      THROW_ON_GL_ERROR();

      glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
      THROW_ON_GL_ERROR();

      GLuint textureHandle;
      glGenTextures(1, &textureHandle);
      THROW_ON_GL_ERROR();

      glBindTexture(GL_TEXTURE_2D, textureHandle);
      THROW_ON_GL_ERROR();

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_HALF_FLOAT, 0);
      THROW_ON_GL_ERROR()

      GLuint colorbuffer;
      glGenRenderbuffers(1, &colorbuffer);
      glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureHandle, 0);
      THROW_ON_GL_ERROR()

      glClearColor(0, 0, 0, 0);
      glClear(GL_COLOR_BUFFER_BIT);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      THROW_ON_GL_ERROR();

      return std::make_unique<SkySimulation::Surface>(fboHandle, textureHandle);
   }

}


struct SkySimulation::Surface {
   Surface() : fbo_(0), colorTexture_(0) {}
   Surface(GLuint fbo, GLuint colorTex) : fbo_(fbo), colorTexture_(colorTex) {}

   GLuint fbo_;
   GLuint colorTexture_;
};


SkySimulation::SkySimulation(){
	m_bUseHDR = true;
	GLUtil()->Init();

	m_nPolygonMode = GL_FILL;

   sky_.Init(1024, 1024, 0);
   sky_.SetExposure(2.0f);

	CQuaternion qOrientation(0.404703f, 0.910458f, 0.040314f, 0.075190f);
	qOrientation.Normalize();
   m_3DCamera
      .SetPosition(CDoubleVector(6.946963, 6.913678, 2.205330))
      .SetOrientation(qOrientation);

   light_ = glm::vec3(0.0f, -400.0f, 1000.0f);
   lightDir_ = glm::normalize(light_);

	m_nSamples = 3;		// Number of sample rays to use in integral equation
	m_Kr = 0.0025f;		// Rayleigh scattering constant
	m_Kr4PI = m_Kr*4.0f*PI;
	m_Km = 0.0010f;		// Mie scattering constant
	m_Km4PI = m_Km*4.0f*PI;
	m_ESun = 20.0f;		// Sun brightness constant
	m_g = -0.990f;		// The Mie phase asymmetry factor

	m_fInnerRadius = 10.0f;
	m_fOuterRadius = 10.25f;

	m_fWavelength[0] = 0.650f;		// 650 nm for red
	m_fWavelength[1] = 0.570f;		// 570 nm for green
	m_fWavelength[2] = 0.475f;		// 475 nm for blue
	m_fWavelength4[0] = powf(m_fWavelength[0], 4.0f);
	m_fWavelength4[1] = powf(m_fWavelength[1], 4.0f);
	m_fWavelength4[2] = powf(m_fWavelength[2], 4.0f);

	m_fRayleighScaleDepth = 0.25f;
	m_fMieScaleDepth = 0.1f;
	// m_pbOpticalDepth.MakeOpticalDepthBuffer(m_fInnerRadius, m_fOuterRadius, m_fRayleighScaleDepth, m_fMieScaleDepth);

   cubeCenterVbo_ = cloudTexture_ = 0u;
}


SkySimulation::~SkySimulation(){}


void SkySimulation::RenderFrame(GLFWwindow* win, unsigned milliseconds, int width, int height) {
   InitShaders();

	// Move the camera
	HandleInput(win, milliseconds * 0.001f);

   SetContext();

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


   glPushMatrix();
   glLoadMatrixf(m_3DCamera.GetViewMatrix().Data());

   C3DObject obj;
   glMultMatrixf(obj.GetModelMatrix(&m_3DCamera).Data());

   CVector vCamera = m_3DCamera.GetPosition();
   CVector vUnitCamera = vCamera / vCamera.Magnitude();

   if(renderClouds_)
      RenderClouds(width, height);
   else {
      RenderGround(vCamera);
      RenderSky(vCamera);
   }

   glPopMatrix();

	glFlush();

   RenderHDR(width, height);
}


void SkySimulation::SetContext() {
   if(m_bUseHDR)
      sky_.SetContext();
   else
      GLUtil()->MakeCurrent();
}


// Lazily initialize shaders, because they can't be loaded until an OpenGL context
// has been created.
void SkySimulation::InitShaders() {
   if(!skyFromAtmosphere_)
      skyFromAtmosphere_ = tfgl::MakeProgram("SkyFromAtmosphere");

   if (!groundFromAtmosphere_)
      groundFromAtmosphere_ = tfgl::MakeProgram("GroundFromAtmosphere");

   if(!raycast_)   {
      auto bindAttr = [this](GLuint program) {
         ::glBindAttribLocation(program, CloudAttr::SlotPosition, "Position");
         THROW_ON_GL_ERROR();

         ::glBindAttribLocation(program, CloudAttr::SlotTexCoord, "TexCoord");
         THROW_ON_GL_ERROR();
      };

      raycast_ = tfgl::MakeProgram("test.vert", "test.frag", "test.geom", bindAttr);
   }
}


void SkySimulation::RenderHDR(int width, int height) {
   if(!m_bUseHDR)
      return;

   GLUtil()->MakeCurrent();
   glViewport(0, 0, width, height);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glPushMatrix();
   glLoadIdentity();
   glMatrixMode(GL_PROJECTION);

   glPushMatrix();
   glLoadIdentity();
   glOrtho(0, 1, 0, 1, -1, 1);

   {
      tfgl::ScopedBinder<sky::HdrSky> hdrBind(sky_);

      glBegin(GL_QUADS);
      glTexCoord2f(0, 0); glVertex2f(0, 0);	// For rect texture, can't use 1 as the max texture coord
      glTexCoord2f(1, 0); glVertex2f(1, 0);
      glTexCoord2f(1, 1); glVertex2f(1, 1);
      glTexCoord2f(0, 1); glVertex2f(0, 1);
      glEnd();
   }

   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
}


void SkySimulation::RenderSky(CVector &vCamera) {
   auto skyShader = skyFromAtmosphere_.get();

   tfgl::ScopedBinder<tfgl::Program> shaderBind(*skyShader);
   skyShader->SetUniform("v3CameraPos", vCamera.x, vCamera.y, vCamera.z);
   skyShader->SetUniform("v3LightPos", lightDir_.x, lightDir_.y, lightDir_.z);
   skyShader->SetUniform("v3InvWavelength", 1 / m_fWavelength4[0], 1 / m_fWavelength4[1], 1 / m_fWavelength4[2]);
   skyShader->SetUniform("fCameraHeight", vCamera.Magnitude());
   skyShader->SetUniform("fCameraHeight2", vCamera.MagnitudeSquared());
   skyShader->SetUniform("fInnerRadius", m_fInnerRadius);
   skyShader->SetUniform("fInnerRadius2", m_fInnerRadius*m_fInnerRadius);
   skyShader->SetUniform("fOuterRadius", m_fOuterRadius);
   skyShader->SetUniform("fOuterRadius2", m_fOuterRadius*m_fOuterRadius);
   skyShader->SetUniform("fKrESun", m_Kr*m_ESun);
   skyShader->SetUniform("fKmESun", m_Km*m_ESun);
   skyShader->SetUniform("fKr4PI", m_Kr4PI);
   skyShader->SetUniform("fKm4PI", m_Km4PI);
   skyShader->SetUniform("fScale", 1.0f / (m_fOuterRadius - m_fInnerRadius));
   skyShader->SetUniform("fScaleDepth", m_fRayleighScaleDepth);
   skyShader->SetUniform("fScaleOverScaleDepth", (1.0f / (m_fOuterRadius - m_fInnerRadius)) / m_fRayleighScaleDepth);
   skyShader->SetUniform("g", m_g);
   skyShader->SetUniform("g2", m_g*m_g);
   skyShader->SetUniform("nSamples", m_nSamples);

   // Render the inside of the sky sphere.
   ::glFrontFace(GL_CW);

   auto pSphere = ::gluNewQuadric();
   ::gluSphere(pSphere, m_fOuterRadius, 100, 100);
   ::gluDeleteQuadric(pSphere);

   ::glFrontFace(GL_CCW);
}


void SkySimulation::RenderGround(CVector &vCamera) {
   auto groundShader = groundFromAtmosphere_.get();

   tfgl::ScopedBinder<tfgl::Program> shaderBind(*groundShader);
   groundShader->SetUniform("v3CameraPos", vCamera.x, vCamera.y, vCamera.z);
   groundShader->SetUniform("v3LightPos", lightDir_.x, lightDir_.y, lightDir_.z);
   groundShader->SetUniform("v3InvWavelength", 1 / m_fWavelength4[0], 1 / m_fWavelength4[1], 1 / m_fWavelength4[2]);
   groundShader->SetUniform("fCameraHeight", vCamera.Magnitude());
   groundShader->SetUniform("fCameraHeight2", vCamera.MagnitudeSquared());
   groundShader->SetUniform("fInnerRadius", m_fInnerRadius);
   groundShader->SetUniform("fInnerRadius2", m_fInnerRadius*m_fInnerRadius);
   groundShader->SetUniform("fOuterRadius", m_fOuterRadius);
   groundShader->SetUniform("fOuterRadius2", m_fOuterRadius*m_fOuterRadius);
   groundShader->SetUniform("fKrESun", m_Kr*m_ESun);
   groundShader->SetUniform("fKmESun", m_Km*m_ESun);
   groundShader->SetUniform("fKr4PI", m_Kr4PI);
   groundShader->SetUniform("fKm4PI", m_Km4PI);
   groundShader->SetUniform("fScale", 1.0f / (m_fOuterRadius - m_fInnerRadius));
   groundShader->SetUniform("fScaleDepth", m_fRayleighScaleDepth);
   groundShader->SetUniform("fScaleOverScaleDepth", (1.0f / (m_fOuterRadius - m_fInnerRadius)) / m_fRayleighScaleDepth);
   groundShader->SetUniform("g", m_g);
   groundShader->SetUniform("g2", m_g*m_g);
   groundShader->SetUniform("s2Test", 0);

   GLUquadricObj *pSphere = gluNewQuadric();
   gluSphere(pSphere, m_fInnerRadius, 100, 100);
   gluDeleteQuadric(pSphere);
}


void SkySimulation::InitClouds() {
   if(!initClouds) {
      cubeCenterVbo_ = CreatePointVbo(0, 0, 0);
      cloudTexture_ = CreatePyroclasticVolume(128, 0.025f);
      intervals_[0] = CreateSurface(800, 800);
      intervals_[1] = CreateSurface(800, 800);
      initClouds = true;
   }
}


void SkySimulation::SetCloudUniforms(int width, int height) {
   const auto viewMatrix = MatToGlm(m_3DCamera.GetViewMatrix());
   const auto modelMatrix = MatToGlm(m_3DCamera.GetModelMatrix(&m_3DCamera));
   const auto modelView = viewMatrix * modelMatrix;
   raycast_->SetUniformMat4("Modelview", glm::value_ptr(modelView));

   const auto projMatrix = glm::perspectiveFov(45.0f, static_cast<float>(width), static_cast<float>(height), 1.0f, 100.0f);
   const auto modelViewProj = projMatrix * modelView;
   raycast_->SetUniformMat4("ModelviewProjection", glm::value_ptr(modelViewProj));

   const auto rayOrigin = glm::transpose(modelView) * glm::vec4(m_3DCamera.GetPositionGlm(), 0.0);
   raycast_->SetUniformVec3("RayOrigin", glm::value_ptr(rayOrigin));

   const auto focalLength = static_cast<float>(1.0 / std::tan(45.0 / 2.0));
   raycast_->SetUniform("FocalLength", focalLength);
   raycast_->SetUniform("WindowSize", static_cast<float>(width), static_cast<float>(height));
}


void SkySimulation::RenderClouds(int width, int height) {
   InitClouds();

   tfgl::ScopedDisable depth(GL_DEPTH_TEST);
   tfgl::ScopedEnable blend(GL_BLEND);
   tfgl::ScopedEnable cullFace(GL_CULL_FACE);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   glBindBuffer(GL_ARRAY_BUFFER, cubeCenterVbo_);
   THROW_ON_GL_ERROR();

   glVertexAttribPointer(SlotPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
   THROW_ON_GL_ERROR();

   glEnableVertexAttribArray(SlotPosition);
   THROW_ON_GL_ERROR();

   glBindTexture(GL_TEXTURE_3D, cloudTexture_);
   THROW_ON_GL_ERROR();

   //glClearColor(0.2f, 0.2f, 0.2f, 0);
   // glClear(GL_COLOR_BUFFER_BIT);

   auto raycast = raycast_.get();
   tfgl::ScopedBinder<tfgl::Program> shaderBind(*raycast);

   SetCloudUniforms(width, height);

   glDrawArrays(GL_POINTS, 0, 1);
   THROW_ON_GL_ERROR();
}



void SkySimulation::OnChar(int upperC) {
   const auto c = tolower(upperC);
	switch(c)
	{
		case 'p':
			m_nPolygonMode = (m_nPolygonMode == GL_FILL) ? GL_LINE : GL_FILL;
			glPolygonMode(GL_FRONT, m_nPolygonMode);
			break;
		
      case 'h':
			m_bUseHDR = !m_bUseHDR;
			break;
		
      case '=':
			m_nSamples++;
			break;

      case '-':
			m_nSamples--;
			break;

      case 'c':
         renderClouds_ = !renderClouds_;
         break;
	}
}


bool IsKeyDown(GLFWwindow* win, const char c, bool& shifted) {
   if(!glfwGetKey(win, c))
      return false;

   shifted = glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT) ||
             glfwGetKey(win, GLFW_KEY_LEFT_SHIFT);

   return true;
}


void SkySimulation::HandleInput(GLFWwindow* win, float seconds) {
   InputRenderParams(win);
   InputCameraOrient(win, seconds);
   InputCameraPosition(win, seconds);
}


void SkySimulation::InputCameraPosition(GLFWwindow* win, float seconds) {
   CVector vAccel(0.0f);
   if(glfwGetKey(win, GLFW_KEY_SPACE)) {
      m_3DCamera.SetVelocity(CVector(0.0f));	// Full stop
   } else {
      // Add camera's acceleration due to thrusters
      auto thrust = Thrust;
      if(glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) ||
         glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL)) {
         thrust *= 10.0f;
      }

      // Thrust forward/reverse affects velocity along the view axis
      if(glfwGetKey(win, GLFW_KEY_W))
         vAccel += m_3DCamera.GetViewAxis() * thrust;
      if(glfwGetKey(win, GLFW_KEY_S))
         vAccel += m_3DCamera.GetViewAxis() * -thrust;

      // Thrust left/right affects velocity along the right axis
      if(glfwGetKey(win, GLFW_KEY_D))
         vAccel += m_3DCamera.GetRightAxis() * thrust;
      if(glfwGetKey(win, GLFW_KEY_A))
         vAccel += m_3DCamera.GetRightAxis() * -thrust;

      // Thrust up/down affects velocity along the up axis
      CVector v = m_3DCamera.GetPosition();
      v.Normalize();
      if(GetKeyState('M') & 0x8000)
         vAccel += v * thrust;
      if(GetKeyState('N') & 0x8000)
         vAccel += v * -thrust;

      m_3DCamera.Accelerate(vAccel, seconds, Resistance);
      CVector vPos = m_3DCamera.GetPosition();
      float fMagnitude = vPos.Magnitude();
      if(fMagnitude < m_fInnerRadius) {
         vPos *= (m_fInnerRadius * (1 + DELTA)) / fMagnitude;
         m_3DCamera.SetPosition(CDoubleVector(vPos.x, vPos.y, vPos.z));
         m_3DCamera.SetVelocity(-m_3DCamera.GetVelocity());
      }
   }
}

void SkySimulation::InputCameraOrient(GLFWwindow* win, float seconds) {
   if(glfwGetKey(win, GLFW_KEY_RIGHT) || glfwGetKey(win, GLFW_KEY_KP_6))
      m_3DCamera.Rotate(m_3DCamera.GetUpAxis(), seconds * -RadiansPerSecond);

   if(glfwGetKey(win, GLFW_KEY_LEFT) || glfwGetKey(win, GLFW_KEY_KP_4))
      m_3DCamera.Rotate(m_3DCamera.GetUpAxis(), seconds * RadiansPerSecond);

   if(glfwGetKey(win, GLFW_KEY_UP) || glfwGetKey(win, GLFW_KEY_KP_8))
      m_3DCamera.Rotate(m_3DCamera.GetRightAxis(), seconds * -RadiansPerSecond);

   if(glfwGetKey(win, GLFW_KEY_DOWN) || glfwGetKey(win, GLFW_KEY_KP_2))
      m_3DCamera.Rotate(m_3DCamera.GetRightAxis(), seconds * RadiansPerSecond);

   if(glfwGetKey(win, GLFW_KEY_KP_7))
      m_3DCamera.Rotate(m_3DCamera.GetViewAxis(), seconds * -RadiansPerSecond);

   if(glfwGetKey(win, GLFW_KEY_KP_9))
      m_3DCamera.Rotate(m_3DCamera.GetViewAxis(), seconds * RadiansPerSecond);
}


void SkySimulation::InputRenderParams(GLFWwindow* win) {
   bool isShifted = false;
   if(const auto isOne = IsKeyDown(win, '1', isShifted)){
      if(isShifted)
         m_Kr = (std::max)(0.0f, m_Kr - 0.0001f);
      else
         m_Kr += 0.0001f;

      m_Kr4PI = m_Kr*4.0f*PI;

   }
   else if(const auto isTwo = IsKeyDown(win,  '2', isShifted))	{
      if(isShifted)
         m_Km = (std::max)(0.0f, m_Km - 0.0001f);
      else
         m_Km += 0.0001f;

      m_Km4PI = m_Km*4.0f*PI;

   }
   else if(IsKeyDown(win,  '3', isShifted)) {
      if(isShifted)
         m_g = (std::max)(-1.0f, m_g - 0.001f);
      else
         m_g = (std::min)(1.0f, m_g + 0.001f);

   }
   else if(IsKeyDown(win,  '4', isShifted))	{
      if(isShifted)
         m_ESun = (std::max)(0.0f, m_ESun - 0.1f);
      else
         m_ESun += 0.1f;

   }
   else if(IsKeyDown(win,  '5', isShifted)) {
      if(isShifted)
         m_fWavelength[0] = (std::max)(0.001f, m_fWavelength[0] -= 0.001f);
      else
         m_fWavelength[0] += 0.001f;

      m_fWavelength4[0] = powf(m_fWavelength[0], 4.0f);

   }
   else if(IsKeyDown(win,  '6', isShifted)) {
      if(isShifted)
         m_fWavelength[1] = (std::max)(0.001f, m_fWavelength[1] -= 0.001f);
      else
         m_fWavelength[1] += 0.001f;

      m_fWavelength4[1] = powf(m_fWavelength[1], 4.0f);

   }
   else if(IsKeyDown(win,  '7', isShifted)) {
      if(isShifted)
         m_fWavelength[2] = (std::max)(0.001f, m_fWavelength[2] -= 0.001f);
      else
         m_fWavelength[2] += 0.001f;

      m_fWavelength4[2] = powf(m_fWavelength[2], 4.0f);

   }
   else if(IsKeyDown(win,  '8', isShifted)) {
      auto exposure = sky_.GetExposure();
      if(isShifted)
         exposure = (std::max)(0.1f, exposure - 0.1f);
      else
         exposure += 0.1f;
      sky_.SetExposure(exposure);
   }
}

