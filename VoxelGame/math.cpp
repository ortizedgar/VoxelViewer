#include "StdAfx.h"
#include "math.h"
#include "vec3.h"

auto math::swap(float *a, float *b)
{
    auto c = *a;
    *a = *b;
    *b = c;
}

// Helper interseccion rayo - esfera
auto math::intersect(vec3 orig, vec3 dir, vec3 center, float radio)
{
    auto *vec3Aux = new vec3;
    auto L = orig - center;
    auto B = 2 * vec3Aux->dot(dir, L);
    auto C = vec3Aux->dot(L, L) - radio*radio;
    auto disc = B*B - 4 * C;
    if (disc >= 0)
    {
        auto t0 = (-B - static_cast<float>(sqrt(disc))) / 2;
        auto t1 = (-B + static_cast<float>(sqrt(disc))) / 2;
        if (t0 > t1)
        {
            this->swap(&t0, &t1);
        }

        // t0 < t1 
        if (fabs(t0 - 15) < 1 || fabs(t1 - 15) < 1)
        {
            return true;
        }
    }

    return false;
}

int math::clamp(int x, int a, int b)
{
    if (x < a)
    {
        return a;
    }
    else
    {
        if (x > b)
        {
            return b;
        }
        else
        {
            return x;
        }
    }
}