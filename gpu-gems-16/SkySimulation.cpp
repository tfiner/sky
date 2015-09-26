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

#include <GLFW/glfw3.h>

namespace {
   // Camera rotation:
   const auto RadiansPerSecond = 1.0f;

   // Camera acceleration rate due to thrusters (units/s*s):
   const auto Thrust = 1.0f;

   // Camera damping effect on velocity:
   const auto Resistance = 0.1f;
}


SkySimulation::SkySimulation() {
	m_bUseHDR = false;
	GLUtil()->Init();

	m_nPolygonMode = GL_FILL;

	m_pBuffer.Init(1024, 1024, 0);
	m_pBuffer.MakeCurrent();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);


	CVector vPos(0, 0, 25);
	m_3DCamera.SetPosition(CDoubleVector(6.946963, 6.913678, 2.205330));
	CQuaternion qOrientation(0.404703f, 0.910458f, 0.040314f, 0.075190f);
	qOrientation.Normalize();
	m_3DCamera = qOrientation;

	m_vLight = CVector(0, 0, 1000);
	m_vLightDirection = m_vLight / m_vLight.Magnitude();
	CTexture::InitStaticMembers(238653, 256);

	m_nSamples = 3;		// Number of sample rays to use in integral equation
	m_Kr = 0.0025f;		// Rayleigh scattering constant
	m_Kr4PI = m_Kr*4.0f*PI;
	m_Km = 0.0010f;		// Mie scattering constant
	m_Km4PI = m_Km*4.0f*PI;
	m_ESun = 20.0f;		// Sun brightness constant
	m_g = -0.990f;		// The Mie phase asymmetry factor
	m_fExposure = 2.0f;

	m_fInnerRadius = 10.0f;
	m_fOuterRadius = 10.25f;
	m_fScale = 1 / (m_fOuterRadius - m_fInnerRadius);

	m_fWavelength[0] = 0.650f;		// 650 nm for red
	m_fWavelength[1] = 0.570f;		// 570 nm for green
	m_fWavelength[2] = 0.475f;		// 475 nm for blue
	m_fWavelength4[0] = powf(m_fWavelength[0], 4.0f);
	m_fWavelength4[1] = powf(m_fWavelength[1], 4.0f);
	m_fWavelength4[2] = powf(m_fWavelength[2], 4.0f);

	m_fRayleighScaleDepth = 0.25f;
	m_fMieScaleDepth = 0.1f;
	m_pbOpticalDepth.MakeOpticalDepthBuffer(m_fInnerRadius, m_fOuterRadius, m_fRayleighScaleDepth, m_fMieScaleDepth);
}


void SkySimulation::RenderFrame(unsigned milliseconds) {
   InitShaders();

	// Move the camera
	HandleInput(milliseconds * 0.001f);

   SetContext();

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	glLoadMatrixf(m_3DCamera.GetViewMatrix());

	C3DObject obj;
	glMultMatrixf(obj.GetModelMatrix(&m_3DCamera));

	CVector vCamera = m_3DCamera.GetPosition();
	CVector vUnitCamera = vCamera / vCamera.Magnitude();

   RenderGound(vCamera);
   RenderSky(vCamera);

	glPopMatrix();
	glFlush();

   RenderHDR();

   // DrawInfo(nMilliseconds);
}


void SkySimulation::SetContext() {
   if(m_bUseHDR)	{
      m_pBuffer.MakeCurrent();
      glViewport(0, 0, 1024, 1024);
   }
   else
      GLUtil()->MakeCurrent();
}


// Lazily initialize shaders.
void SkySimulation::InitShaders() {
   if(!m_shSkyFromAtmosphere) {
      m_shSkyFromAtmosphere = std::make_unique<CShaderObject>();
      m_shGroundFromAtmosphere = std::make_unique<CShaderObject>();
      m_shSpaceFromAtmosphere = std::make_unique<CShaderObject>();

      m_shSkyFromAtmosphere->Load("SkyFromAtmosphere");
      m_shGroundFromAtmosphere->Load("GroundFromAtmosphere");
      m_shSpaceFromAtmosphere->Load("SpaceFromAtmosphere");
   }
}

