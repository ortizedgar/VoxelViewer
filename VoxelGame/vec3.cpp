#include "StdAfx.h"
#include "vec3.h"

vec3 vec3::cross(vec3 u, vec3 v)
{
    return vec3(u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x);
}

float vec3::dot(vec3 u, vec3 v)
{
    return static_cast<float>(u.x*v.x + u.y*v.y + u.z*v.z);
}

vec3::vec3(double a, double b, double c)
{
    x = a;
    y = b;
    z = c;
}

vec3 vec3::operator+(vec3 &q)
{
    return vec3(x + q.x, y + q.y, z + q.z);
}

// Diferencia de vectores
vec3 vec3::operator-(vec3 &q)
{
    return vec3(x - q.x, y - q.y, z - q.z);
}

// escalar x vector
vec3 vec3::operator*(float k)
{
    return vec3(k*x, k*y, k*z);
}

float vec3::length()
{
    return static_cast<float>(sqrt(fabs(x*x + y*y + z*z)));
}

void vec3::normalize()
{
    auto m = length();
    if (fabs(m) > 0.001)
    {
        x /= m;
        y /= m;
        z /= m;
    }
}

void vec3::rotar_xz(float an)
{
    x = static_cast<float>(x*cos(an) - z*sin(an));
    z = static_cast<float>(x*sin(an) + z*cos(an));
}

void vec3::rotar_xz(vec3 O, float an)
{
    *this = *this - O;
    x = static_cast<float>(x*cos(an) - z*sin(an));
    z = static_cast<float>(x*sin(an) + z*cos(an));
    *this = *this + O;
}

void vec3::rotar(vec3 o, vec3 eje, float theta)
{
    auto a = static_cast<float>(o.x);
    auto b = static_cast<float>(o.y);
    auto c = static_cast<float>(o.z);
    auto u = static_cast<float>(eje.x);
    auto v = static_cast<float>(eje.y);
    auto w = static_cast<float>(eje.z);

    auto u2 = u*u;
    auto v2 = v*v;
    auto w2 = w*w;
    auto cosT = static_cast<float>(cos(theta));
    auto sinT = static_cast<float>(sin(theta));
    auto l2 = u2 + v2 + w2;
    auto l = static_cast<float>(sqrt(l2));

    // El vector de rotacion es casi nulo
    if (l2 < 0.000000001)
    {
        return;
    }

    auto xr = static_cast<float>(a*(v2 + w2) + u*(-b*v - c*w + u*x + v*y + w*z)
        + (-a*(v2 + w2) + u*(b*v + c*w - v*y - w*z) + (v2 + w2)*x)*cosT
        + l*(-c*v + b*w - w*y + v*z)*sinT);
    xr /= l2;

    auto yr = static_cast<float>(b*(u2 + w2) + v*(-a*u - c*w + u*x + v*y + w*z)
        + (-b*(u2 + w2) + v*(a*u + c*w - u*x - w*z) + (u2 + w2)*y)*cosT
        + l*(c*u - a*w + w*x - u*z)*sinT);
    yr /= l2;

    auto zr = static_cast<float>(c*(u2 + v2) + w*(-a*u - b*v + u*x + v*y + w*z)
        + (-c*(u2 + v2) + w*(a*u + b*v - u*x - v*y) + (u2 + v2)*z)*cosT
        + l*(-b*u + a*v - v*x + u*y)*sinT);
    zr /= l2;

    x = xr;
    y = yr;
    z = zr;
}