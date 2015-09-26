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
#include "PixelBuffer.h"
#include "Noise.h"

void PixelBuffer::MakeOpticalDepthBuffer(float fInnerRadius, float fOuterRadius, float fRayleighScaleHeight, float fMieScaleHeight)
{
	const int nSize = 64;
	const int nSamples = 50;
	const float fScale = 1.0f / (fOuterRadius - fInnerRadius);
	//std::ofstream ofScale1("scale1.txt");
	//std::ofstream ofScale2("scale2.txt");

	Init(nSize, nSize, 1, 4, GL_RGBA, FloatType);
	int nIndex = 0;
	float fPrev = 0;
	for(int nAngle=0; nAngle<nSize; nAngle++)
	{
		// As the y tex coord goes from 0 to 1, the angle goes from 0 to 180 degrees
		float fCos = 1.0f - (nAngle+nAngle) / (float)nSize;
		float fAngle = acosf(fCos);
		CVector vRay(sinf(fAngle), cosf(fAngle), 0);	// Ray pointing to the viewpoint

		/*char szName[256];
		sprintf(szName, "graph%-2.2d.txt", nAngle);
		std::ofstream ofGraph;
		if(fCos >= 0.0f)
			ofGraph.open(szName);
		ofGraph << "# fCos = " << fCos << std::endl;*/

		float fFirst = 0;
		for(int nHeight=0; nHeight<nSize; nHeight++)
		{
			// As the x tex coord goes from 0 to 1, the height goes from the bottom of the atmosphere to the top
			float fHeight = DELTA + fInnerRadius + ((fOuterRadius - fInnerRadius) * nHeight) / nSize;
			CVector vPos(0, fHeight, 0);				// The position of the camera

			// If the ray from vPos heading in the vRay direction intersects the inner radius (i.e. the planet), then this spot is not visible from the viewpoint
			float B = 2.0f * (vPos | vRay);
			float Bsq = B * B;
			float Cpart = (vPos | vPos);
			float C = Cpart - fInnerRadius*fInnerRadius;
			float fDet = Bsq - 4.0f * C;
			bool bVisible = (fDet < 0 || (0.5f * (-B - sqrtf(fDet)) <= 0) && (0.5f * (-B + sqrtf(fDet)) <= 0));
			float fRayleighDensityRatio;
			float fMieDensityRatio;
			if(bVisible)
			{
				fRayleighDensityRatio = expf(-(fHeight - fInnerRadius) * fScale / fRayleighScaleHeight);
				fMieDensityRatio = expf(-(fHeight - fInnerRadius) * fScale / fMieScaleHeight);
			}
			else
			{
				// Smooth the transition from light to shadow (it is a soft shadow after all)
				fRayleighDensityRatio = ((float *)m_pBuffer)[nIndex - nSize*m_nChannels] * 0.5f;
				fMieDensityRatio = ((float *)m_pBuffer)[nIndex+2 - nSize*m_nChannels] * 0.5f;
			}

			// Determine where the ray intersects the outer radius (the top of the atmosphere)
			// This is the end of our ray for determining the optical depth (vPos is the start)
			C = Cpart - fOuterRadius*fOuterRadius;
			fDet = Bsq - 4.0f * C;
			float fFar = 0.5f * (-B + sqrtf(fDet));

			// Next determine the length of each sample, scale the sample ray, and make sure position checks are at the center of a sample ray
			float fSampleLength = fFar / nSamples;
			float fScaledLength = fSampleLength * fScale;
			CVector vSampleRay = vRay * fSampleLength;
			vPos += vSampleRay * 0.5f;

			// Iterate through the samples to sum up the optical depth for the distance the ray travels through the atmosphere
			float fRayleighDepth = 0;
			float fMieDepth = 0;
			for(int i=0; i<nSamples; i++)
			{
				float fHeight = vPos.Magnitude();
				float fAltitude = (fHeight - fInnerRadius) * fScale;
				//fAltitude = Max(fAltitude, 0.0f);
				fRayleighDepth += expf(-fAltitude / fRayleighScaleHeight);
				fMieDepth += expf(-fAltitude / fMieScaleHeight);
				vPos += vSampleRay;
			}

			// Multiply the sums by the length the ray traveled
			fRayleighDepth *= fScaledLength;
			fMieDepth *= fScaledLength;

			if(!_finite(fRayleighDepth) || fRayleighDepth > 1.0e25f)
				fRayleighDepth = 0;
			if(!_finite(fMieDepth) || fMieDepth > 1.0e25f)
				fMieDepth = 0;

			// Store the results for Rayleigh to the light source, Rayleigh to the camera, Mie to the light source, and Mie to the camera
			((float *)m_pBuffer)[nIndex++] = fRayleighDensityRatio;
			((float *)m_pBuffer)[nIndex++] = fRayleighDepth;
			((float *)m_pBuffer)[nIndex++] = fMieDensityRatio;
			((float *)m_pBuffer)[nIndex++] = fMieDepth;

			/*
			if(nHeight == 0)
			{
				fFirst = fRayleighDepth;
				if(fCos >= 0.0f)
				{
					ofScale1 << 1-fCos << "\t" << logf(fRayleighDepth / fRayleighScaleHeight) << std::endl;
					ofScale2 << 1-fCos << "\t" << logf(fMieDepth) << std::endl;
					fPrev = fRayleighDepth;
				}
			}
			float x = (fHeight-fInnerRadius) / (fOuterRadius-fInnerRadius);
			float y = fRayleighDepth / fFirst;
			ofGraph << x << "\t" << y << std::endl;
			*/
		}
		//ofGraph << std::endl;
	}
}

