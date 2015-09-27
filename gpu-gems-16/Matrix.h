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

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <math.h>

#include <algorithm>

class CMatrix;
class CQuaternion;
class C3DObject;

// Small number for comparing floating point numbers
const auto DELTA = 1e-6f;

inline float DegToRad(float d)   {
   return d * 0.01745329251994f;
}


/*******************************************************************************
* Template Class: TVector
********************************************************************************
* This template class implements a simple 3D vector with x, y, and z coordinates.
* Several functions and operators are defined to make working with vectors easier,
* and because it's templatized, any numeric type can be used with it. Macros are
* defined for the most common types.
*******************************************************************************/


template <class T> class TVector {
public:
	T x, y, z;

	// Constructors
	TVector()									{}
	TVector(const T a, const T b, const T c)	{ x = a; y = b; z = c; }
	TVector(const T t)							{ *this = t; }
	TVector(const T *pt)						{ *this = pt; }
	TVector(const TVector<T> &v)				{ *this = v; }

	// Casting and unary operators
	operator TVector<float>()					{ return TVector<float>((float)x, (float)y, (float)z); }
	operator TVector<double>()					{ return TVector<double>((double)x, (double)y, (double)z); }
	operator T*()								{ return &x; }
	T &operator[](const int n)					{ return (&x)[n]; }
	operator const T*() const					{ return &x; }
	T operator[](const int n) const				{ return (&x)[n]; }
	TVector<T> operator-() const				{ return TVector<T>(-x, -y, -z); }

	// Equal and comparison operators
	void operator=(const T t)					{ x = y = z = t; }
	void operator=(const T *pt)					{ x = pt[0]; y = pt[1]; z = pt[2]; }
	void operator=(const TVector<T> &v)			{ x = v.x; y = v.y; z = v.z; }
	bool operator==(TVector<T> &v) const		{ return (Abs(x - v.x) <= (T)0.001f && Abs(y - v.y) <= 0.001f && Abs(z - v.z) <= 0.001f); }
	bool operator!=(TVector<T> &v) const		{ return !(*this == v); }

	// Arithmetic operators (vector with scalar)
	TVector<T> operator+(const T t) const		{ return TVector<T>(x+t, y+t, z+t); }
	TVector<T> operator-(const T t) const		{ return TVector<T>(x-t, y-t, z-t); }
	TVector<T> operator*(const T t) const		{ return TVector<T>(x*t, y*t, z*t); }
	TVector<T> operator/(const T t) const		{ return TVector<T>(x/t, y/t, z/t); }
	void operator+=(const T t)					{ x += t; y += t; z += t; }
	void operator-=(const T t)					{ x -= t; y -= t; z -= t; }
	void operator*=(const T t)					{ x *= t; y *= t; z *= t; }
	void operator/=(const T t)					{ x /= t; y /= t; z /= t; }

	// Arithmetic operators (vector with vector)
	TVector<T> operator+(const TVector<T> &v) const	{ return TVector<T>(x+v.x, y+v.y, z+v.z); }
	TVector<T> operator-(const TVector<T> &v) const	{ return TVector<T>(x-v.x, y-v.y, z-v.z); }
	TVector<T> operator*(const TVector<T> &v) const	{ return TVector<T>(x*v.x, y*v.y, z*v.z); }
	TVector<T> operator/(const TVector<T> &v) const	{ return TVector<T>(x/v.x, y/v.y, z/v.z); }
	void operator+=(const TVector<T> &v)		{ x += v.x; y += v.y; z += v.z; }
	void operator-=(const TVector<T> &v)		{ x -= v.x; y -= v.y; z -= v.z; }
	void operator*=(const TVector<T> &v)		{ x *= v.x; y *= v.y; z *= v.z; }
	void operator/=(const TVector<T> &v)		{ x /= v.x; y /= v.y; z /= v.z; }

	// Dot and cross product operators
	T operator|(const TVector<T> &v) const		{ return x*v.x + y*v.y + z*v.z; }
	TVector<T> operator^(const TVector<T> &v) const	{ return TVector<T>(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x); }
	void operator^=(const TVector<T> &v)		{ *this = *this ^ v; }

	// Magnitude/distance methods
	T MagnitudeSquared() const					{ return x*x + y*y + z*z; }
	T Magnitude() const							{ return (T)sqrtf((float)MagnitudeSquared()); }
	T DistanceSquared(const TVector<T> &v) const{ return (*this - v).MagnitudeSquared(); }
	T Distance(const TVector<T> &v) const		{ return (*this - v).Magnitude(); }
	TVector<T> Midpoint(const TVector<T> &v) const	{ return CVector((*this - v) / 2 + v); }
	TVector<T> Average(const TVector<T> &v) const	{ return CVector((*this + v) / 2); }

	// Advanced methods (should only be used with float or double types)
	void Normalize()							{ *this /= Magnitude(); }
	double Angle(const TVector<T> &v) const		{ return acos(*this | v); }
	TVector<T> Reflect(const TVector<T> &n) const
	{
		T t = (T)Magnitude();
		TVector<T> v = *this / t;
		return (v - n * (2 * (v | n))) * t;
	}


	TVector<T> Rotate(const T tAngle, const TVector<T> &n) const
	{
		T tCos = (T)cos(tAngle);
		T tSin = (T)sin(tAngle);
		return TVector<T>(*this * tCos + ((n * *this) * (1 - tCos)) * n + (*this ^ n) * tSin);
	}
};

