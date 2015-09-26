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

#include <sstream>

#include <GLFW/glfw3.h>

CLog *CLog::m_pSingleton = NULL;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char *pszCmdLine, int nShowCmd) {
	CLog log;
	log.Init(Debug, "AtmosphereTest", 0);
	OpenGLApp app;
   app.Run();
}

void OpenGLApp::Run() {
	InitMode(false, 800, 600);
}

void OnResize(GLFWwindow* window, int width, int height) {
   if(!height || !width)
      return;

   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(45.0, (double)width / (double)height, 0.001, 100.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}


void OnKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
   if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, GL_TRUE);

   // Only process the up character.
   if (action == GLFW_RELEASE)
      if (auto app = reinterpret_cast<OpenGLApp*>(glfwGetWindowUserPointer(window)))
         app->GetSimulation()->OnChar(key);
}


bool OpenGLApp::InitMode(bool bFullScreen, int width, int height){
	m_nWidth = width;
	m_nHeight = height;

   if(!glfwInit())
      throw std::runtime_error("Unable to init GLFW, aborting.");

   window_ = glfwCreateWindow(GetWidth(), GetHeight(), "Atmosphere Test", NULL, NULL);
   if(!window_)
      throw std::runtime_error("Unable to create application window, aborting.");

   glfwSetWindowUserPointer(window_, this);

   glfwSetWindowSizeCallback(window_, OnResize);
   glfwSetKeyCallback(window_, OnKey);

   OnCreate();

   OnResize(window_, width, height);

   m_bActive = true;
   while(!glfwWindowShouldClose(window_)) {
      OnIdle();
      glfwPollEvents();
   }

   OnDestroy();
	return true;
}


bool OpenGLApp::OnIdle() {
	if(!m_bActive)
		return false;
	const auto milliseconds = timeGetTime();
	simulation_->RenderFrame(window_, milliseconds-lastRender, m_nWidth, m_nHeight);
   glfwSwapBuffers(window_);
	lastRender = milliseconds;
	Sleep(0);
	return true;
}

void OpenGLApp::Pause()
{
	if(m_bActive)
	{
#ifndef _DEBUG
		//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
#endif
		simulation_->Pause();
		m_bActive = false;
	}
}

void OpenGLApp::Restore()
{
	if(!m_bActive)
	{
#ifndef _DEBUG
		//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif
		m_bActive = true;
		lastRender = timeGetTime();
		simulation_->Restore();
	}
}

int OpenGLApp::OnCreate() 
{
   glfwMakeContextCurrent(window_);
   glfwSwapInterval(1);

   auto glewStatus = ::glewInit();
   if(GLEW_OK != glewStatus) {
      std::ostringstream ss;
      ss << "Glew error: " << ::glewGetErrorString(glewStatus) << "\n";
      throw std::runtime_error(ss.str());
   }

	simulation_ = new SkySimulation;
	return 0;
}

void OpenGLApp::OnDestroy()
{
	if(simulation_)
	{
		delete simulation_;
		simulation_ = NULL;
	}

   glfwDestroyWindow(window_);
   glfwTerminate();
}

void OpenGLApp::OnSize(int nType, int nWidth, int nHeight) {
	if(!nHeight || !nWidth)
		return;

   m_nWidth = nWidth;
   m_nHeight = nHeight;
	glViewport(0, 0, nWidth, nHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)nWidth / (double)nHeight, 0.001, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void OpenGLApp::MakeCurrent(){ 
   glfwMakeContextCurrent(window_); 
}