void SkySimulation::RenderHDR() {
   if(!m_bUseHDR)
      return;

   GLUtil()->MakeCurrent();
   glViewport(0, 0, GetOpenGLApp()->GetWidth(), GetOpenGLApp()->GetHeight());
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glDisable(GL_LIGHTING);
   glPushMatrix();
   glLoadIdentity();
   glMatrixMode(GL_PROJECTION);
   glPushMatrix();
   glLoadIdentity();
   glOrtho(0, 1, 0, 1, -1, 1);

   m_pBuffer.BindTexture(m_fExposure, m_bUseHDR);
   glBegin(GL_QUADS);
   glTexCoord2f(0, 0); glVertex2f(0, 0);	// For rect texture, can't use 1 as the max texture coord
   glTexCoord2f(1, 0); glVertex2f(1, 0);
   glTexCoord2f(1, 1); glVertex2f(1, 1);
   glTexCoord2f(0, 1); glVertex2f(0, 1);
   glEnd();
   m_pBuffer.ReleaseTexture();

   glPopMatrix();
   glMatrixMode(GL_MODELVIEW);
   glPopMatrix();
   glEnable(GL_LIGHTING);
}


void SkySimulation::RenderSky(CVector &vCamera) {
   auto pSkyShader = m_shSkyFromAtmosphere.get();

   pSkyShader->Enable();
   pSkyShader->SetUniformParameter3f("v3CameraPos", vCamera.x, vCamera.y, vCamera.z);
   pSkyShader->SetUniformParameter3f("v3LightPos", m_vLightDirection.x, m_vLightDirection.y, m_vLightDirection.z);
   pSkyShader->SetUniformParameter3f("v3InvWavelength", 1 / m_fWavelength4[0], 1 / m_fWavelength4[1], 1 / m_fWavelength4[2]);
   pSkyShader->SetUniformParameter1f("fCameraHeight", vCamera.Magnitude());
   pSkyShader->SetUniformParameter1f("fCameraHeight2", vCamera.MagnitudeSquared());
   pSkyShader->SetUniformParameter1f("fInnerRadius", m_fInnerRadius);
   pSkyShader->SetUniformParameter1f("fInnerRadius2", m_fInnerRadius*m_fInnerRadius);
   pSkyShader->SetUniformParameter1f("fOuterRadius", m_fOuterRadius);
   pSkyShader->SetUniformParameter1f("fOuterRadius2", m_fOuterRadius*m_fOuterRadius);
   pSkyShader->SetUniformParameter1f("fKrESun", m_Kr*m_ESun);
   pSkyShader->SetUniformParameter1f("fKmESun", m_Km*m_ESun);
   pSkyShader->SetUniformParameter1f("fKr4PI", m_Kr4PI);
   pSkyShader->SetUniformParameter1f("fKm4PI", m_Km4PI);
   pSkyShader->SetUniformParameter1f("fScale", 1.0f / (m_fOuterRadius - m_fInnerRadius));
   pSkyShader->SetUniformParameter1f("fScaleDepth", m_fRayleighScaleDepth);
   pSkyShader->SetUniformParameter1f("fScaleOverScaleDepth", (1.0f / (m_fOuterRadius - m_fInnerRadius)) / m_fRayleighScaleDepth);
   pSkyShader->SetUniformParameter1f("g", m_g);
   pSkyShader->SetUniformParameter1f("g2", m_g*m_g);

   glFrontFace(GL_CW);
   //glEnable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ONE);

   auto pSphere = gluNewQuadric();
   gluSphere(pSphere, m_fOuterRadius, 100, 50);
   gluDeleteQuadric(pSphere);

   //glDisable(GL_BLEND);
   glFrontFace(GL_CCW);
   pSkyShader->Disable();
}