using CVector = TVector<float>;
using CDoubleVector = TVector<double>;



// Returns the normal vector of two vectors (the normalized cross product)
template <class T> inline TVector<T> NormalVector(const TVector<T> &v1, const TVector<T> &v2)
{
	TVector<T> v = v1 ^ v2;
	v.Normalize();
	return v;
}

// Returns the normal vector of a triangle or 3 points on a plane (assumes counter-clockwise orientation)
template <class T> inline TVector<T> NormalVector(const TVector<T> &p1, const TVector<T> &p2, const TVector<T> &p3)
{
	return NormalVector(p2-p1, p3-p1);
}

// Returns the direction vector between two points
template <class T> inline TVector<T> DirectionVector(const TVector<T> &p1, const TVector<T> &p2)
{
	TVector<T> v = p2 - p1;
	v.Normalize();
	return v;
}

template <class T> inline float dot(const TVector<T> &v1, const TVector<T> &v2)
{
	return v1 | v2;
}

template <class T> inline float length(const TVector<T> &v1)
{
	return v1.Magnitude();
}


/*******************************************************************************
* Template Class: TVector4
********************************************************************************
* This template class implements a simple 4D vector with x, y, z, and w
* coordinates. Like TVector, it is templatized and macros are defined for the
* most common types.
*******************************************************************************/
#define CVector4		TVector4<float>
#define CDoubleVector4	TVector4<double>
#define CIntVector4		TVector4<int>
#define CByteVector4	TVector4<unsigned char>
template <class T> class TVector4
{
public:
	T x, y, z, w;

	// Constructors
	TVector4()									{}
	TVector4(const T a, const T b, const T c, const T d)	{ x = a; y = b; z = c; w = d; }
	TVector4(const T t)							{ *this = t; }
	TVector4(const T *pt)						{ *this = pt; }
	TVector4(const TVector<T> &v)				{ *this = v; }
	TVector4(const TVector4<T> &v)				{ *this = v; }

	// Casting and unary operators
	operator T*()								{ return &x; }
	T &operator[](const int n)					{ return (&x)[n]; }
	operator const T*() const					{ return &x; }
	T operator[](const int n) const				{ return (&x)[n]; }
	TVector4<T> operator-() const				{ return TVector4<T>(-x, -y, -z, -w); }

	// Equal and comparison operators
	void operator=(const T t)					{ x = y = z = w = t; }
	void operator=(const T *pt)					{ x = pt[0]; y = pt[1]; z = pt[2]; w = pt[3]; }
	void operator=(const TVector<T> &v)			{ x = v.x; y = v.y; z = v.z; w = 0; }
	void operator=(const TVector4<T> &v)		{ x = v.x; y = v.y; z = v.z; w = v.w; }
	bool operator==(TVector4<T> &v) const		{ return (Abs(x - v.x) <= (T)DELTA && Abs(y - v.y) <= (T)DELTA && Abs(z - v.z) <= (T)DELTA && Abs(w - v.w) <= (T)DELTA); }
	bool operator!=(TVector4<T> &v) const		{ return !(*this == v); }

	// Arithmetic operators (vector with scalar)
	TVector4<T> operator+(const T t) const		{ return TVector4<T>(x+t, y+t, z+t, w+t); }
	TVector4<T> operator-(const T t) const		{ return TVector4<T>(x-t, y-t, z-t, w-t); }
	TVector4<T> operator*(const T t) const		{ return TVector4<T>(x*t, y*t, z*t, w*t); }
	TVector4<T> operator/(const T t) const		{ return TVector4<T>(x/t, y/t, z/t, w/t); }
	void operator+=(const T t)					{ x += t; y += t; z += t; w += t; }
	void operator-=(const T t)					{ x -= t; y -= t; z -= t; w -= t; }
	void operator*=(const T t)					{ x *= t; y *= t; z *= t; w *= t; }
	void operator/=(const T t)					{ x /= t; y /= t; z /= t; w /= t; }

	// Arithmetic operators (vector with vector)
	TVector4<T> operator+(const TVector4<T> &v) const	{ return TVector4<T>(x+v.x, y+v.y, z+v.z, w+v.w); }
	TVector4<T> operator-(const TVector4<T> &v) const	{ return TVector4<T>(x-v.x, y-v.y, z-v.z, w-v.w); }
	TVector4<T> operator*(const TVector4<T> &v) const	{ return TVector4<T>(x*v.x, y*v.y, z*v.z, w*v.w); }
	TVector4<T> operator/(const TVector4<T> &v) const	{ return TVector4<T>(x/v.x, y/v.y, z/v.z, w/v.w); }
	void operator+=(const TVector4<T> &v)		{ x += v.x; y += v.y; z += v.z; w += v.w; }
	void operator-=(const TVector4<T> &v)		{ x -= v.x; y -= v.y; z -= v.z; w -= v.w; }
	void operator*=(const TVector4<T> &v)		{ x *= v.x; y *= v.y; z *= v.z; w *= v.w; }
	void operator/=(const TVector4<T> &v)		{ x /= v.x; y /= v.y; z /= v.z; w /= v.w; }

	// Magnitude/normalize methods
	T MagnitudeSquared() const					{ return x*x + y*y + z*z + w*w; }
	T Magnitude() const							{ return (T)sqrt(MagnitudeSquared()); }
	void Normalize()							{ *this /= Magnitude(); }
};


