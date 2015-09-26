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

#include <math.h>
#include <float.h>
#include <stdlib.h>

#include <algorithm>

#define MAX_DIMENSIONS		4					// Maximum number of dimensions in a noise object
#define MAX_OCTAVES			128					// Maximum # of octaves in an fBm object


/*******************************************************************************
* Class: CRandom
********************************************************************************
* This class wraps a random number generator. I plan to implement my own random
* number generator so I can keep the seeds as member variables (which is more
* flexible than using statics or globals). I was using one I found on the
* Internet implemented in assembler, but I was having problems with it so I
* removed it for this demo.
*******************************************************************************/
class CRandom
{
public:
	CRandom()						{}
	CRandom(unsigned int nSeed)		{ Init(nSeed); }
	void Init(unsigned int nSeed)	{ srand(nSeed); }
	double Random()					{ return (double)rand()/(double)RAND_MAX; }
	double RandomD(double dMin, double dMax)
	{
		double dInterval = dMax - dMin;
		double d = dInterval * Random();
		return dMin + (std::min)(d, dInterval);
	}
	unsigned int RandomI(unsigned int nMin, unsigned int nMax)
	{
		unsigned int nInterval = nMax - nMin;
		unsigned int i = (unsigned int)((nInterval+1.0) * Random());
		return nMin + (std::min)(i, nInterval);
	}
};

/*******************************************************************************
* Class: CNoise
********************************************************************************
* This class implements the Perlin noise function. Initialize it with the number
* of dimensions (1 to 4) and a random seed. I got the source for the first 3
* dimensions from "Texturing & Modeling: A Procedural Approach". I added the
* extra dimension because it may be desirable to use 3 spatial dimensions and
* one time dimension. The noise buffers are set up as member variables so that
* there may be several instances of this class in use at the same time, each
* initialized with different parameters.
*******************************************************************************/
class CNoise
{
protected:
	int m_nDimensions;						// Number of dimensions used by this object
	unsigned char m_nMap[256];				// Randomized map of indexes into buffer
	float m_nBuffer[256][MAX_DIMENSIONS];	// Random n-dimensional buffer

	float Lattice(int ix, float fx, int iy=0, float fy=0, int iz=0, float fz=0, int iw=0, float fw=0)
	{
		int n[4] = {ix, iy, iz, iw};
		float f[4] = {fx, fy, fz, fw};
		int nIndex = 0;
		for(auto i=0; i<m_nDimensions; i++)
			nIndex = m_nMap[(nIndex + n[i]) & 0xFF];
		float fValue = 0;
		for(auto i=0; i<m_nDimensions; i++)
			fValue += m_nBuffer[nIndex][i] * f[i];
		return fValue;
	}

public:
	CNoise()	{}
	CNoise(int nDimensions, unsigned int nSeed)	{ Init(nDimensions, nSeed); }
	void Init(int nDimensions, unsigned int nSeed);
	float Noise(float *f);
};

/*******************************************************************************
* Class: CSeededNoise
********************************************************************************
*******************************************************************************/
class CSeededNoise
{
protected:
	float m_nBuffer[64][64];

	float Lattice(int ix, float fx, int iy=0, float fy=0, int iz=0, float fz=0)
	{
		float fValue = m_nBuffer[ix][iy];
		return fValue;
	}

public:
	CSeededNoise()	{}
	CSeededNoise(unsigned int nSeed)	{ Init(nSeed); }
	void Init(unsigned int nSeed);
	float Noise(float *f);
};

/*******************************************************************************
* Class: CFractal
********************************************************************************
* This class implements fBm, or fractal Brownian motion. Since fBm uses Perlin
* noise, this class is derived from CNoise. Initialize it with the number of
* dimensions (1 to 4), a random seed, H (roughness ranging from 0 to 1), and
* the lacunarity (2.0 is often used). Many of the fractal routines came from
* "Texturing & Modeling: A Procedural Approach". fBmTest() is my own creation,
* and I created it to generate my first planet.
*******************************************************************************/
class CFractal : public CNoise
{
protected:
	float m_fH;
	float m_fLacunarity;
	float m_fExponent[MAX_OCTAVES];

public:
	CFractal()	{}
	CFractal(int nDimensions, unsigned int nSeed, float fH, float fLacunarity)
	{
		Init(nDimensions, nSeed, fH, fLacunarity);
	}
	void Init(int nDimensions, unsigned int nSeed, float fH, float fLacunarity)
	{
		CNoise::Init(nDimensions, nSeed);
		m_fH = fH;
		m_fLacunarity = fLacunarity;
		float f = 1;
		for(int i=0; i<MAX_OCTAVES; i++) 
		{
			m_fExponent[i] = powf(f, -m_fH);
			f *= m_fLacunarity;
		}
	}
	float fBm(float *f, float fOctaves);
	float Turbulence(float *f, float fOctaves);
	float Multifractal(float *f, float fOctaves, float fOffset);
	float Heterofractal(float *f, float fOctaves, float fOffset);
	float HybridMultifractal(float *f, float fOctaves, float fOffset, float fGain);
	float RidgedMultifractal(float *f, float fOctaves, float fOffset, float fThreshold);
	float fBmTest(float *f, int nStart, int nEnd, float fInitial=0.0f);
	float fBmTest(float *f, float fOctaves);
};

