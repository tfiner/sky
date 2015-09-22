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
#include "GameApp.h"
#include "GameEngine.h"

#include <sstream>

#include <GLFW/glfw3.h>

CWinApp *CWinApp::m_pMainApp;
CLog *CLog::m_pSingleton = NULL;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char *pszCmdLine, int nShowCmd)
{
	CLog log;
	log.Init(Debug, "AtmosphereTest", 0);
	CGameApp app(hInstance, hPrevInstance, pszCmdLine, nShowCmd);
   app.InitInstance();
}

bool CGameApp::InitInstance() {
	return InitMode(false, 800, 600);
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

   GetGameApp()->GetGameEngine()->OnChar(key);
}


bool CGameApp::InitMode(bool bFullScreen, int nWidth, int nHeight)
{
	m_nWidth = nWidth;
	m_nHeight = nHeight;

   if(!glfwInit()) {
      MessageBox("Unable to init GLFW, aborting.");
      return false;
   }

   window_ = glfwCreateWindow(GetWidth(), GetHeight(), "Atmosphere Test", NULL, NULL);
   if(!window_)
   {
      MessageBox("Unable to create application window, aborting.");
      glfwTerminate();
      return false;
   }

   glfwSetWindowSizeCallback(window_, OnResize);
   glfwSetKeyCallback(window_, OnKey);

   OnCreate();

   m_bActive = true;
   while(!glfwWindowShouldClose(window_)) {
      OnIdle();
      glfwPollEvents();
   }

   OnDestroy();
	return true;
}

int CGameApp::ExitInstance()
{
	UnregisterClass(m_szAppName, m_hInstance);
	return 0;
}

bool CGameApp::OnIdle()
{
	if(!m_bActive)
		return false;
	int nTimer = timeGetTime();
	m_pGameEngine->RenderFrame(nTimer-m_nTimer);
   glfwSwapBuffers(window_);
	m_nTimer = nTimer;
	Sleep(0);
	return true;
}

void CGameApp::Pause()
{
	if(m_bActive)
	{
#ifndef _DEBUG
		//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
#endif
		m_pGameEngine->Pause();
		m_bActive = false;
	}
}

void CGameApp::Restore()
{
	if(!m_bActive)
	{
#ifndef _DEBUG
		//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif
		m_bActive = true;
		m_nTimer = timeGetTime();
		m_pGameEngine->Restore();
	}
}

int CGameApp::OnCreate() 
{
   glfwMakeContextCurrent(window_);
   glfwSwapInterval(1);

   auto glewStatus = ::glewInit();
   if(GLEW_OK != glewStatus) {
      std::ostringstream ss;
      ss << "Glew error: " << ::glewGetErrorString(glewStatus) << "\n";
      throw std::runtime_error(ss.str());
   }

	m_pGameEngine = new CGameEngine;
	return 0;
}

void CGameApp::OnDestroy()
{
	if(m_pGameEngine)
	{
		delete m_pGameEngine;
		m_pGameEngine = NULL;
	}

   glfwDestroyWindow(window_);
   glfwTerminate();
}

void CGameApp::OnSize(int nType, int nWidth, int nHeight)
{
	if(!nHeight || !nWidth)
		return;

	glViewport(0, 0, nWidth, nHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)nWidth / (double)nHeight, 0.001, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/*
LRESULT CALLBACK CGameApp::WindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	CRect rect;
	switch(nMsg)
	{
		case WM_CREATE:
			return GetGameApp()->OnCreate(hWnd);
		case WM_DESTROY:
			GetGameApp()->OnDestroy();
			break;
		case WM_CLOSE:
			GetGameApp()->wnd_.DestroyWindow();
			PostQuitMessage(0);
			return 0;
		case WM_SIZE:
			GetGameApp()->OnSize(wParam, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_ACTIVATE:
			if(wParam)
				GetGameApp()->Restore();
			else
				GetGameApp()->Pause();
			break;
		case WM_GETMINMAXINFO:
			rect = CRect(0, 0, 320, 240);
			CWnd(hWnd).CalcWindowRect(&rect);
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = rect.Width();
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = rect.Height();
			return 0;
		case WM_SYSCOMMAND:
			// Prevent system commands (like closing, moving, sizing, screensaver, power management, etc) when active
			//if(GetGameApp()->m_bActive)
			//	return 0;
			break;
		case WM_POWERBROADCAST:
			// Prevent power suspend when active
			if(GetGameApp()->m_bActive && wParam == PBT_APMQUERYSUSPEND)
				return BROADCAST_QUERY_DENY;
			break;
		case WM_CHAR:
			GetGameApp()->m_pGameEngine->OnChar(wParam);
			break;
	}
	return DefWindowProc(hWnd, nMsg, wParam, lParam);
}
*/


void CGameApp::MakeCurrent(){ 
   glfwMakeContextCurrent(window_); 
}