/*******************************************************************************
* Class: CQuaternion
********************************************************************************
* This class implements a 4D quaternion. Several functions and operators are
* defined to make working with quaternions easier. Quaternions are often used to
* represent rotations, and have a number of advantages over other constructs.
* Their main disadvantage is that they are unintuitive.
*
* Note: This class is not templatized because integral data types don't make sense
*       and there's no need for double-precision.
*******************************************************************************/
class CQuaternion
{
public:
	float x, y, z, w;

	// Constructors
	CQuaternion()								{}
	CQuaternion(const float a, const float b, const float c, const float d)	{ x = a; y = b; z = c; w = d; }
	CQuaternion(const CVector &v, const float f){ SetAxisAngle(v, f); }
	CQuaternion(const CVector &v)				{ *this = v; }
	CQuaternion(const CQuaternion &q)			{ *this = q; }
	CQuaternion(const CMatrix &m)				{ *this = m; }
	CQuaternion(const float *p)					{ *this = p; }

	// Casting and unary operators
	operator float*()							{ return &x; }
	float &operator[](const int n)				{ return (&x)[n]; }
	operator const float*() const				{ return &x; }
	float operator[](const int n) const			{ return (&x)[n]; }
	CQuaternion operator-() const				{ return CQuaternion(-x, -y, -z, -w); }

	// Equal and comparison operators
	void operator=(const CVector &v)			{ x = v.x; y = v.y; z = v.z; w = 0; }
	void operator=(const CQuaternion &q)		{ x = q.x; y = q.y; z = q.z; w = q.w; }
	void operator=(const CMatrix &m);
	void operator=(const float *p)				{ x = p[0]; y = p[1]; z = p[2]; w = p[3]; }

	// Arithmetic operators (quaternion and scalar)
	CQuaternion operator+(const float f) const	{ return CQuaternion(x+f, y+f, z+f, w+f); }
	CQuaternion operator-(const float f) const	{ return CQuaternion(x-f, y-f, z-f, w-f); }
	CQuaternion operator*(const float f) const	{ return CQuaternion(x*f, y*f, z*f, w*f); }
	CQuaternion operator/(const float f) const	{ return CQuaternion(x/f, y/f, z/f, w/f); }
	void operator+=(const float f)				{ x+=f; y+=f; z+=f; w+=f; }
	void operator-=(const float f)				{ x-=f; y-=f; z-=f; w-=f; }
	void operator*=(const float f)				{ x*=f; y*=f; z*=f; w*=f; }
	void operator/=(const float f)				{ x/=f; y/=f; z/=f; w/=f; }

	// Arithmetic operators (quaternion and quaternion)
	CQuaternion operator+(const CQuaternion &q) const	{ return CQuaternion(x+q.x, y+q.y, z+q.z, w+q.w); }
	CQuaternion operator-(const CQuaternion &q) const	{ return CQuaternion(x-q.x, y-q.y, z-q.z, w-q.w); }
	CQuaternion operator*(const CQuaternion &q) const;	// Multiplying quaternions is a special operation
	void operator+=(const CQuaternion &q)	{ x+=q.x; y+=q.y; z+=q.z; w+=q.w; }
	void operator-=(const CQuaternion &q)	{ x-=q.x; y-=q.y; z-=q.z; w-=q.w; }
	void operator*=(const CQuaternion &q)	{ *this = *this * q; }

	// Magnitude/normalize methods
	float MagnitudeSquared() const			{ return x*x + y*y + z*z + w*w; }
	float Magnitude() const					{ return sqrtf(MagnitudeSquared()); }
	void Normalize()						{ *this /= Magnitude(); }

	// Advanced quaternion methods
	CQuaternion Conjugate() const			{ return CQuaternion(-x, -y, -z, w); }
	CQuaternion Inverse() const				{ return Conjugate() / MagnitudeSquared(); }
	CQuaternion UnitInverse() const			{ return Conjugate(); }

