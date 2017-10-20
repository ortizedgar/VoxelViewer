#pragma once

#include "Texture.h"
#include "../GL/glew.h"
#include "vec3.h"
#include "Glyph.h"

class RenderEngine
{
public:
    RenderEngine();
    virtual ~RenderEngine();
    bool Initialize(HDC hContext_i);
    void Resize(int nWidth_i, int nHeight_i);
    void Render();
    void RayCasting();
    void RayCasting2();
    void TextureVR();
    void Release();
    void setShaders();
    void loadShaders(char *vs, char *fs, GLhandleARB *vs_main, GLhandleARB *fs_main, GLhandleARB *shader_prog);
    void initFonts();
    void renderText(int px, int py, char *text);
    void renderText(float k, int px, int py, char *text);
    void renderCircle(int px, int py, int r);
    void renderGradientRect(int px0, int py0, int dx, int dy);
    void renderGradientRoundRect(int px0, int py0, int dx, int dy, int prx, int pry);
    void renderGradientRoundRect2(int px0, int py0, int dx, int dy, int prx, int pry);
    void renderGradientRoundRect3(int px0, int py0, int dx, int dy, int prx, int pry);
    void renderGradientLine(int px0, int py0, int px1, int py1);
    void renderLine(int px0, int py0, int px1, int py1, BYTE r, BYTE g, BYTE b);
    void renderRect(int px0, int py0, int dx, int d1);
    void renderHUD();
    void setDemoMode(bool demoMode) { this->_demoMode = demoMode; }
    void setTotalFrames(int totalFrames) { this->_totalFrames = totalFrames; }
    HDC m_hDC;
    HGLRC m_hglContext;
    Texture tex;
    float fps;
    float time;
    float elapsed_time;
    vec3 lookFrom;
    vec3 viewDir;
    vec3 U, V;
    float voxel_step0;
    float voxel_step;
    float cant_total;
    int filtro;
    float mr, mg, mb;
    bool target_hit;
    float vel_tras;
    float max_vel_tras;
    int game_status;
    int game_stage;
    float timer_catch;
    float timer_fire;
    int cant_capturados;
    GLint fbWidth;
    GLint fbHeight;
    bool modo_aceleracion;
    bool modo_visionX;
    float E;        // escala global

    // Fonts
    Glyph glyphs[255];

    // imagen presentacion
    Texture2d pres, hud, fondo, fire;

    // ecg
    int cant_muestras;
    float ECG[65535];
    void initECG();

private:
    void CheckCompilationStatus(GLhandleARB * vs_main);
    char *textFileRead(char *fn);
    bool CheckTargetHit();
    void CheckObjetivoEnLaMira();
    void FireWeapon();
    void RenderTitleText(unsigned int xPosition, int *yPosition, char * mensaje);
    void RenderStartText(unsigned int xPosition, int *yPosition, char * mensaje);
    void RenderExplanationText(unsigned int xPosition, int *yPosition, char * mensaje);
    void RenderEndScreen();
    void RenderGame();
    void RenderStartScreen();
    void RenderFullScreenQuad(int texId);
    void RenderQuad(Texture2d *tx, int px, int py, float an = 0, float k = 1);
    GLhandleARB _vertexShaderRayCasting, _fragmentShaderRayCasting, _rayCastingShaderProgram;
    GLhandleARB _vertexShaderRayCasting2, _fragmentShaderRayCasting2, _rayCastingShaderProgram2;
    GLhandleARB _vertexShaderTextureVR, _fragmentShaderTextureVR, _textureVRShaderProgram;
    GLhandleARB _vertexShaderFixed, _fragmentShaderFixed, _fixedShaderProgram;

    bool _demoMode;
    int _totalFrames;
};