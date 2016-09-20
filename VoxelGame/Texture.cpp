#include "StdAfx.h"
#include "Texture.h"
#include "../GL/glew.h"
#include "math.h"

#include <gl/GL.h>

Texture::Texture()
{
    this->dx(0);
    this->dy(0);
    this->dz(0);
    this->id(0);
}

Texture::~Texture()
{
    if (this->id() != 0)
    {
        glDeleteTextures(1, (GLuint*)&this->_id);
    }
}

bool Texture::CreateFromFile(LPCTSTR lpDataFile_i, int nWidth_i, int nHeight_i, int nSlices_i)
{
    CFile Medfile;
    if (!Medfile.Open(lpDataFile_i, CFile::modeRead))
    {
        return false;
    }

    this->dx(nWidth_i);
    this->dy(nHeight_i);
    this->dz(nSlices_i);

    int size = this->dx()*this->dy()*this->dz();
    BYTE* chBuffer = new BYTE[2 * size];
    if (!chBuffer)
    {
        return false;
    }

    char* pRGBABuffer = new char[size * 4];
    if (!pRGBABuffer)
    {
        return false;
    }

    memset(chBuffer, 0, size);
    Medfile.Read(chBuffer, size);

    for (int nIndx = 0; nIndx < size; ++nIndx)
    {
        pRGBABuffer[nIndx * 4] = chBuffer[nIndx];
        pRGBABuffer[nIndx * 4 + 1] = chBuffer[nIndx];
        pRGBABuffer[nIndx * 4 + 2] = chBuffer[nIndx];
        pRGBABuffer[nIndx * 4 + 3] = chBuffer[nIndx];
    }

    // experimento, le pongo una caja roja
    for (int i = 0; i < 20; ++i)
    {
        int r = 8;
        int x = static_cast<int>((float)rand() / (float)RAND_MAX*200.0f + 20);
        int y = static_cast<int>((float)rand() / (float)RAND_MAX*200.0f + 20);
        int z = static_cast<int>((float)rand() / (float)RAND_MAX*200.0f + 20);
        Ellipsoid(pRGBABuffer, x - r, y - r, z - r, x + r, y + r, z + r);
    }

    if (0 != this->id())
    {
        glDeleteTextures(1, (GLuint*)&this->_id);
    }

    glGenTextures(1, (GLuint*)&this->_id);

    glBindTexture(GL_TEXTURE_3D, this->id());
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //GL_UNSIGNED_SHORT_4_4_4_4
    //GL_UNSIGNED_BYTE
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA4, this->dx(), this->dy(), this->dz(), 0, GL_RGBA, GL_UNSIGNED_BYTE, pRGBABuffer);
    glBindTexture(GL_TEXTURE_3D, 0);

    delete[] chBuffer;
    delete[] pRGBABuffer;
    return true;
}

void Texture::Box2(char *buff, int x0, int y0, int z0, int x1, int y1, int z1)
{
    auto mathAux = new math;
    x0 = mathAux->clamp(x0, 0, 255);
    y0 = mathAux->clamp(y0, 0, 255);
    z0 = mathAux->clamp(z0, 0, 255);
    x1 = mathAux->clamp(x1, 0, 255);
    y1 = mathAux->clamp(y1, 0, 255);
    z1 = mathAux->clamp(z1, 0, 255);

    for (int x = x0; x < x1; ++x)
    {
        for (int y = y0; y < y1; ++y)
        {
            for (int z = z0; z < z1; ++z)
            {
                buff[(z*this->dx()*this->dy() + y*this->dx() + x) * 4] = (char)255;
                buff[(z*this->dx()*this->dy() + y*this->dx() + x) * 4 + 1] = 0;
                buff[(z*this->dx()*this->dy() + y*this->dx() + x) * 4 + 2] = 0;
            }
        }
    }
}