	CVector RotateVector(const CVector &v) const	{ 
      CVector rv(CQuaternion(*this) * CQuaternion(v) * UnitInverse());
      return rv;
   }

	void SetAxisAngle(const CVector &vAxis, const float fAngle)
	{
		// 4 muls, 2 trig function calls
		float f = fAngle * 0.5f;
		*this = vAxis * sinf(f);
		w = cosf(f);
	}
	void GetAxisAngle(CVector &vAxis, float &fAngle) const
	{
		// 4 muls, 1 div, 2 trig function calls
		fAngle = acosf(w);
		vAxis = *this / sinf(fAngle);
		fAngle *= 2.0f;
	}

	void Rotate(const CQuaternion &q)			{ *this = q * *this; }
	void Rotate(const CVector &vAxis, const float fAngle)
	{
		CQuaternion q;
		q.SetAxisAngle(vAxis, fAngle);
		Rotate(q);
	}
	void Rotate(const CVector &vFrom, const CVector &vTo)
	{
		CVector vAxis = vFrom ^ vTo;
		vAxis.Normalize();
		float fCos = vFrom | vTo;
		Rotate(vAxis, acosf(fCos));
	}

	CVector GetViewAxis() const
	{
		// 6 muls, 7 adds
		float x2 = x + x, y2 = y + y, z2 = z + z;
		float xx = x * x2, xz = x * z2;
		float yy = y * y2, yz = y * z2;
		float wx = w * x2, wy = w * y2;
		return -CVector(xz+wy, yz-wx, 1-(xx+yy));
	}
	CVector GetUpAxis() const
	{
		// 6 muls, 7 adds
		float x2 = x + x, y2 = y + y, z2 = z + z;
		float xx = x * x2, xy = x * y2;
		float yz = y * z2, zz = z * z2;
		float wx = w * x2, wz = w * z2;
		return CVector(xy-wz, 1-(xx+zz), yz+wx);
	}
	CVector GetRightAxis() const
	{
		// 6 muls, 7 adds
		float x2 = x + x, y2 = y + y, z2 = z + z;
		float xy = x * y2, xz = x * z2;
		float yy = y * y2, zz = z * z2;
		float wy = w * y2, wz = w * z2;
		return CVector(1-(yy+zz), xy+wz, xz-wy);
	}
};

extern CQuaternion Slerp(const CQuaternion &q1, const CQuaternion &q2, const float t);

/*******************************************************************************
* Class: CMatrix
********************************************************************************
* This class implements a 4x4 matrix. Several functions and operators are
* defined to make working with matrices easier. The values are kept in column-
* major order to make it easier to use with OpenGL. For performance reasons,
* most of the functions assume that all matrices are orthogonal, which means the
* bottom row is [ 0 0 0 1 ]. Since I plan to use the GL_PROJECTION matrix to
* handle the projection matrix, I should never need to use any other kind of
* matrix, and I get a decent performance boost by ignoring the bottom row.
*
* Note: This class is not templatized because integral data types don't make sense
*       and there's no need for double-precision.
*******************************************************************************/
class CMatrix
{
public:
	// This class uses column-major order, as used by OpenGL
	// Here are the ways in which the matrix values can be accessed:
	// | f11 f21 f31 f41 |   | f1[0] f1[4] f1[8]  f1[12] |   | f2[0][0] f2[1][0] f2[2][0] f2[3][0] |
	// | f12 f22 f32 f42 |   | f1[1] f1[5] f1[9]  f1[13] |   | f2[0][1] f2[1][1] f2[2][1] f2[3][1] |
	// | f13 f23 f33 f43 | = | f1[2] f1[6] f1[10] f1[14] | = | f2[0][2] f2[1][2] f2[2][2] f2[3][2] |
	// | f14 f24 f34 f44 |   | f1[3] f1[7] f1[11] f1[15] |   | f2[0][3] f2[1][3] f2[2][3] f2[3][3] |
	union
	{
		struct { float f11, f12, f13, f14, f21, f22, f23, f24, f31, f32, f33, f34, f41, f42, f43, f44; };
		float f1[16];
		float f2[4][4];
	};

	CMatrix()							{}
	CMatrix(const float f)				{ *this = f; }
	CMatrix(const float *pf)			{ *this = pf; }
	CMatrix(const CQuaternion &q)		{ *this = q; }

	// Init functions
	void ZeroMatrix()
	{
		f11 = f12 = f13 = f14 = f21 = f22 = f23 = f24 = f31 = f32 = f33 = f34 = f41 = f42 = f43 = f44 = 0;
	}
	void IdentityMatrix()
	{
		f12 = f13 = f14 = f21 = f23 = f24 = f31 = f32 = f34 = f41 = f42 = f43 = 0;
		f11 = f22 = f33 = f44 = 1;
	}

