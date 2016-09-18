#pragma once

#include "vec3.h"

#define M_PI        3.14159265359f
#define M_PI_2      1.570796326795f
#define M_2PI       6.28318530718f

class math
{
public:
    bool intersect(vec3 orig, vec3 dir, vec3 center, float radio);
    void swap(float *a, float *b);
    int clamp(int x, int a, int b);
};