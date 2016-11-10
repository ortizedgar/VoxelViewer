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

    // Getters and Setter
    auto dx() { return this->_dx; }
    auto dx(int dx) { this->_dx = dx; }
    auto dy() { return this->_dy; }
    auto dy(int dy) { this->_dy = dy; }
    auto dz() { return this->_dz; }
    auto dz(int dz) { this->_dz = dz; }
    auto id() { return this->_id; }
    auto id(int id) { this->_id = id; }
    auto AnomalieRadius() { return this->_anomalieRadius; }
    auto Anomalies() { return this->_anomalies; }
    auto Anomalies(int anomalies) { this->_anomalies = anomalies; }

private:
    int _dx, _dy, _dz, _id, _anomalies, _anomalieRadius;
};


class Texture2d
{
public:
	Texture2d();
	virtual ~Texture2d();
	bool CreateFromFile(LPCTSTR lpDataFile_i);
	int dx, dy;
	int id;
};