	const float* Data() const { return f1; }
	float &operator[](const int n)					{ return f1[n]; }
	float &operator()(const int i, const int j)		{ return f2[i][j]; }
	operator const float*() const					{ return f1; }
	float operator[](const int n) const				{ return f1[n]; }
	float operator()(const int i, const int j) const{ return f2[i][j]; }

	void operator=(const float f)					{ for(register int i=0; i<16; i++) f1[i] = f; }
	void operator=(const float *pf)					{ for(register int i=0; i<16; i++) f1[i] = pf[i]; }
	void operator=(const CQuaternion &q);

	CMatrix operator*(const CMatrix &m) const;
	void operator*=(const CMatrix &m)				{ *this = *this * m; }
	CVector operator*(const CVector &v) const		{ return TransformVector(v); }

	CVector TransformVector(const CVector &v) const
	{
		// 9 muls, 9 adds
		// | f11 f21 f31 f41 |   | v.x |   | f11*v.x+f21*v.y+f31*v.z+f41 |
		// | f12 f22 f32 f42 |   | v.y |   | f12*v.x+f22*v.y+f32*v.z+f42 |
		// | f13 f23 f33 f43 | * | v.z | = | f13*v.x+f23*v.y+f33*v.z+f43 |
		// | 0   0   0   1   |   | 1   |   | 1                           |
		return CVector((f11*v.x+f21*v.y+f31*v.z+f41),
					   (f12*v.x+f22*v.y+f32*v.z+f42),
					   (f13*v.x+f23*v.y+f33*v.z+f43));
	}
	CVector TransformNormal(const CVector &v) const
	{
		// 9 muls, 6 adds
		// (Transpose rotation vectors, ignore position)
		// | f11 f12 f13 0 |   | v.x |   | f11*v.x+f12*v.y+f13*v.z |
		// | f21 f22 f23 0 |   | v.y |   | f21*v.x+f22*v.y+f23*v.z |
		// | f31 f32 f33 0 | * | v.z | = | f31*v.x+f32*v.y+f33*v.z |
		// | 0   0   0   1 |   | 1   |   | 1                       |
		return CVector((f11*v.x+f12*v.y+f13*v.z),
					   (f21*v.x+f22*v.y+f23*v.z),
					   (f31*v.x+f32*v.y+f33*v.z));
	}

	// Translate functions
	void TranslateMatrix(const float x, const float y, const float z)
	{
		// | 1  0  0  x |
		// | 0  1  0  y |
		// | 0  0  1  z |
		// | 0  0  0  1 |
		f12 = f13 = f14 = f21 = f23 = f24 = f31 = f32 = f34 = 0;
		f11 = f22 = f33 = f44 = 1;
		f41 = x; f42 = y; f43 = z;
	}
	void TranslateMatrix(const float *pf)		{ TranslateMatrix(pf[0], pf[1], pf[2]); }
	void Translate(const float x, const float y, const float z)
	{
		// 9 muls, 9 adds
		// | f11 f21 f31 f41 |   | 1  0  0  x |   | f11 f21 f31 f11*x+f21*y+f31*z+f41 |
		// | f12 f22 f32 f42 |   | 0  1  0  y |   | f12 f22 f32 f12*x+f22*y+f32*z+f42 |
		// | f13 f23 f33 f43 | * | 0  0  1  z | = | f13 f23 f33 f13*x+f23*y+f33*z+f43 |
		// | 0   0   0   1   |   | 0  0  0  1 |   | 0   0   0   1                     |
		f41 = f11*x+f21*y+f31*z+f41;
		f42 = f12*x+f22*y+f32*z+f42;
		f43 = f13*x+f23*y+f33*z+f43;
	}
	void Translate(const float *pf)				{ Translate(pf[0], pf[1], pf[2]); }

	// Scale functions
	void ScaleMatrix(const float x, const float y, const float z)
	{
		// | x  0  0  0 |
		// | 0  y  0  0 |
		// | 0  0  z  0 |
		// | 0  0  0  1 |
		f12 = f13 = f14 = f21 = f23 = f24 = f31 = f32 = f34 = f41 = f42 = f43 = 0;
		f11 = x; f22 = y; f33 = z; f44 = 1;
	}
	void ScaleMatrix(const float *pf)			{ ScaleMatrix(pf[0], pf[1], pf[2]); }
	void Scale(const float x, const float y, const float z)
	{
		// 9 muls
		// | f11 f21 f31 f41 |   | x  0  0  0 |   | f11*x f21*y f31*z f41 |
		// | f12 f22 f32 f42 |   | 0  y  0  0 |   | f12*x f22*y f32*z f42 |
		// | f13 f23 f33 f43 | * | 0  0  z  0 | = | f13*x f23*y f33*z f43 |
		// | 0   0   0   1   |   | 0  0  0  1 |   | 0     0     0     1   |
		f11 *= x; f21 *= y; f31 *= z;
		f12 *= x; f22 *= y; f32 *= z;
		f13 *= x; f23 *= y; f33 *= z;
	}
	void Scale(const float *pf)					{ Scale(pf[0], pf[1], pf[2]); }