void SkySimulation::RenderGound(CVector &vCamera) {
   auto pGroundShader = m_shGroundFromAtmosphere.get();

   pGroundShader->Enable();
   pGroundShader->SetUniformParameter3f("v3CameraPos", vCamera.x, vCamera.y, vCamera.z);
   pGroundShader->SetUniformParameter3f("v3LightPos", m_vLightDirection.x, m_vLightDirection.y, m_vLightDirection.z);
   pGroundShader->SetUniformParameter3f("v3InvWavelength", 1 / m_fWavelength4[0], 1 / m_fWavelength4[1], 1 / m_fWavelength4[2]);
   pGroundShader->SetUniformParameter1f("fCameraHeight", vCamera.Magnitude());
   pGroundShader->SetUniformParameter1f("fCameraHeight2", vCamera.MagnitudeSquared());
   pGroundShader->SetUniformParameter1f("fInnerRadius", m_fInnerRadius);
   pGroundShader->SetUniformParameter1f("fInnerRadius2", m_fInnerRadius*m_fInnerRadius);
   pGroundShader->SetUniformParameter1f("fOuterRadius", m_fOuterRadius);
   pGroundShader->SetUniformParameter1f("fOuterRadius2", m_fOuterRadius*m_fOuterRadius);
   pGroundShader->SetUniformParameter1f("fKrESun", m_Kr*m_ESun);
   pGroundShader->SetUniformParameter1f("fKmESun", m_Km*m_ESun);
   pGroundShader->SetUniformParameter1f("fKr4PI", m_Kr4PI);
   pGroundShader->SetUniformParameter1f("fKm4PI", m_Km4PI);
   pGroundShader->SetUniformParameter1f("fScale", 1.0f / (m_fOuterRadius - m_fInnerRadius));
   pGroundShader->SetUniformParameter1f("fScaleDepth", m_fRayleighScaleDepth);
   pGroundShader->SetUniformParameter1f("fScaleOverScaleDepth", (1.0f / (m_fOuterRadius - m_fInnerRadius)) / m_fRayleighScaleDepth);
   pGroundShader->SetUniformParameter1f("g", m_g);
   pGroundShader->SetUniformParameter1f("g2", m_g*m_g);
   pGroundShader->SetUniformParameter1i("s2Test", 0);

   GLUquadricObj *pSphere = gluNewQuadric();
   gluSphere(pSphere, m_fInnerRadius, 100, 50);
   gluDeleteQuadric(pSphere);
   pGroundShader->Disable();
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
		case '+':
			m_nSamples++;
			break;
		case '-':
			m_nSamples--;
			break;
	}
}


bool IsKeyDown(const char c, bool& shifted) {
   if(!glfwGetKey(GetOpenGLApp()->GetWindow(), c))
      return false;

   shifted = glfwGetKey(GetOpenGLApp()->GetWindow(), GLFW_KEY_RIGHT_SHIFT) ||
             glfwGetKey(GetOpenGLApp()->GetWindow(), GLFW_KEY_LEFT_SHIFT);

   return true;
}


void SkySimulation::HandleInput(float fSeconds) {
   InputRenderParams();
   InputCameraOrient(fSeconds);
   InputCameraPosition(fSeconds);
}


void SkySimulation::InputCameraPosition(float fSeconds) {
   const auto win = GetOpenGLApp()->GetWindow();
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

      m_3DCamera.Accelerate(vAccel, fSeconds, Resistance);
      CVector vPos = m_3DCamera.GetPosition();
      float fMagnitude = vPos.Magnitude();
      if(fMagnitude < m_fInnerRadius) {
         vPos *= (m_fInnerRadius * (1 + DELTA)) / fMagnitude;
         m_3DCamera.SetPosition(CDoubleVector(vPos.x, vPos.y, vPos.z));
         m_3DCamera.SetVelocity(-m_3DCamera.GetVelocity());
      }
   }
}

