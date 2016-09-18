#include "vec3.h"

class mat4
{
public:
    double m[16];
    mat4(double a11 = 0, double a12 = 0, double a13 = 0, double a14 = 0,
        double a21 = 0, double a22 = 0, double a23 = 0, double a24 = 0,
        double a31 = 0, double a32 = 0, double a33 = 0, double a34 = 0,
        double a41 = 0, double a42 = 0, double a43 = 0, double a44 = 0);
    static mat4 RotateX(double an);
    static mat4 RotateY(double an);
    static mat4 RotateZ(double an);
    static mat4 fromBase(vec3 N, vec3 U, vec3 V);
    mat4 operator*(mat4 B);
};