	// Rotate functions
	void RotateXMatrix(const float fRadians)
	{
		// | 1 0    0     0 |
		// | 0 fCos -fSin 0 |
		// | 0 fSin fCos  0 |
		// | 0 0    0     1 |
		f12 = f13 = f14 = f21 = f24 = f31 = f34 = f41 = f42 = f43 = 0;
		f11 = f44 = 1;

		float fCos = cosf(fRadians);
		float fSin = sinf(fRadians);
		f22 = f33 = fCos;
		f23 = fSin;
		f32 = -fSin;
	}
	void RotateX(const float fRadians)
	{
		// 12 muls, 6 adds, 2 trig function calls
		// | f11 f21 f31 f41 |   | 1 0    0     0 |   | f11 f21*fCos+f31*fSin f31*fCos-f21*fSin f41 |
		// | f12 f22 f32 f42 |   | 0 fCos -fSin 0 |   | f12 f22*fCos+f32*fSin f32*fCos-f22*fSin f42 |
		// | f13 f23 f33 f43 | * | 0 fSin fCos  0 | = | f13 f23*fCos+f33*fSin f33*fCos-f23*fSin f43 |
		// | 0   0   0   1   |   | 0 0    0     1 |   | 0   0                 0                 1   |
		float fTemp, fCos, fSin;
		fCos = cosf(fRadians);
		fSin = sinf(fRadians);
		fTemp = f21*fCos+f31*fSin;
		f31 = f31*fCos-f21*fSin;
		f21 = fTemp;
		fTemp = f22*fCos+f32*fSin;
		f32 = f32*fCos-f22*fSin;
		f22 = fTemp;
		fTemp = f23*fCos+f33*fSin;
		f33 = f33*fCos-f23*fSin;
		f23 = fTemp;
	}
	void RotateYMatrix(const float fRadians)
	{
		// | fCos  0 fSin  0 |
		// | 0     1 0     0 |
		// | -fSin 0 fCos  0 |
		// | 0     0 0     1 |
		f12 = f14 = f21 = f23 = f24 = f32 = f34 = f41 = f42 = f43 = 0;
		f22 = f44 = 1;

		float fCos = cosf(fRadians);
		float fSin = sinf(fRadians);
		f11 = f33 = fCos;
		f13 = -fSin;
		f31 = fSin;
	}
	void RotateY(const float fRadians)
	{
		// 12 muls, 6 adds, 2 trig function calls
		// | f11 f21 f31 f41 |   | fCos  0 fSin  0 |   | f11*fCos-f31*fSin f21 f11*fSin+f31*fCos f41 |
		// | f12 f22 f32 f42 |   | 0     1 0     0 |   | f12*fCos-f32*fSin f22 f12*fSin+f32*fCos f42 |
		// | f13 f23 f33 f43 | * | -fSin 0 fCos  0 | = | f13*fCos-f33*fSin f23 f13*fSin+f33*fCos f43 |
		// | 0   0   0   1   |   | 0     0 0     1 |   | 0                 0   0                 1   |
		float fTemp, fCos, fSin;
		fCos = cosf(fRadians);
		fSin = sinf(fRadians);
		fTemp = f11*fCos-f31*fSin;
		f31 = f11*fSin+f31*fCos;
		f11 = fTemp;
		fTemp = f12*fCos-f32*fSin;
		f32 = f12*fSin+f32*fCos;
		f12 = fTemp;
		fTemp = f13*fCos-f33*fSin;
		f33 = f13*fSin+f33*fCos;
		f13 = fTemp;
	}
	void RotateZMatrix(const float fRadians)
	{
		// | fCos -fSin 0 0 |
		// | fSin fCos  0 0 |
		// | 0    0     1 0 |
		// | 0    0     0 1 |
		f13 = f14 = f23 = f24 = f31 = f32 = f34 = f41 = f42 = f43 = 0;
		f33 = f44 = 1;

		float fCos = cosf(fRadians);
		float fSin = sinf(fRadians);
		f11 = f22 = fCos;
		f12 = fSin;
		f21 = -fSin;
	}
	void RotateZ(const float fRadians)
	{
		// 12 muls, 6 adds, 2 trig function calls
		// | f11 f21 f31 f41 |   | fCos -fSin 0 0 |   | f11*fCos+f21*fSin f21*fCos-f11*fSin f31 f41 |
		// | f12 f22 f32 f42 |   | fSin fCos  0 0 |   | f12*fCos+f22*fSin f22*fCos-f12*fSin f32 f42 |
		// | f13 f23 f33 f43 | * | 0    0     1 0 | = | f13*fCos+f23*fSin f23*fCos-f13*fSin f33 f43 |
		// | 0   0   0   1   |   | 0    0     0 1 |   | 0                 0                 0   1   |
		float fTemp, fCos, fSin;
		fCos = cosf(fRadians);
		fSin = sinf(fRadians);
		fTemp = f11*fCos+f21*fSin;
		f21 = f21*fCos-f11*fSin;
		f11 = fTemp;
		fTemp = f12*fCos+f22*fSin;
		f22 = f22*fCos-f12*fSin;
		f12 = fTemp;
		fTemp = f13*fCos+f23*fSin;
		f23 = f23*fCos-f13*fSin;
		f13 = fTemp;
	}
	void RotateMatrix(const CVector &v, const float f)
	{
		// 15 muls, 10 adds, 2 trig function calls
		float fCos = cosf(f);
		CVector vCos = v * (1 - fCos);
		CVector vSin = v * sinf(f);

		f14 = f24 = f34 = f41 = f42 = f43 = 0;
		f44 = 1;

		f11 = (v.x * vCos.x) + fCos;
		f21 = (v.x * vCos.y) - (vSin.z);
		f31 = (v.x * vCos.z) + (vSin.y);
		f12 = (v.y * vCos.x) + (vSin.z);
		f22 = (v.y * vCos.y) + fCos;
		f32 = (v.y * vCos.z) - (vSin.x);
		f13 = (v.z * vCos.x) - (vSin.y);
		f32 = (v.z * vCos.y) + (vSin.x);
		f33 = (v.z * vCos.z) + fCos;
	}
	void Rotate(const CVector &v, const float f)
	{
		// 51 muls, 37 adds, 2 trig function calls
		CMatrix mat;
		mat.RotateMatrix(v, f);
		*this *= mat;
	}

