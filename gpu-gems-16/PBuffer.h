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


#include <memory>

// Avoid windows.h
struct HDC__;
using HDC = struct HDC__*;

struct HGLRC__;
using HGLRC = struct HGLRC__*;

struct HPBUFFERARB__;
using HPBUFFERARB = struct HPBUFFERARB__*;



class PBuffer {
public:
	enum {NoFlags = 0x00, DepthBuffer = 0x01, StencilBuffer = 0x02};

	PBuffer()							{ m_hBuffer = NULL; }
	PBuffer(int nWidth, int nHeight, int nFlags=(DepthBuffer|StencilBuffer))
	{
		m_hBuffer = NULL;
		Init(nWidth, nHeight);
	}
   ~PBuffer();

	bool Init(int nWidth, int nHeight, int nFlags=(DepthBuffer|StencilBuffer));
	void Cleanup();
	void HandleModeSwitch();

	int GetWidth()						{ return m_nWidth; }
	int GetHeight()						{ return m_nHeight; }
	int GetFlags()						{ return m_nFlags; }

   void MakeCurrent();
   void BindTexture(float fExposure, bool bUseExposure = true);
   void ReleaseTexture();


private:
   int m_nWidth;
   int m_nHeight;
   int m_nFlags;

   HDC m_hDC;
   HGLRC m_hGLRC;
   HPBUFFERARB m_hBuffer;
   unsigned int m_nTextureID;

   bool m_bATI;
   unsigned int m_nTarget;
};


