#pragma once

class Texture
{
public:
    Texture();
    virtual ~Texture();
    bool CreateFromFile(LPCTSTR lpDataFile_i, int nWidth_i, int nHeight_i, int nSlices_i);
    bool CreateFromTest(int n, int nWidth_i, int nHeight_i, int nSlices_i);
    
    // Sobre el RAW
    void Box(BYTE *buff, int x0, int y0, int z0, int x1, int y1, int z1);

    // Sobre el RGBA
    void Box2(char *buff, int x0, int y0, int z0, int x1, int y1, int z1);
    void Ellipsoid(char *buff, int x0, int y0, int z0, int x1, int y1, int z1);

    int dx, dy, dz, id;
};