	void ModelMatrix(const CQuaternion &q, const CVector &vFrom)
	{
		*this = q;
		f41 = vFrom.x;
		f42 = vFrom.y;
		f43 = vFrom.z;
	}
	void ModelMatrix(const CVector &vFrom, const CVector &vView, const CVector &vUp, const CVector &vRight)
	{
		f11 = vRight.x;	f21 = vUp.x;	f31 = -vView.x;	f41 = vFrom.x;
		f12 = vRight.y;	f22 = vUp.y;	f32 = -vView.y;	f42 = vFrom.y;
		f13 = vRight.z;	f23 = vUp.z;	f33 = -vView.z;	f43 = vFrom.z;
		f14 = 0;		f24 = 0;		f34 = 0;		f44 = 1;
	}
	void ModelMatrix(const CVector &vFrom, const CVector &vAt, const CVector &vUp)
	{
		CVector vView = vAt - vFrom;
		vView.Normalize();
		CVector vRight = vView ^ vUp;
		vRight.Normalize();
		CVector vTrueUp = vRight ^ vView;
		vTrueUp.Normalize();
		ModelMatrix(vFrom, vView, vTrueUp, vRight);
	}

	void ViewMatrix(const CQuaternion &q, const CVector &vFrom)
	{
		*this = q;
		Transpose();
		f41 = -(vFrom.x*f11 + vFrom.y*f21 + vFrom.z*f31);
		f42 = -(vFrom.x*f12 + vFrom.y*f22 + vFrom.z*f32);
		f43 = -(vFrom.x*f13 + vFrom.y*f23 + vFrom.z*f33);
	}
	void ViewMatrix(const CVector &vFrom, const CVector &vView, const CVector &vUp, const CVector &vRight)
	{
		// 9 muls, 9 adds
		f11 = vRight.x;	f21 = vRight.y;	f31 = vRight.z;	f41 = -(vFrom | vRight);
		f12 = vUp.x;	f22 = vUp.y;	f32 = vUp.z;	f42 = -(vFrom | vUp);
		f13 = -vView.x;	f23 = -vView.y;	f33 = -vView.z;	f43 = -(vFrom | -vView);
		f14 = 0;		f24 = 0;		f34 = 0;		f44 = 1;
	}
	void ViewMatrix(const CVector &vFrom, const CVector &vAt, const CVector &vUp)
	{
		CVector vView = vAt - vFrom;
		vView.Normalize();
		CVector vRight = vView ^ vUp;
		vRight.Normalize();
		CVector vTrueUp = vRight ^ vView;
		vTrueUp.Normalize();
		ViewMatrix(vFrom, vView, vTrueUp, vRight);
	}

	void ProjectionMatrix(const float fNear, const float fFar, const float fFOV, const float fAspect)
	{
		// 2 muls, 3 divs, 2 adds, 1 trig function call
		float h = 1.0f / tanf(DegToRad(fFOV * 0.5f));
		float Q = fFar / (fFar - fNear);
		f12 = f13 = f14 = f21 = f23 = f24 = f31 = f32 = f41 = f42 = f44 = 0;
		f11 = h / fAspect;
		f22 = h;
		f33 = Q;
		f34 = 1;
		f43 = -Q*fNear;
	}

