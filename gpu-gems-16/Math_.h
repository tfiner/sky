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

#include <cmath>

// Defines
#define PI					3.14159f			// PI
#define HALF_PI				1.57080f			// PI / 2
#define TWO_PI				6.28318f			// PI * 2
#define INV_PI				0.318310f			// 1 / PI
#define INV_TWO_PI			0.159155f			// 1 / (PI*2)
#define INV_HALF_PI			0.636618f			// 1 / (PI/2)

#define LOGHALF				-0.693147f			// log(0.5)
#define LOGHALFI			-1.442695f			// Inverse of log(0.5)
#define DELTA				1e-6f				// Small number for comparing floating point numbers

#define HALF_RAND			(RAND_MAX/2)

// Macros
#define SQUARE(a)			((a) * (a))
#define FLOOR(a)			((int)(a) - ((a) < 0 && (a) != (int)(a)))
#define CEILING(a)			((int)(a) + ((a) > 0 && (a) != (int)(a)))
#define MIN(a, b)			((a) < (b) ? (a) : (b))
#define MAX(a, b)			((a) > (b) ? (a) : (b))
#define ABS(a)				((a) < 0 ? -(a) : (a))
#define CLAMP(a, b, x)		((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))
#define LERP(a, b, x)		((a) + (x) * ((b) - (a)))
#define CUBIC(a)			((a) * (a) * (3 - 2*(a)))
#define STEP(a, x)			((x) >= (a))
#define BOXSTEP(a, b, x)	Clamp(0, 1, ((x)-(a))/((b)-(a)))
#define PULSE(a, b, x)		(((x) >= (a)) - ((x) >= (b)))
#define GAMMA(a, g)			powf(a, 1/g)
#define BIAS(a, b)			powf(a, logf(b) * LOGHALFI)
#define EXPOSE(l, k)		(1 - expf(l * k))
#define DEGTORAD(x)			((x) * 0.01745329251994f)
#define RADTODEG(x)			((x) * 57.29577951308f)
#define SWAP(a, b, t)		{ t = a; a = b; b = t; }

// Inline functions (use instead of macros to avoid performing slow operations twice)
template <class T> T Min(T a, T b)				{ return (a < b ? a : b); }
template <class T> T Max(T a, T b)				{ return (a > b ? a : b); }
inline float Square(float a)					{ return a * a; }
inline int Floor(float a)						{ return ((int)a - (a < 0 && a != (int)a)); }
inline int Ceiling(float a)						{ return ((int)a + (a > 0 && a != (int)a)); }
inline float Abs(float a)						{ return (a < 0 ? -a : a); }
inline float Clamp(float a, float b, float x)	{ return (x < a ? a : (x > b ? b : x)); }
inline float Lerp(float a, float b, float x)	{ return a + x * (b - a); }
inline float Cubic(float a)						{ return a * a * (3 - 2 * a); }
inline float Step(float a, float x)				{ return (float)(x >= a); }
inline float Boxstep(float a, float b, float x)	{ return Clamp(0, 1, (x - a) / (b - a)); }
inline float Pulse(float a, float b, float x)	{ return (float)((x >= a) - (x >= b)); }

