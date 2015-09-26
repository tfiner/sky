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

#include "Matrix.h"
#include "3DBuffer.h"

#include <cassert>


/*******************************************************************************
* Class: CPixelBuffer
********************************************************************************
* This class implements a general-purpose pixel buffer to be used for anything.
* It is often used by CTexture to set up OpenGL textures, so many of the
* parameters you use to initialize it look like the parameters you would pass
* to glTexImage1D or glTexImage2D. Some of the standard pixel buffer routines
* call fast MMX functions implemented in PixelBuffer.asm.
*******************************************************************************/
class PixelBuffer : public ThreeDBuffer
{
protected:
	int m_nFormat;				// The format of the pixel data (i.e. GL_LUMINANCE, GL_RGBA)

public:
	PixelBuffer() : ThreeDBuffer() {}
	PixelBuffer(int nWidth, int nHeight, int nDepth, int nChannels=3, int nFormat=GL_RGB, BufferDataType nDataType=UnsignedByteType) : 
      ThreeDBuffer(nWidth, nHeight, nDepth, nDataType, nChannels)	{
		m_nFormat = nFormat;
	}

	int GetFormat()				{ return m_nFormat; }

	void Init(int nWidth, int nHeight, int nDepth, int nChannels=3, int nFormat=GL_RGB, BufferDataType nDataType=UnsignedByteType, void *pBuffer=NULL)
	{
		ThreeDBuffer::Init(nWidth, nHeight, nDepth, nDataType, nChannels, pBuffer);
		m_nFormat = nFormat;
	}

	// Miscellaneous initalization routines
	void MakeOpticalDepthBuffer(float fInnerRadius, float fOuterRadius, float fRayleighScaleHeight, float fMieScaleHeight);
};