void SkySimulation::InputCameraOrient(float fSeconds) {
   const auto win = GetOpenGLApp()->GetWindow();
   if(glfwGetKey(win, GLFW_KEY_RIGHT) || glfwGetKey(win, GLFW_KEY_KP_6))
      m_3DCamera.Rotate(m_3DCamera.GetUpAxis(), fSeconds * -RadiansPerSecond);

   if(glfwGetKey(win, GLFW_KEY_LEFT) || glfwGetKey(win, GLFW_KEY_KP_4))
      m_3DCamera.Rotate(m_3DCamera.GetUpAxis(), fSeconds * RadiansPerSecond);

   if(glfwGetKey(win, GLFW_KEY_UP) || glfwGetKey(win, GLFW_KEY_KP_8))
      m_3DCamera.Rotate(m_3DCamera.GetRightAxis(), fSeconds * -RadiansPerSecond);

   if(glfwGetKey(win, GLFW_KEY_DOWN) || glfwGetKey(win, GLFW_KEY_KP_2))
      m_3DCamera.Rotate(m_3DCamera.GetRightAxis(), fSeconds * RadiansPerSecond);

   if(glfwGetKey(win, GLFW_KEY_KP_7))
      m_3DCamera.Rotate(m_3DCamera.GetViewAxis(), fSeconds * -RadiansPerSecond);

   if(glfwGetKey(win, GLFW_KEY_KP_9))
      m_3DCamera.Rotate(m_3DCamera.GetViewAxis(), fSeconds * RadiansPerSecond);
}


void SkySimulation::InputRenderParams() {
   bool isShifted = false;
   if(const auto isOne = IsKeyDown('1', isShifted)){
      if(isShifted)
         m_Kr = Max(0.0f, m_Kr - 0.0001f);
      else
         m_Kr += 0.0001f;

      m_Kr4PI = m_Kr*4.0f*PI;

   }
   else if(const auto isTwo = IsKeyDown('2', isShifted))	{
      if(isShifted)
         m_Km = Max(0.0f, m_Km - 0.0001f);
      else
         m_Km += 0.0001f;

      m_Km4PI = m_Km*4.0f*PI;

   }
   else if(IsKeyDown('3', isShifted)) {
      if(isShifted)
         m_g = Max(-1.0f, m_g - 0.001f);
      else
         m_g = Min(1.0f, m_g + 0.001f);

   }
   else if(IsKeyDown('4', isShifted))	{
      if(isShifted)
         m_ESun = Max(0.0f, m_ESun - 0.1f);
      else
         m_ESun += 0.1f;

   }
   else if(IsKeyDown('5', isShifted)) {
      if(isShifted)
         m_fWavelength[0] = Max(0.001f, m_fWavelength[0] -= 0.001f);
      else
         m_fWavelength[0] += 0.001f;

      m_fWavelength4[0] = powf(m_fWavelength[0], 4.0f);

   }
   else if(IsKeyDown('6', isShifted)) {
      if(isShifted)
         m_fWavelength[1] = Max(0.001f, m_fWavelength[1] -= 0.001f);
      else
         m_fWavelength[1] += 0.001f;

      m_fWavelength4[1] = powf(m_fWavelength[1], 4.0f);

   }
   else if(IsKeyDown('7', isShifted)) {
      if(isShifted)
         m_fWavelength[2] = Max(0.001f, m_fWavelength[2] -= 0.001f);
      else
         m_fWavelength[2] += 0.001f;

      m_fWavelength4[2] = powf(m_fWavelength[2], 4.0f);

   }
   else if(IsKeyDown('8', isShifted)) {
      if(isShifted)
         m_fExposure = Max(0.1f, m_fExposure - 0.1f);
      else
         m_fExposure += 0.1f;
   }
}

