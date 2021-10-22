# pragma once

#include <cmath>

namespace Math{

class Vec4f
{
public:
    Vec4f() :x(0), y(0), z(0), w(0)  {}
    Vec4f(float _x, float _y , float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}

    inline Vec4f operator + (const float a) const       { return Vec4f(x + a, y + a, z + a, w + a); }
    inline Vec4f operator - (const float a) const       { return Vec4f(x - a, y - a, z - a, w - a); }
    inline Vec4f operator * (const float a) const       { return Vec4f(x * a, y * a, z * a, w * a); }
    inline Vec4f operator / (const float a) const       { return Vec4f(x / a, y / a, z / a, w / a); }

    inline Vec4f& operator += (const float a)     { x += a; y += a; z += a; w += a; return *this; }
    inline Vec4f& operator -= (const float a)     { x -= a; y -= a; z -= a; w -= a; return *this; }
    inline Vec4f& operator *= (const float a)     { x *= a; y *= a; z *= a; w *= a; return *this; }
    inline Vec4f& operator /= (const float a)     { x /= a; y /= a; z /= a; w /= a; return *this; }

    inline Vec4f operator + (const Vec4f &v) const      { return Vec4f(x + v.x, y + v.y, z + v.z, w + v.w); }
    inline Vec4f operator - (const Vec4f &v) const      { return Vec4f(x - v.x, y - v.y, z - v.z, w - v.w); }
    inline Vec4f operator * (const Vec4f &v) const      { return Vec4f(x * v.x, y * v.y, z * v.z, w * v.w); }

    inline Vec4f& operator += (const Vec4f &v)      { x+= v.x; y += v.y; z += v.z; w += v.w; return *this; }
    inline Vec4f& operator -= (const Vec4f &v)      { x-= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
    inline Vec4f& operator *= (const Vec4f &v)      { x*= v.x; y *= v.y; z *= v.z; w -= v.w; return *this; }

    inline bool operator == (const Vec4f &v) const      { return x == v.x && y == v.y && z == v.z && w == v.w; }
    inline bool operator != (const Vec4f &v) const      { return x != v.x && y != v.y && z != v.z && w != v.w; }

public:
    union {
        struct { float x, y, z, w; };
        struct { float r, g, b, a; };
        struct { float v[4];    };
    };

};

}