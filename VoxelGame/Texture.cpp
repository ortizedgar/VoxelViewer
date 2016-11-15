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
    this->_anomalies = 100;
    this->_anomalieRadius = 8;
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

    // Se agregan anomalias
    auto _anomaliesPositions = new int*[_anomalies];
    auto dimensiones = 3, x = 0, y = 0, z = 0;
    for (auto i = 0; i < this->_anomalies; ++i)
    {
        _anomaliesPositions[i] = new int[dimensiones];
        for (auto j = 0; j < dimensiones; j++)
        {
            _anomaliesPositions[i][j] = static_cast<int>((float)rand() / (float)RAND_MAX*200.0f + 20);
        }

        x = _anomaliesPositions[i][0];
        y = _anomaliesPositions[i][1];
        z = _anomaliesPositions[i][2];
        this->Ellipsoid(pRGBABuffer, x - this->_anomalieRadius, y - this->_anomalieRadius, z - this->_anomalieRadius, x + this->_anomalieRadius, y + this->_anomalieRadius, z + this->_anomalieRadius);
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


Texture2d::Texture2d()
{
	id = dx = dy = 0;
}

Texture2d::~Texture2d()
{
	if (id != 0)
	{
		glDeleteTextures(1, (GLuint*)&id);
	}
}

bool Texture2d::CreateFromFile(LPCTSTR lpDataFile_i, bool color_key)
{
	FILE *file;
	BYTE header[54];
	unsigned int dataPos;
	unsigned int size;
	BYTE *data , *rgba;

	USES_CONVERSION;
	file = fopen(W2A(lpDataFile_i), "rb");

	if (file == NULL)
	{
		return false;
	}

	if (fread(header, 1, 54, file) != 54)
	{
		return false;
	}

	if (header[0] != 'B' || header[1] != 'M')
	{
		return false;
	}

	dataPos = *(int*)&(header[0x0A]);
	size = *(int*)&(header[0x22]);
	dx = *(int*)&(header[0x12]);
	dy = *(int*)&(header[0x16]);

	if (size == NULL)
		size = dx * dy * 3;
	if (dataPos == NULL)
		dataPos = 54;

	data = new BYTE[size];
	rgba = new BYTE[dx * dy * 4 ];

	fread(data, 1, size, file);

	// Transformo de BGR a RGBA 
	int l = dx * dy;
	int t = 0;
	for (int i = 0; t<l; ++i , ++t)
	{
		BYTE b = data[i * 3 + 0];
		BYTE g = data[i * 3 + 1];
		BYTE r = data[i * 3 + 2];

		if (color_key)		// usar color key
		{
			if(b < 5 && g < 5 && r < 5)
				rgba[t * 4 + 3] =0;		// color key
			else
			{
				// y la hago monocroma
				rgba[t * 4 + 0] = 255;		// R
				rgba[t * 4 + 1] = 255;		// G
				rgba[t * 4 + 2] = 255;		// B

				rgba[t * 4 + 3] = 255;		// A
			}

		}
		else
		{
			rgba[t * 4 + 0] = r;
			rgba[t * 4 + 1] = g;
			rgba[t * 4 + 2] = b;
			rgba[t * 4 + 3] = 255;

		}

		int col = i%dx;
		if (col == dx - 1)		// el ultimo de la linea
		{
			int resto;
			if ((resto = (col % 4))>0 && resto<4)
				i += (4 - resto) - 1;
		}

	}

	fclose(file);
	glGenTextures(1, (GLuint*)&id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dx, dy, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

	delete[] data;
	delete[] rgba;
	return true;
}