	// For orthogonal matrices, I belive this also gives you the inverse.
	void Transpose()
	{
		(std::swap)(f12, f21);
		(std::swap)(f13, f31);
		(std::swap)(f14, f41);
		(std::swap)(f23, f32);
		(std::swap)(f24, f42);
		(std::swap)(f34, f43);
	}
};


inline bool operator ==(const CMatrix& m1, const CMatrix& m2) {
   auto equal = true;
   for(auto i = 0; equal && i < 16; ++i)
      equal = m1.f1[i] == m2.f1[i];

   return equal;
}


inline glm::fquat QuatToGlm(const CQuaternion& q) {
   return glm::fquat(q.w, q.x, q.y, q.z);
}

inline CQuaternion GlmToQuat(const glm::fquat& q) {
   return CQuaternion(q.x, q.y, q.z, q.w);
}


inline glm::mat4 MatToGlm(const CMatrix& mat) {
   auto m = glm::mat4();
   std::copy(std::begin(mat.f1), std::end(mat.f1), glm::value_ptr(m));
   return m;
}

inline CMatrix GlmToMat(const glm::mat4& mat) {
   return CMatrix(glm::value_ptr(mat));
}

inline bool operator==(const CQuaternion& q1, const glm::fquat& q2) {
   return   q1.x == q2.x &&
            q1.y == q2.y &&
            q1.z == q2.z &&
            q1.w == q2.w;
}

inline bool operator==(const glm::fquat& q1, const CQuaternion& q2) {
   return q2 == q1;
}



/****************************************************************************
* Class: C3DObject
*****************************************************************************
* This class represents a basic 3D object in the scene. It has a 3D position,
* orientation, velocity, and a parent which provides its frame of reference
* in the scene.
* Note: This class is derived from CQuaternion so it will inherit useful
*       functions like Rotate(), GetViewAxis(), GetUpAxis(), and GetRightAxis().
****************************************************************************/
class C3DObject {
protected:
   glm::fquat     orient_;
   CQuaternion    qOrient_;

   glm::dvec3     position_;
   glm::vec3      velocity_;

public:
	C3DObject() : 
      orient_(1.0f, 0.0f, 0.0f, 0.0f), 
      qOrient_(GlmToQuat(orient_)), 
      position_(0.0f), 
      velocity_(0.0f) {}

   C3DObject& SetOrientation(const CQuaternion& q) {
      qOrient_ = q;
      orient_ = glm::normalize(QuatToGlm(qOrient_));
      assert(orient_ == qOrient_);
      return *this;
   }

   C3DObject& SetPosition(CDoubleVector &v) {
      position_[0] = v[0];
      position_[1] = v[1];
      position_[2] = v[2];
      return *this;
   }

	CDoubleVector GetPosition() { 
      return CDoubleVector(position_[0], position_[1], position_[2]);
   }
	
   C3DObject& SetVelocity(CVector &v) { 
      velocity_[0] = v[0];
      velocity_[1] = v[1];
      velocity_[2] = v[2];
      return *this;
   }
	
   CVector GetVelocity() { 
      return CVector(velocity_[0], velocity_[1], velocity_[2]);
   }

	CMatrix GetViewMatrix()	{
      assert(orient_ == qOrient_);

      CMatrix m1 = GlmToQuat(orient_);
		m1.Transpose();

      CMatrix m2 = qOrient_;
      m2.Transpose();
      assert(m1 == m2);

		return m2;
	}

   void Rotate(const CVector& axis, const float angle) {
      auto q = GlmToQuat(orient_);
      q.Rotate(axis, angle);
      orient_ = QuatToGlm(q);
      qOrient_.Rotate(axis, angle);

      assert(orient_ == qOrient_);
   }

   CVector GetUpAxis() const {
      return GlmToQuat(orient_).GetUpAxis();
   }


   CVector GetRightAxis() const {
      return GlmToQuat(orient_).GetRightAxis();
   }


   CVector GetViewAxis() const {
      return GlmToQuat(orient_).GetViewAxis();
   }


   CMatrix GetModelMatrix(C3DObject *pCamera) {
      assert(orient_ == qOrient_);

		// Don't use the normal model matrix because it causes precision problems if the camera and model are too far away from the origin.
		// Instead, pretend the camera is at the origin and offset all model matrices by subtracting the camera's position.
		CMatrix m1;
		m1.ModelMatrix(GlmToQuat(orient_), GetPosition() - pCamera->GetPosition());

      CMatrix m2;
      m2.ModelMatrix(qOrient_, GetPosition() - pCamera->GetPosition());

      assert(m1 == m2);
		return m2;
	}

	void Accelerate(CVector &vAccel, float seconds, float resistance=0){
      const auto accel = glm::vec3(vAccel[0], vAccel[1], vAccel[2]);
		velocity_ += accel * seconds;
		if(resistance > DELTA)
         velocity_ *= 1.0f - resistance * seconds;

      position_ += velocity_ * seconds;
	}

};

