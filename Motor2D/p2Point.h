// ----------------------------------------------------
// Point class    -----------
// ----------------------------------------------------

#ifndef __P2POINT_H__
#define __P2POINT_H__

#include "p2Defs.h"
#include <math.h>

template<class TYPE>
class p2Point
{
public:

	TYPE x, y;

	p2Point()
	{}

	p2Point(const p2Point<TYPE>& v)
	{
		this->x = v.x;
		this->y = v.y;
	}

	p2Point(const TYPE& x, const TYPE& y)
	{
		this->x = x;
		this->y = y;
	}

	p2Point& create(const TYPE& x, const TYPE& y)
	{
		this->x = x;
		this->y = y;

		return(*this);
	}

	// Math ------------------------------------------------
	p2Point operator -(const p2Point &v) const
	{
		p2Point r;

		r.x = x - v.x;
		r.y = y - v.y;

		return(r);
	}

	p2Point operator + (const p2Point &v) const
	{
		p2Point r;

		r.x = x + v.x;
		r.y = y + v.y;

		return(r);
	}

	const p2Point& operator -=(const p2Point &v)
	{
		x -= v.x;
		y -= v.y;

		return(*this);
	}

	const p2Point& operator +=(const p2Point &v)
	{
		x += v.x;
		y += v.y;

		return(*this);
	}

	bool operator ==(const p2Point& v) const
	{
		return (x == v.x && y == v.y);
	}

	bool operator !=(const p2Point& v) const
	{
		return (x != v.x || y != v.y);
	}

	// Utils ------------------------------------------------
	bool IsZero() const
	{
		return (x == 0 && y == 0);
	}

	p2Point& SetToZero()
	{
		x = y = 0;
		return(*this);
	}

	p2Point& Negate()
	{
		x = -x;
		y = -y;

		return(*this);
	}

	// Distances ---------------------------------------------
	TYPE DistanceTo(const p2Point& v) const
	{
		TYPE fx = x - v.x;
		TYPE fy = y - v.y;

		return sqrtf((fx*fx) + (fy*fy));
	}

	TYPE DistanceNoSqrt(const p2Point& v) const
	{
		TYPE fx = x - v.x;
		TYPE fy = y - v.y;

		return (fx*fx) + (fy*fy);
	}

	TYPE DistanceManhattan(const p2Point& v) const
	{
		return abs(v.x - x) + abs(v.y - y);
	}

	bool PointInRect(const TYPE rx, const TYPE ry, const TYPE rw, const TYPE rh)const
	{
		bool ret = false;

		if (x > rx && y > ry && x < rx + rw &&  y < ry + rh)
			ret = true;

		return ret;
	}

	int Sign(float value)const
	{
		return (value >= 0) ? 1 : -1;
	}

	void Normalize()
	{
		float module = sqrt(x * x + y * y);
		x = x / module;
		y = y / module;
	}


	float AngleFromVector()
	{
		float angle;

		angle = atan2f(y, x);

		return angle;
	}

	//Scales the vector in x units
	void Scale(int value)
	{
		fPoint point(x, y);
		float module = sqrt(x * x + y * y);
		point.x = x / module;
		point.y = y / module;

		point.x = point.x * value;
		point.y = point.y * value;

		x = point.x;
		y = point.y;
	}

	//Rotates clockwise the vector
	void Rotate(float radians)
	{
		fPoint point;

		point.x = (cos(radians) * x) + (sin(radians) * y);
		point.y = (-sin(radians) * x) + (cos(radians) * y);

		x = point.x;
		y = point.y;
	}

	float sign(p2Point<float> p1, p2Point<float> p2, p2Point<float> p3)
	{
		return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
	}

	bool PointInTriangle(p2Point<float> p1, p2Point<float> p2, p2Point<float> p3)
	{
		bool b1, b2, b3;
		fPoint p0(x, y);

		b1 = sign(p0, p1, p2) < 0.0f;
		b2 = sign(p0, p2, p3) < 0.0f;
		b3 = sign(p0, p3, p1) < 0.0f;

		return ((b1 == b2) && (b2 == b3));
	}
};

typedef p2Point<int> iPoint;
typedef p2Point<float> fPoint;

#endif // __P2POINT_H__