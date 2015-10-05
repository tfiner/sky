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

#pragma once

#include "GLUtil.h"
#include "PBuffer.h"
#include "OpenGLApp.h"
#include "Matrix.h"
#include "PixelBuffer.h"
#include "HdrSky.h"

#include <glm/glm.hpp>

#include <memory>

struct GLFWwindow;

namespace tfgl {
   class Program;
}

class SkySimulation {
public:
	SkySimulation();
   // For pimpl of Programs.
   ~SkySimulation();

   void RenderFrame(GLFWwindow* win, unsigned milliseconds, int width, int height);
   void SetContext();
   void InitShaders();
   void RenderHDR(int width, int height);
   void RenderSky(CVector &vCamera);
   void RenderGound(CVector &vCamera);
 
   void Pause()	{}
	void Restore()	{}

	void HandleInput(GLFWwindow* win, float fSeconds);
   void InputCameraPosition(GLFWwindow* win, float fSeconds);
   void InputCameraOrient(GLFWwindow* win, float fSeconds);
   void InputRenderParams(GLFWwindow* win);

   void OnChar(int c);

private:
   float m_fFPS;
   int m_nTime;

   C3DObject m_3DCamera;

   glm::vec3 light_;
   glm::vec3 lightDir_;
   
   // Variables that can be tweaked with keypresses
   bool m_bUseHDR;
   int m_nSamples;
   GLenum m_nPolygonMode;
   float m_Kr, m_Kr4PI;
   float m_Km, m_Km4PI;
   float m_ESun;
   float m_g;

   float m_fInnerRadius;
   float m_fOuterRadius;
   float m_fScale;
   float m_fWavelength[3];
   float m_fWavelength4[3];
   float m_fRayleighScaleDepth;
   float m_fMieScaleDepth;
   PixelBuffer m_pbOpticalDepth;

   std::unique_ptr<tfgl::Program> m_shSkyFromAtmosphere;
   std::unique_ptr<tfgl::Program> m_shGroundFromAtmosphere;
   std::unique_ptr<tfgl::Program> m_shSpaceFromAtmosphere;

   sky::HdrSky sky_;
};


