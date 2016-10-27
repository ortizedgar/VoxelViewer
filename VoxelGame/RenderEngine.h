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
    void TextureVR();
    void Release();
    void setShaders();
    void loadShaders(char *vs, char *fs, GLhandleARB *vs_main, GLhandleARB *fs_main, GLhandleARB *shader_prog);
    void initFonts();
    void renderText(int px, int py, char *text);
    void renderText(float k, int px, int py, char *text);
    void renderCircle(int px, int py, int r);
    void renderRect(int px0, int py0, int dx, int d1);
    void renderHUD();
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
    int filtro;
    float mr, mg, mb;
    bool target_hit;
    float vel_tras;
    int game_status;
    float timer_catch;
    int cant_capturados;
    GLint fbWidth;
    GLint fbHeight;
    GLhandleARB vs_main, fs_main, shader_prog;
    GLhandleARB vs2_main, fs2_main, shader_prog2;

    // Fonts
    Glyph glyphs[255];

private:
    void CheckCompilationStatus(GLhandleARB * vs_main);
    char *textFileRead(char *fn);
    bool CheckTargetHit();
    void CheckObjetivoEnLaMira();
    void FireWeapon();
};