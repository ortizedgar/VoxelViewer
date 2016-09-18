#pragma once

class vec3
{
public:
    double x, y, z;
    vec3(double a = 0, double b = 0, double c = 0);
    vec3 operator-(vec3 &q);
    vec3 operator+(vec3 &q);
    vec3 operator*(float k);
    float length();
    void normalize();
    void rotar_xz(float an);
    void rotar_xz(vec3 O, float an);
    void rotar(vec3 o, vec3 eje, float theta);

    vec3 cross(vec3 u, vec3 v);
    float dot(vec3 u, vec3 v);
};