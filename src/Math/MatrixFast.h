/*
	Author: bitluni 2019
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
		http://bitluni.net
*/
#pragma once
#include <math.h>

class Vector
{
public:
	float v[4];

	Vector(float x = 0, float y = 0, float z = 0, float w = 1)
	{
		v[0] = x;
		v[1] = y;
		v[2] = z;
		v[3] = w;
	}

	Vector operator*(float s) const
	{
		return Vector(v[0] * s, v[1] * s, v[2] * s, 1);
	}

	Vector &operator*=(float s)
	{
		*this = *this * s;
		return *this;
	}

	float operator[](int i) const
	{
		return v[i];
	}

	Vector operator+(const Vector &v2) const
	{
		return Vector(v[0] + v2.v[0], v[1] + v2.v[1], v[2] + v2.v[2], 1);
	}

	Vector operator-(const Vector &v2) const
	{
		return Vector(v[0] - v2.v[0], v[1] - v2.v[1], v[2] - v2.v[2], 1);
	}

	Vector operator-() const
	{
		return Vector(-v[0], -v[1], -v[2], 1);
	}

	void normalize()
	{
		float l2 = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
		if (!l2)
			return;
		float rl = 1 / sqrt(l2);
		*this *= rl;
	}

	float length() const
	{
		float l2 = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
		if (!l2) return 0.f;
		return sqrt(l2);
	}

	float dot(const Vector &v2) const
	{
		return v[0] * v2.v[0] + v[1] * v2.v[1] + v[2] * v2.v[2];
	}
};

class Matrix
{
public:
	float m[4][4];

	Matrix()
		: Matrix(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)
	{
	}

	Matrix(float m00, float m01, float m02, float m03,
		   float m10, float m11, float m12, float m13,
		   float m20, float m21, float m22, float m23,
		   float m30, float m31, float m32, float m33)
	{
		m[0][0] = m00;
		m[0][1] = m01;
		m[0][2] = m02;
		m[0][3] = m03;
		m[1][0] = m10;
		m[1][1] = m11;
		m[1][2] = m12;
		m[1][3] = m13;
		m[2][0] = m20;
		m[2][1] = m21;
		m[2][2] = m22;
		m[2][3] = m23;
		m[3][0] = m30;
		m[3][1] = m31;
		m[3][2] = m32;
		m[3][3] = m33;
	}

	static Matrix identity()
	{
		return Matrix(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
	}

	static Matrix scaling(float s)
	{
		return Matrix(s, 0, 0, 0, 0, s, 0, 0, 0, 0, s, 0, 0, 0, 0, 1);
	}

	static Matrix scaling(float u, float v, float w)
	{
		return Matrix(u, 0, 0, 0, 0, v, 0, 0, 0, 0, w, 0, 0, 0, 0, 1);
	}

	static Matrix translation(float x, float y, float z)
	{
		return Matrix(1, 0, 0, x, 0, 1, 0, y, 0, 0, 1, z, 0, 0, 0, 1);
	}

	static Matrix rotation(float a, float x, float y, float z)
	{
		float cosa = cos(a);
		float rcosa = 1 - cosa;
		float sina = sin(a);
		return Matrix(
			x * x * rcosa + cosa, x * y * rcosa - z * sina, x * z * rcosa + y * sina, 0,
			y * x * rcosa + z * sina, y * y * rcosa + cosa, y * z * rcosa - x * sina, 0,
			z * x * rcosa - y * sina, z * y * rcosa + x * sina, z * z * rcosa + cosa, 0,
			0, 0, 0, 1);
	}

	static Matrix perspective(float fov, float near, float far)
	{
		float scale = tan(fov * 0.5 * M_PI / 180);
		return Matrix(
			scale, 0, 0, 0,
			0, scale, 0, 0,
			0, 0, -far * near / (far - near), 0,
			0, 0, -1, 0);
	}

	Vector operator*(const Vector &v)
	{
		return Vector(
			v.v[0] * m[0][0] + v.v[1] * m[0][1] + v.v[2] * m[0][2] + m[0][3],
			v.v[0] * m[1][0] + v.v[1] * m[1][1] + v.v[2] * m[1][2] + m[1][3],
			v.v[0] * m[2][0] + v.v[1] * m[2][1] + v.v[2] * m[2][2] + m[2][3],
			v.v[0] * m[3][0] + v.v[1] * m[3][1] + v.v[2] * m[3][2] + m[3][3]);
	}

	Matrix operator*(const Matrix &m2)
	{
		Matrix mr;
		for (int y = 0; y < 4; y++)
			for (int x = 0; x < 4; x++)
				mr.m[y][x] = m[y][0] * m2.m[0][x] + m[y][1] * m2.m[1][x] + m[y][2] * m2.m[2][x] + m[y][3] * m2.m[3][x];
		return mr;
	}

	Matrix &operator*=(const Matrix &m2)
	{
		*this = *this * m2;
		return *this;
	}
};
