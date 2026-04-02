#pragma once
#include <cmath>

struct Vec3
{
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3  operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3  operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3  operator*(float s)       const { return {x*s,   y*s,   z*s};   }
    Vec3& operator+=(const Vec3& o){ x+=o.x; y+=o.y; z+=o.z; return *this; }

    float dot  (const Vec3& o) const { return x*o.x + y*o.y + z*o.z; }
    Vec3  cross(const Vec3& o) const {
        return { y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x };
    }
    float len() const { return std::sqrt(x*x + y*y + z*z); }
    Vec3  normalized() const {
        float l = len();
        return l > 1e-9f ? Vec3(x/l, y/l, z/l) : Vec3(0,1,0);
    }
};

// Rodrigues rotation: rotate v by deg degrees around unit axis
inline Vec3 rotateAround(const Vec3& v, const Vec3& axis, float deg)
{
    float r = deg * 3.14159265f / 180.f;
    float c = std::cos(r), s = std::sin(r);
    return v * c + axis.cross(v) * s + axis * (axis.dot(v) * (1.f - c));
}