void Texture::Ellipsoid(char *buff, int x0, int y0, int z0, int x1, int y1, int z1)
{
    auto mathAux = new math;
    x0 = mathAux->clamp(x0, 0, 255);
    y0 = mathAux->clamp(y0, 0, 255);
    z0 = mathAux->clamp(z0, 0, 255);
    x1 = mathAux->clamp(x1, 0, 255);
    y1 = mathAux->clamp(y1, 0, 255);
    z1 = mathAux->clamp(z1, 0, 255);

    int cx = (x1 + x0) / 2;
    int cy = (y1 + y0) / 2;
    int cz = (z1 + z0) / 2;

    int rx = (x1 - x0) / 2;
    int ry = (y1 - y0) / 2;
    int rz = (z1 - z0) / 2;

    float rx2 = static_cast<float>(rx*rx);
    float ry2 = static_cast<float>(ry*ry);
    float rz2 = static_cast<float>(rz*rz);

    for (int x = x0; x < x1; ++x)
    {
        for (int y = y0; y < y1; ++y)
        {
            for (int z = z0; z < z1; ++z)
            {
                float sx = (float)(x - cx);
                float sy = (float)(y - cy);
                float sz = (float)(z - cz);

                float tx = sx*sx / rx2;
                float ty = sy*sy / ry2;
                float tz = sz*sz / rz2;

                float k = tx + ty + tz;
                if (k <= 1)
                {
                    int ndx = (z*this->dx()*this->dy() + y*this->dx() + x) * 4;
                    //buff[ndx] = clamp ( buff[ndx] + 255*(1-k) , 0, 255);
                    buff[ndx] = (char)255;
                }
            }
        }
    }
}

void Texture::Box(BYTE *buff, int x0, int y0, int z0, int x1, int y1, int z1)
{
    for (int x = x0; x < x1; ++x)
    {
        for (int y = y0; y < y1; ++y)
        {
            for (int z = z0; z < z1; ++z)
            {
                buff[z*this->dx()*this->dy() + y*this->dx() + x] = 255;
            }
        }
    }
}

bool Texture::CreateFromTest(int n, int nWidth_i, int nHeight_i, int nSlices_i)
{
    this->dx(nWidth_i);
    this->dy(nHeight_i);
    this->dz(nSlices_i);

    int size = this->dx()*this->dy()*this->dz();
    BYTE * chBuffer = new BYTE[size];
    if (!chBuffer)
    {
        return false;
    }

    char* pRGBABuffer = new char[size * 4];
    if (!pRGBABuffer)
    {
        return false;
    }

    memset(chBuffer, 0, size);

    // Creo una caja
    if (n == 0)
    {
        Box(chBuffer, 0, 0, 0, this->dx(), this->dy(), 10);
        Box(chBuffer, 0, 0, this->dz() - 10, this->dx(), this->dy(), this->dz());
    }
    else
    {
        Box(chBuffer, this->dx() / 2 - 10, this->dy() / 2 - 10, this->dz() / 2 - 10, this->dx() / 2 + 10, this->dy() / 2 + 10, this->dz() / 2 + 10);
    }

    for (int nIndx = 0; nIndx < size; ++nIndx)
    {
        pRGBABuffer[nIndx * 4] = chBuffer[nIndx];
        pRGBABuffer[nIndx * 4 + 1] = chBuffer[nIndx];
        pRGBABuffer[nIndx * 4 + 2] = chBuffer[nIndx];
        pRGBABuffer[nIndx * 4 + 3] = chBuffer[nIndx];
    }

    if (0 != this->id())
    {
        glDeleteTextures(1, (GLuint*)&this->_id);
    }

    glGenTextures(1, (GLuint*)&this->_id);

    glBindTexture(GL_TEXTURE_3D, this->id());
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, this->dx(), this->dy(), this->dz(), 0, GL_RGBA, GL_UNSIGNED_BYTE, pRGBABuffer);
    glBindTexture(GL_TEXTURE_3D, 0);

    delete[] chBuffer;
    delete[] pRGBABuffer;
    return true;
}