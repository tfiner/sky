// Copied right from GLUtil, I'm going to replace this with my own (simpler) shader class.

#pragma once

#include "Log.h"
#include "tfgl/Exception.h"

#include <GL/glew.h>

#include <fstream>
#include <map>
#include <string>

class CShaderObject {
public:
   CShaderObject()
   {
      m_hProgram = glCreateProgramObjectARB();
      m_hVertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
      m_hFragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
      THROW_ON_GL_ERROR();
   }
   ~CShaderObject()
   {
      glDeleteObjectARB(m_hFragmentShader);
      glDeleteObjectARB(m_hVertexShader);
      glDeleteObjectARB(m_hProgram);
   }

   bool Load(const char *pszPath, const char *pszPath2 = NULL)
   {
      char szPath[_MAX_PATH], *psz;
      int bSuccess;

      sprintf(szPath, "%s.vert", pszPath);
      LogInfo("Compiling GLSL shader %s", szPath);
      std::ifstream ifVertexShader(szPath, std::ios::binary);
      ifVertexShader.seekg(0, std::ios::end);
      auto nBytes = static_cast<int>(ifVertexShader.tellg());
      ifVertexShader.seekg(0, std::ios::beg);
      psz = new char[nBytes + 1];
      ifVertexShader.read(psz, nBytes);
      psz[nBytes] = 0;
      ifVertexShader.close();
      glShaderSourceARB(m_hVertexShader, 1, (const char **)&psz, &nBytes);
      glCompileShaderARB(m_hVertexShader);
      glGetObjectParameterivARB(m_hVertexShader, GL_OBJECT_COMPILE_STATUS_ARB, &bSuccess);
      delete psz;
      if(!bSuccess)
      {
         LogError("Failed to compile vertex shader %s", szPath);
         LogGLErrors();
         LogGLInfoLog(m_hVertexShader);
         return false;
      }

      sprintf(szPath, "%s.frag", pszPath2 ? pszPath2 : pszPath);
      LogInfo("Compiling GLSL shader %s", szPath);
      std::ifstream ifFragmentShader(szPath, std::ios::binary);
      ifFragmentShader.seekg(0, std::ios::end);
      nBytes = static_cast<int>(ifFragmentShader.tellg());
      ifFragmentShader.seekg(0, std::ios::beg);
      psz = new char[nBytes];
      ifFragmentShader.read(psz, nBytes);
      ifFragmentShader.close();
      glShaderSourceARB(m_hFragmentShader, 1, (const char **)&psz, &nBytes);
      glCompileShaderARB(m_hFragmentShader);
      glGetObjectParameterivARB(m_hFragmentShader, GL_OBJECT_COMPILE_STATUS_ARB, &bSuccess);
      delete psz;
      if(!bSuccess)
      {
         LogError("Failed to compile fragment shader %s", szPath);
         LogGLErrors();
         LogGLInfoLog(m_hFragmentShader);
         return false;
      }

      glAttachObjectARB(m_hProgram, m_hVertexShader);
      glAttachObjectARB(m_hProgram, m_hFragmentShader);
      glLinkProgramARB(m_hProgram);

      glGetObjectParameterivARB(m_hProgram, GL_OBJECT_LINK_STATUS_ARB, &bSuccess);
      if(!bSuccess)
      {
         LogError("Failed to link shader %s", szPath);
         LogGLErrors();
         LogGLInfoLog(m_hProgram);
         return false;
      }

      THROW_ON_GL_ERROR();
      LogGLInfoLog(m_hProgram);
      return true;
   }

   void Enable()
   {
      glUseProgramObjectARB(m_hProgram);
   }
   void Disable()
   {
      glUseProgramObjectARB(NULL);
   }

   GLint GetUniformParameterID(const char *pszParameter)
   {
      auto it = m_mapParameters.find(pszParameter);
      if(it == m_mapParameters.end()) {
         GLint nLoc = glGetUniformLocationARB(m_hProgram, pszParameter);
         it = m_mapParameters.insert(std::pair<std::string, GLint>(pszParameter, nLoc)).first;
      }
      return it->second;
   }

   /*
   void BindTexture(const char *pszParameter, unsigned int nID)
   {
   GLint n = GetUniformParameterID(pszParameter);
   glBindTexture(GL_TEXTURE_2D, nID);
   glUniform1iARB(n, nID);
   }
   */
   void SetUniformParameter1i(const char *pszParameter, int n1)
   {
      glUniform1iARB(GetUniformParameterID(pszParameter), n1);
   }
   void SetUniformParameter1f(const char *pszParameter, float p1)
   {
      glUniform1fARB(GetUniformParameterID(pszParameter), p1);
   }
   void SetUniformParameter3f(const char *pszParameter, float p1, float p2, float p3)
   {
      glUniform3fARB(GetUniformParameterID(pszParameter), p1, p2, p3);
   }

private:
   GLuint m_hProgram;
   GLuint m_hVertexShader;
   GLuint m_hFragmentShader;
   std::map<std::string, GLuint> m_mapParameters;

   void LogGLErrors()
   {
      GLenum glErr;
      while((glErr = glGetError()) != GL_NO_ERROR)
         LogError((const char *)gluErrorString(glErr));
   }
   void LogGLInfoLog(GLhandleARB hObj)
   {
      int nBytes;
      glGetObjectParameterivARB(hObj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &nBytes);
      if(nBytes)
      {
         char *pInfo = new char[nBytes];
         glGetInfoLogARB(hObj, nBytes, &nBytes, pInfo);
         LogInfo(pInfo);
         delete pInfo;
      }
   }
};

