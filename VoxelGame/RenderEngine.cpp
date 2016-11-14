#include "StdAfx.h"
#include "RenderEngine.h"
#include "mat4.h"
#include "vec4.h"
#include "math.h"

#include <memory>

#pragma comment ( lib, "OpenGL32.lib" )
#pragma comment ( lib, "glew32.lib" )

RenderEngine::RenderEngine()
{
    game_status = 0;
    game_stage = 0;
    fps = 0;
    viewDir = vec3(1, 0, 0);
    U = vec3(0, 1, 0);
    V = vec3(0, 0, 1);
    timer_catch = 0;
    voxel_step = 0.5;
    voxel_step0 = 75.0;
    vel_tras = 20;
	max_vel_tras = 80;
    lookFrom = vec3(-80, 0, 0);
    filtro = 0;
    cant_capturados = 0;
	modo_aceleracion = true;
	modo_visionX = false;
}

RenderEngine::~RenderEngine()
{
}

bool RenderEngine::Initialize(HDC hContext_i)
{
    m_hDC = hContext_i;

    // Setting up the dialog to support the OpenGL.
    PIXELFORMATDESCRIPTOR stPixelFormatDescriptor;
    memset(&stPixelFormatDescriptor, 0, sizeof(PIXELFORMATDESCRIPTOR));
    stPixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    stPixelFormatDescriptor.nVersion = 1;
    stPixelFormatDescriptor.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    stPixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
    stPixelFormatDescriptor.cColorBits = 24;
    stPixelFormatDescriptor.cDepthBits = 32;
    stPixelFormatDescriptor.cStencilBits = 8;
    stPixelFormatDescriptor.iLayerType = PFD_MAIN_PLANE;

    // Collect the pixel format.
    auto nPixelFormat = ChoosePixelFormat(hContext_i, &stPixelFormatDescriptor);
    if (nPixelFormat == 0)
    {
        AfxMessageBox(_T("Error while Choosing Pixel format"));
        return false;
    }

    // Set the pixel format 
    if (!SetPixelFormat(hContext_i, nPixelFormat, &stPixelFormatDescriptor))
    {
        AfxMessageBox(_T("Error while setting pixel format"));
        return false;
    }

    // Create a device context
    m_hglContext = wglCreateContext(hContext_i);
    if (!m_hglContext)
    {
        AfxMessageBox(_T("Rendering Context Creation Failed"));
        return false;
    }

    // Make the created device context as the current device context.
    BOOL bResult = wglMakeCurrent(hContext_i, m_hglContext);
    if (!bResult)
    {
        AfxMessageBox(_T("wglMakeCurrent Failed"));
        return false;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Inicializa las texturas 3d, para eso uso el wrapper de Opengl Glew
    glewInit();
    if (GL_TRUE != glewGetExtension("GL_EXT_texture3D"))
    {
        AfxMessageBox(_T("3D texture is not supported !"));
        return false;
    }

    // Pongo los shaders
    this->setShaders();

    // Inicio el sistema de fonts simples
    this->initFonts();

    // if( !tex.CreateFromFile( "media/bonsai.raw", 256, 256,256))
    if (!tex.CreateFromFile(_T("../media/mri-head.raw"), 256, 256, 256))
    {
        AfxMessageBox(_T("Failed to read the data"));
    }

    /*
    if( !tex.CreateFromTest( 1, 256, 256,256))
    {
        AfxMessageBox( _T( "Failed to read the data" ));
    }*/

	// Imagen presentacion
	if (!pres.CreateFromFile(_T("../media/pres.bmp")))
	{
		AfxMessageBox(_T("Failed to read pres.bmp"));
	}

	// Imagen de fondo
	if (!fondo.CreateFromFile(_T("../media/pres_fondo.bmp")))
	{
		AfxMessageBox(_T("Failed to read pres_fondo.bmp"));
	}

	// Imagen HUD
	if (!hud.CreateFromFile(_T("../media/hud.bmp") , true))
	{
		AfxMessageBox(_T("Failed to read hud.bmp"));
	}

    GLint dims[4] = { 0 };
    glGetIntegerv(GL_VIEWPORT, dims);
    fbWidth = dims[2];
    fbHeight = dims[3];
	float ex = (float)fbWidth / 1276.;
	float ey = (float)fbHeight / 734.;
	E = min(ex, ey);

	initECG();


    return true;
}

void RenderEngine::initFonts()
{
    CDC *pDC = CDC::FromHandle(m_hDC);
    CFont hfont, *hfontOld;
    hfont.CreateFont(48, 13, 0, 0, FW_EXTRALIGHT, 0, 0, 0, 0, OUT_TT_PRECIS, 0, CLEARTYPE_QUALITY, FIXED_PITCH || FF_ROMAN, NULL);
    hfontOld = pDC->SelectObject(&hfont);
    char s[1];
    auto nNumPoints = 0;
    for (auto i = 0; i < 255; ++i)
    {
        s[0] = i;
        pDC->SetBkMode(TRANSPARENT);
        pDC->BeginPath();
        TextOutA(pDC->m_hDC, 0, 0, s, 1);
        pDC->EndPath();
        pDC->FlattenPath();
        nNumPoints = pDC->GetPath((LPPOINT)NULL, (LPBYTE)NULL, 0);
        if (nNumPoints > 0)
        {
            glyphs[i].lpPt = new POINT[nNumPoints];
            glyphs[i].lpB = new BYTE[nNumPoints];
            glyphs[i].nNumPoints = pDC->GetPath(glyphs[i].lpPt, glyphs[i].lpB, nNumPoints);
        }
    }

    pDC->SelectObject(hfontOld);
}

void RenderEngine::Resize(int nWidth_i, int nHeight_i)
{
    glViewport(0, 0, nWidth_i, nHeight_i);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    auto AspectRatio = (GLdouble)(nWidth_i) / (GLdouble)(nHeight_i);
    if (nWidth_i <= nHeight_i)
    {
        glOrtho(-1, 1, -(1 / AspectRatio), 1 / AspectRatio, 2.0f*-1, 2.0f * 1);
    }
    else
    {
        glOrtho(-1 * AspectRatio, 1 * AspectRatio, -1, 1, 2.0f*-1, 2.0f * 1);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void RenderEngine::Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (tex.id() == 0)
    {
        this->renderText(10, 510, "Sin archivo cargado");
    }
    else
    {
        if (this->game_stage == 0)
        {
            this->RenderStartScreen();
        }

        if (this->game_stage == 1)
        {
            this->RenderGame();
        }

        if (this->game_stage == 2)
        {
            glDisable(GL_ALPHA_TEST);
            glDisable(GL_BLEND);
            this->RenderEndScreen();
        }
    }

    SwapBuffers(m_hDC);
}

void RenderEngine::RenderEndScreen()
{

	RenderFullScreenQuad(fondo.id);
	glLineWidth(5);
	glColor3ub(255, 255, 255);
	renderText(0.003f, 50, 50, "Game over");

	char saux[255];
	sprintf(saux, "Score: %d", cant_capturados * 100);
	glColor3ub(240, 240, 240);
	renderText(0.002f, 50, 100, saux);
	glLineWidth(1);
	glColor3ub(255,255,255);
	renderText(0.002f, 50, 100, saux);


	glLineWidth(1);
	glColor3ub(240, 240, 240);
	renderText(0.001f, 50, fbHeight - 100, "Realtime Voxel Rendering - ACyT UADE");
	renderText(0.001f, 50, fbHeight - 70, "Mariano Banquiero");
	renderText(0.001f, 50, fbHeight - 40, "Edgar Ortiz");


}

void RenderEngine::RenderGame()
{
	if(modo_visionX)
		RayCasting2();
	else
		RayCasting();
	TextureVR();
}

void RenderEngine::RenderStartScreen()
{

	RenderFullScreenQuad(pres.id );

	/*
	auto xPosition = this->fbWidth / 2;
	auto yPosition = this->fbHeight / 8;
	char *mensaje = "Vexplorer!";
	this->RenderTitleText(xPosition / 2, &yPosition, mensaje);
	mensaje = "Bienvenido a Vexplorer!";
	this->RenderExplanationText(xPosition / 8, &yPosition, mensaje);
	mensaje = "Un videojuego de exploracion 3D. El objetivo es";
	this->RenderExplanationText(xPosition / 8, &yPosition, mensaje);
	mensaje = "eliminar todas las esferas rojas posibles";
	this->RenderExplanationText(xPosition / 8, &yPosition, mensaje);
	mensaje = "antes que se agote el tiempo.";
	this->RenderExplanationText(xPosition / 8, &yPosition, mensaje);
	mensaje = "Controles:";
	this->RenderExplanationText(xPosition / 8, &yPosition, mensaje);
	mensaje = "W - Avanzar";
	this->RenderExplanationText(xPosition / 8, &yPosition, mensaje);
	mensaje = "S - Retroceder";
	this->RenderExplanationText(xPosition / 8, &yPosition, mensaje);
	mensaje = "Mouse - Mirar";
	this->RenderExplanationText(xPosition / 8, &yPosition, mensaje);
	mensaje = "Boton izquierdo - Disparar arma";
	this->RenderExplanationText(xPosition / 8, &yPosition, mensaje);
	mensaje = "F - Encender/Apagar filtro";
	this->RenderExplanationText(xPosition / 8, &yPosition, mensaje);
	mensaje = "Presione cualquier tecla para comenzar!";
	this->RenderStartText(xPosition / 8, &yPosition, mensaje);
	*/

}

void RenderEngine::RenderTitleText(unsigned int xPosition, int *yPosition, char * mensaje)
{
    glLineWidth(0);
    glColor4f(1.f, 0.0f, 0.f, 1.f);
    this->renderText(0.0017f, xPosition, *yPosition, mensaje);
    *yPosition += 50;
}

void RenderEngine::RenderStartText(unsigned int xPosition, int *yPosition, char * mensaje)
{
    glLineWidth(0);
    glColor4f(1.f, 1.0f, 0.f, 1.f);
    this->renderText(0.0017f, xPosition, *yPosition, mensaje);
    *yPosition += 50;
}

void RenderEngine::RenderExplanationText(unsigned int xPosition, int *yPosition, char * mensaje)
{
    glLineWidth(0);
    glColor4f(1.f, 1.0f, 1.f, 1.f);
    this->renderText(0.0017f, xPosition, *yPosition, mensaje);
    *yPosition += 50;
}

// Version con RayCasting
void RenderEngine::RayCasting()
{
    float fov = M_PI / 4.0f;
    float DX = static_cast<float>(fbWidth);
    float DY = static_cast<float>(fbHeight);
    float k = 2 * static_cast<float>(tan(fov / 2));
    vec3 Dy = U * (k*DY / DX);
    vec3 Dx = V * k;

    // Direccion de cada rayo
    // D = N + Dy*y + Dx*x;
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glEnable(GL_TEXTURE_3D);

    glUseProgramObjectARB(this->_rayCastingShaderProgram);

    glUniform3f(glGetUniformLocation(this->_rayCastingShaderProgram, "iLookFrom"), static_cast<float>(lookFrom.x), static_cast<float>(lookFrom.y), static_cast<float>(lookFrom.z));
    glUniform3f(glGetUniformLocation(this->_rayCastingShaderProgram, "iViewDir"), static_cast<float>(viewDir.x), static_cast<float>(viewDir.y), static_cast<float>(viewDir.z));
    glUniform3f(glGetUniformLocation(this->_rayCastingShaderProgram, "iDx"), static_cast<float>(Dx.x), static_cast<float>(Dx.y), static_cast<float>(Dx.z));
    glUniform3f(glGetUniformLocation(this->_rayCastingShaderProgram, "iDy"), static_cast<float>(Dy.x), static_cast<float>(Dy.y), static_cast<float>(Dy.z));
    glUniform1f(glGetUniformLocation(this->_rayCastingShaderProgram, "voxel_step"), voxel_step);
    glUniform1f(glGetUniformLocation(this->_rayCastingShaderProgram, "voxel_step0"), voxel_step0);
    glUniform1i(glGetUniformLocation(this->_rayCastingShaderProgram, "game_status"), game_status);
    glUniform1f(glGetUniformLocation(this->_rayCastingShaderProgram, "time"), time);
    glUniform1i(glGetUniformLocation(this->_rayCastingShaderProgram, "filter"), filtro);

    glActiveTexture(GL_TEXTURE);
    glBindTexture(GL_TEXTURE_3D, tex.id());

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glBegin(GL_QUADS);

    auto dx = 0.0f;
    auto dy = 0.0f;
    glTexCoord2f(-1, -1);
    glVertex3f(-1, -1, 0);

    glTexCoord2f(1, -1);
    glVertex3f(1 - dx, -1, 0);

    glTexCoord2f(1, 1);
    glVertex3f(1 - dx, 1 - dy, 0);

    glTexCoord2f(-1, 1);
    glVertex3f(-1, 1 - dy, 0);

    glEnd();
    glUseProgramObjectARB(0);
    glLoadIdentity();
    glDisable(GL_TEXTURE_3D);

    char saux[1024];
    if (0)
    {
        sprintf_s(saux, "Ray Casting fps = %.1f", fps);
        renderText(10, 10, saux);
        sprintf_s(saux, "step= %.3f  step0=%.3f", voxel_step, voxel_step0);
        renderText(10, 30, saux);
        sprintf_s(saux, "LookFrom= (%d,%d,%d)", (int)lookFrom.x, (int)lookFrom.y, (int)lookFrom.z);
        renderText(10, 50, saux);
        //sprintf_s(saux,"ViewDir =(%.3f,%.3f,%.3f)  UP=(%.3f,%.3f,%.3f",viewDir.x,viewDir.y,viewDir.z ,U.x,U.y,U.z);
        //renderText(10,50,saux);
    }
    this->target_hit = false;

    // Verifico si pasa sobre un objetivo
    this->CheckObjetivoEnLaMira();

    this->renderHUD();

    if (timer_catch > 0)
    {
        timer_catch -= elapsed_time;
        if (timer_catch <= 0)
        {
            timer_catch = 0;
            game_status = 0;
        }
    }

    // Eliminar objetivo
    this->FireWeapon();
}



// modo vision X
void RenderEngine::RayCasting2()
{
	float fov = M_PI / 4.0f;
	float DX = static_cast<float>(fbWidth);
	float DY = static_cast<float>(fbHeight);
	float k = 2 * static_cast<float>(tan(fov / 2));
	vec3 Dy = U * (k*DY / DX);
	vec3 Dx = V * k;

	// Direccion de cada rayo
	// D = N + Dy*y + Dx*x;
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glEnable(GL_TEXTURE_3D);

	glUseProgramObjectARB(this->_rayCastingShaderProgram2);

	glUniform3f(glGetUniformLocation(this->_rayCastingShaderProgram2, "iLookFrom"), static_cast<float>(lookFrom.x), static_cast<float>(lookFrom.y), static_cast<float>(lookFrom.z));
	glUniform3f(glGetUniformLocation(this->_rayCastingShaderProgram2, "iViewDir"), static_cast<float>(viewDir.x), static_cast<float>(viewDir.y), static_cast<float>(viewDir.z));
	glUniform3f(glGetUniformLocation(this->_rayCastingShaderProgram2, "iDx"), static_cast<float>(Dx.x), static_cast<float>(Dx.y), static_cast<float>(Dx.z));
	glUniform3f(glGetUniformLocation(this->_rayCastingShaderProgram2, "iDy"), static_cast<float>(Dy.x), static_cast<float>(Dy.y), static_cast<float>(Dy.z));
	glUniform1f(glGetUniformLocation(this->_rayCastingShaderProgram2, "voxel_step"), voxel_step);
	glUniform1f(glGetUniformLocation(this->_rayCastingShaderProgram2, "voxel_step0"), voxel_step0);
	glUniform1i(glGetUniformLocation(this->_rayCastingShaderProgram2, "game_status"), game_status);
	glUniform1f(glGetUniformLocation(this->_rayCastingShaderProgram2, "time"), time);
	glUniform1i(glGetUniformLocation(this->_rayCastingShaderProgram2, "filter"), filtro);

	glActiveTexture(GL_TEXTURE);
	glBindTexture(GL_TEXTURE_3D, tex.id());

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

	glBegin(GL_QUADS);

	auto dx = 0.0f;
	auto dy = 0.0f;
	glTexCoord2f(-1, -1);
	glVertex3f(-1, -1, 0);

	glTexCoord2f(1, -1);
	glVertex3f(1 - dx, -1, 0);

	glTexCoord2f(1, 1);
	glVertex3f(1 - dx, 1 - dy, 0);

	glTexCoord2f(-1, 1);
	glVertex3f(-1, 1 - dy, 0);

	glEnd();
	glUseProgramObjectARB(0);
	glLoadIdentity();
	glDisable(GL_TEXTURE_3D);

	char saux[1024];
	this->target_hit = false;

	// Verifico si pasa sobre un objetivo
	this->CheckObjetivoEnLaMira();

	this->renderHUD();

	if (timer_catch > 0)
	{
		timer_catch -= elapsed_time;
		if (timer_catch <= 0)
		{
			timer_catch = 0;
			game_status = 0;
		}
	}

	// Eliminar objetivo
	this->FireWeapon();
}

void RenderEngine::FireWeapon()
{
    if (GetAsyncKeyState(VK_LBUTTON) || (this->_demoMode && this->_totalFrames % 10 == 0))
    {
        if (this->target_hit) {
            this->cant_capturados++;
            this->tex.Anomalies(this->tex.Anomalies() - 1);
        }

        auto pos = lookFrom + viewDir*voxel_step0;
        auto drawDistance = 128;
        glBindTexture(GL_TEXTURE_3D, tex.id());
        auto radius = this->tex.AnomalieRadius() * 2;
        auto size = 4 * radius*radius*radius;
        auto *RGBABuffer = new char[size];
        memset(RGBABuffer, 0, size);
        glTexSubImage3D(GL_TEXTURE_3D, 0, static_cast<int>(pos.x + drawDistance) - radius / 2, static_cast<int>(pos.z + drawDistance) - radius / 2, static_cast<int>(pos.y + drawDistance) - radius / 2, radius, radius, radius, GL_RGBA, GL_UNSIGNED_BYTE, RGBABuffer);
        delete[] RGBABuffer;
    }
}

void RenderEngine::CheckObjetivoEnLaMira()
{
    auto *array = (GLfloat*)calloc(4192, sizeof(GLfloat));
    if (array != nullptr)
    {
        int r = 5;
        int cant = 4 * r*r;
        mr = mg = mb = 0;
        glReadPixels(fbWidth / 2 - r, fbHeight / 2 - r, 2 * r, 2 * r, GL_RGB, GL_FLOAT, array);
        for (auto i = 0; i < cant; ++i)
        {
            mr += array[3 * i];
            mg += array[3 * i + 1];
            mb += array[3 * i + 2];
        }

        mr /= cant;
        mg /= cant;
        mb /= cant;

        auto R = mr * 256;
        auto G = mg * 256;
        auto B = mb * 256;
        if (R > G && R > B && R > 200)
        {
            this->target_hit = true;
        }
    }
}

void RenderEngine::TextureVR()
{
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.05f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    // Escalo y roto con respecto al centro del cubo 
    glTranslatef(0.5f, 0.5f, 0.5f);
	//glScaled(E, (float)tex.dy()/(float)tex.dx()  *E, (float)tex.dx() / (float)tex.dz()*E);
	//glScaled(0.5,1,1);
	//mat4 transform = mat4::RotateX(an_x) * mat4::RotateY(an_y) * mat4::RotateZ(an_z);
    //mat4 transform = mat4::fromBase(viewDir , U,V);
    auto t = time*0.1f;
    auto transform = mat4::RotateX(t) * mat4::RotateY(t) * mat4::RotateZ(t);
    glMultMatrixd((const double *)transform.m());
    glTranslatef(-0.5f, -0.5f, -0.5f);

    glEnable(GL_TEXTURE_3D);

    glUseProgramObjectARB(this->_textureVRShaderProgram);
    auto pos = lookFrom;
    glUniform3f(glGetUniformLocation(this->_textureVRShaderProgram, "pos"), static_cast<float>(pos.x), static_cast<float>(pos.y), static_cast<float>(pos.z));
    glUniform3f(glGetUniformLocation(this->_textureVRShaderProgram, "iViewDir"), static_cast<float>(viewDir.x), static_cast<float>(viewDir.y), static_cast<float>(viewDir.z));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, tex.id());

    GLint texLoc = glGetUniformLocation(this->_textureVRShaderProgram, "s_texture0");
    glUniform1i(texLoc, 0);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    auto TexIndex = 0.f;
    auto s = 0.f;
	auto d = 0.f;
	auto q = 0.25f;
	for (auto fIndx = -1.0f; fIndx <= 1.0f; fIndx += 0.05f)
    {
        glBegin(GL_QUADS);
        TexIndex = fIndx;
        s = (1 - fIndx) / 4.0f * 0.1f;
        d = 1.25f ;

        glTexCoord3f(0, 0, ((float)TexIndex + 1.0f) / 2.0f);
        glVertex3f(-1 + s + d + q, -1 + s + d, TexIndex);

        glTexCoord3f(1, 0, ((float)TexIndex + 1.0f) / 2.0f);
        glVertex3f(1 - s , -1 + s + d, TexIndex);

        glTexCoord3f(1, 1, ((float)TexIndex + 1.0f) / 2.0f);
        glVertex3f(1 - s , 1 - s, TexIndex);

        glTexCoord3f(0, 1, ((float)TexIndex + 1.0f) / 2.0f);
        glVertex3f(-1 + s + d + q, 1 - s, TexIndex);

        glEnd();
    }

    glUseProgramObjectARB(0);
    glLoadIdentity();
    glDisable(GL_TEXTURE_3D);

    /*char saux[40];
    sprintf_s(saux,"fps = %.1f",fps);
    renderText(10,10,saux);*/
}

void RenderEngine::Release()
{
    //TODO:
    // falta liberar todo....
}



// HUD lateral
void RenderEngine::renderHUD()
{
	int mx = fbWidth / 2;
	int my = fbHeight / 2;
	int pos_y_electro, pos_y_aux, pos_y_time;

	// preview craneo
	renderGradientRoundRect(960 * E, 5 * E, 310 * E, my-5*E, 25 * E, 25 * E);
	// electro
	renderGradientRoundRect(960 * E, pos_y_electro = my+5*E, 310 * E, 100 * E, 0, 0);
	
	// scoreboarad aux
	renderGradientRoundRect2(960 * E, pos_y_aux = my + 110*E , 310 * E, 100 * E, 50 * E, 25 * E);
	renderGradientRoundRect2(1010 * E, pos_y_time = my + 220*E, 260 * E, 100 * E, 25 * E, 25 * E);

	char saux[40];

	// Target MIRA
	int px = fbWidth / 2;
	int py = fbHeight / 2;
	int r = this->CheckTargetHit() ? 80 : 40;
	glColor4ub(0, 113, 192,150);
	this->renderCircle(px, py, r);

	glColor4ub(0, 143, 222, 255);
	float x0 = 2 * px / (float)fbWidth - 1;
	float y0 = 1 - 2 * py / (float)fbHeight;
	float rx = 2 * (r + 10) *E / (float)fbWidth;
	float ry = 2 * (r + 10) *E / (float)fbHeight;

	glLineWidth(4);
	glBegin(GL_LINES);
	glVertex3f(x0 - rx, y0, 0);
	glVertex3f(x0 - rx*0.25f, y0, 0);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(x0 + rx, y0, 0);
	glVertex3f(x0 + rx*0.25f, y0, 0);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(x0, y0 - ry, 0);
	glVertex3f(x0, y0 - ry*0.25f, 0);
	glEnd();

	glBegin(GL_LINES);
	glVertex3f(x0, y0 + ry, 0);
	glVertex3f(x0, y0 + ry*0.25f, 0);
	glEnd();

	glLineWidth(1);
	rx *= 0.9f;
	ry *= 0.9f;
	auto alfa = 0.f;
	for (auto an = 0; an <= 360; an += 10)
	{
		alfa = an*3.1415f / 180.0f;
		glBegin(GL_LINES);
		glVertex3f(x0 + static_cast<float>(rx*cos(alfa)), static_cast<float>(y0 + ry*sin(alfa)), 0);
		glVertex3f(x0 + static_cast<float>(rx*cos(alfa)*0.9), static_cast<float>(y0 + ry*sin(alfa)*0.9), 0);
		glEnd();
	}

	glColor4ub(0, 170, 242, 255);
	glLineWidth(1);
	rx *= 3.f;
	ry *= 3.f;
	alfa = 0.f;
	for (auto an = 0; an <= 360; an += 10)
	{
		alfa = an*3.1415f / 180.0f;
		glBegin(GL_LINES);
		glVertex3f(x0 + static_cast<float>(rx*cos(alfa)), static_cast<float>(y0 + ry*sin(alfa)), 0);
		glVertex3f(x0 + static_cast<float>(rx*cos(alfa)*0.85), static_cast<float>(y0 + ry*sin(alfa)*0.85), 0);
		glEnd();
	}

	if (this->CheckTargetHit())
	{
		renderText(px - 40, py, "Fire!");
	}

	// velocity
	glLineWidth(3);
	rx = 2 * (r + 10) *E / (float)fbWidth;
	ry = 2 * (r + 10) *E / (float)fbHeight;
	float an_hasta = 100 * vel_tras / max_vel_tras;
	for (auto an = 0; an <= an_hasta; an += 5)
	{
		float alfa = (an-90)*3.1415f / 180.0f;
		if(an<50)
			glColor4ub(128, 255, 128, 255);
		else
		if (an<80)
			glColor4ub(255, 128, 0, 255);
		else
			glColor4ub(255, 0, 0, 255);

		glBegin(GL_LINES);
		glVertex3f(x0 - static_cast<float>(rx*cos(alfa)), static_cast<float>(y0 + ry*sin(alfa)), 0);
		glVertex3f(x0 - static_cast<float>(rx*cos(alfa)*0.7), static_cast<float>(y0 + ry*sin(alfa)*0.7), 0);
		glEnd();
	}


	// loof from
	glLineWidth(1);
	renderLine(mx + 25*E, my+25*E, mx + 100*E, my + 50*E, 240, 240, 240);
	renderLine(mx + 100 * E, my + 50 * E, mx + 300 * E, my + 50 * E, 240, 240, 240);
	renderLine(mx + 300 * E, my + 50 * E, mx + 300 * E, my + 75 * E, 240, 240, 240);
	renderLine(mx + 300 * E, my + 75 * E, mx + 150 * E, my + 75 * E, 240, 240, 240);
	renderLine(mx + 150 * E, my + 75 * E, mx + 150 * E, my + 50 * E,240, 240, 240);
	glColor4f(1, 1, 1, 1);
	sprintf(saux, "[%d,%d,%d]", (int)lookFrom.x, (int)lookFrom.y, (int)lookFrom.z);
	renderText(0.00075f*E, mx + 160*E , my + 56*E, saux);

	// Color promedio
	renderLine(mx+50*E, my-50*E , mx + 100*E, my - 150*E, 240, 240, 240);
	renderLine(mx + 100 * E, my - 150 * E, mx + 250 * E, my - 150 * E, 240, 240, 240);
	renderLine(mx + 250 * E, my - 150 * E, mx + 250 * E, my - 175 * E, 240, 240, 240);
	renderLine(mx + 250 * E, my - 175 * E, mx + 150 * E, my - 175 * E, 240, 240, 240);
	renderLine(mx + 150 * E, my - 175 * E, mx + 150 * E, my - 150 * E, 240, 240, 240);
	glLineWidth(1);
	glColor4f(1, 1, 1, 1);
	sprintf(saux, "I:%d", (int)(mr*255));
	renderText(0.00075f* E, mx + 160*E, my - 170 * E, saux);
	glColor3f(mr, mg, mb);
	renderRect(mx + 225 * E, my -172 * E, 22*E, 22 * E);


	// ECG
	glLineWidth(3);
	glColor4f(1, 1, 1, 0.25);
	this->renderText(0.0012f* E, 970*E, pos_y_electro + 10 * E, "EKG");

	glLineWidth(3);
	glColor3ub(150, 255, 150);
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i <3200; i += 5)
	{
		int k = i + time * 2000;
		int px = (970  + i / 12.) * E;
		int py = pos_y_electro + 30*E + ECG[k%cant_muestras] * 50*E;
		float x0 = 2 * px / (float)fbWidth - 1;
		float y0 = 1 - 2 * py/ (float)fbHeight;
		glVertex3f(x0, y0, 0);
	}
	glEnd();



	// fps y time
	// Debug fps
	sprintf_s(saux, "fps = %.1f", this->fps);
	renderText(10 * E, 10 * E, saux);
	// score
	sprintf_s(saux, "SCORE :  %04d", this->cant_capturados * 100);
	renderText(10 * E, 40 * E, saux);

	// Time
	glLineWidth(4);
	glColor4f(1, 1, 1, 0.25);
	this->renderText(0.0015f* E, 1020 * E, pos_y_time + 10 * E, "Time Limit");
	glColor4f(1, 1, 1, 0.5f);
	sprintf_s(saux, "%02d:%02d", (int)(time / 60), ((int)time) % 60);
	this->renderText(0.0025f* E, 1030 * E, pos_y_time + 40 * E, saux);
	glLineWidth(2);
	glColor4f(1, 1, 1, 1);
	this->renderText(0.0025f* E, 1030 * E, pos_y_time + 40 * E, saux);


	// Step0 - cant pasos
	glLineWidth(4);
	glColor4f(1, 1, 1, 0.25);
	this->renderText(0.0015f* E, 970 * E, pos_y_aux + 10 * E, "Step0 - st");
	glColor4f(1, 1, 1, 0.5f);
	sprintf_s(saux, "%d : %.3f", (int)voxel_step0 ,voxel_step);
	this->renderText(0.0015f* E, 980 * E, pos_y_aux + 40 * E, saux);
	glLineWidth(2);
	glColor4f(1, 1, 1, 1);
	this->renderText(0.0015f* E, 980 * E, pos_y_aux + 40 * E, saux);

	//sprintf_s(saux, "vel: %.3f", vel_tras);
	//this->renderText(0.0015f, 980, 500, saux);

	glLineWidth(1);
	glColor4f(1, 1, 1, 1);
	// modo navegacion o aceleracion
	this->renderText(0.0012f*E, mx - 150 * E, my - 150 * E, modo_aceleracion ? "ACL" : "NAV");
	// modo de vision: normal / x
	strcpy(saux, modo_visionX ? "VX" : "NRM");
	if(filtro)
		strcat(saux, "+TX");
	this->renderText(0.0012f*E, mx - 150 * E, my + 150 * E, saux);

	RenderQuad(&hud , mx , my , atan2(viewDir.y, viewDir.x));



}

bool RenderEngine::CheckTargetHit()
{
    return this->target_hit && this->filtro == 0;
}


void RenderEngine::renderCircle(int px, int py, int r)
{
    float x0 = 2 * px / (float)fbWidth - 1;
    float y0 = 1 - 2 * py / (float)fbHeight;
    float rx = 2 * r / (float)fbWidth;
    float ry = 2 * r / (float)fbHeight;

    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(x0, y0, 0);
    auto alfa = 0.f;
    for (auto an = 0; an <= 360; an += 10)
    {
        alfa = an*3.1415f / 180.0f;
        glVertex3f(x0 + static_cast<float>(rx*cos(alfa)), static_cast<float>(y0 + ry*sin(alfa)), 0);
    }

    glEnd();
}



void RenderEngine::renderGradientRect(int px0, int py0, int dx, int dy)
{
	// clear rect
	glColor3ub(0, 0, 0);
	renderRect(px0, py0, dx, dy);

	int px1 = px0 + dx;
	int py1 = py0 + dy;

	float x0 = 2 * px0 / (float)fbWidth - 1;
	float y0 = 1 - 2 * py0 / (float)fbHeight;
	float x1 = 2 * px1 / (float)fbWidth - 1;
	float y1 = 1 - 2 * py1 / (float)fbHeight;

	glColor3ub(42, 226, 228);
	glBegin(GL_LINE_STRIP);
	glVertex3f(x0, y0, 0);
	glVertex3f(x1, y0, 0);
	glVertex3f(x1, y1, 0);
	glVertex3f(x0, y1, 0);
	glVertex3f(x0, y0, 0);
	glEnd();

	
}


void RenderEngine::renderGradientRoundRect(int px0, int py0, int dx, int dy, int prx, int pry)
{
	// clear rect
	glColor3ub(0, 0, 0);
	renderRect(px0 + prx, py0, dx-prx, dy);
	renderRect(px0, py0+pry, prx, dy-pry);

	int px1 = px0 + dx;
	int py1 = py0 + dy;

	float x0 = 2 * px0  / (float)fbWidth - 1;
	float y0 = 1 - 2 * py0 / (float)fbHeight;
	float x1 = 2 * px1 / (float)fbWidth - 1;
	float y1 = 1 - 2 * py1 / (float)fbHeight;
	float rx = 2 * prx  / (float)fbWidth;
	float ry = 2 * pry  / (float)fbHeight;

	// clear tri
	glBegin(GL_TRIANGLES);
	glVertex3f(x0, y0-ry, 0);
	glVertex3f(x0+rx, y0, 0);
	glVertex3f(x0+rx, y0-ry, 0);
	glEnd();

	// draw lines
	glLineWidth(12);
	glColor4ub(32, 177, 179, 40);
	glBegin(GL_LINE_STRIP);
	glVertex3f(x0, y0 - ry, 0);
	glVertex3f(x0 + rx, y0, 0);
	glVertex3f(x1, y0, 0);
	glVertex3f(x1, y1, 0);
	glVertex3f(x0, y1, 0);
	glVertex3f(x0, y0 - ry, 0);
	glEnd();
	glLineWidth(8);
	glColor4ub(32, 177, 179, 100);
	glBegin(GL_LINE_STRIP);
	glVertex3f(x0, y0 - ry, 0);
	glVertex3f(x0 + rx, y0, 0);
	glVertex3f(x1, y0, 0);
	glVertex3f(x1, y1, 0);
	glVertex3f(x0, y1, 0);
	glVertex3f(x0, y0 - ry, 0);
	glEnd();

	glLineWidth(2);
	glColor3ub(42, 226, 228);
	glBegin(GL_LINE_STRIP);
	glVertex3f(x0, y0 - ry, 0);
	glVertex3f(x0 + rx, y0, 0);
	glVertex3f(x1, y0, 0);
	glVertex3f(x1, y1, 0);
	glVertex3f(x0, y1, 0);
	glVertex3f(x0, y0 - ry, 0);
	glEnd();

}


void RenderEngine::renderGradientRoundRect3(int px0, int py0, int dx, int dy, int prx, int pry)
{
	// clear rect
	glColor3ub(0, 0, 0);
	renderRect(px0, py0, dx - prx, dy);
	renderRect(px0+dx-prx, py0 + pry, prx, dy - pry);

	int px1 = px0 + dx;
	int py1 = py0 + dy;

	float x0 = 2 * px0 / (float)fbWidth - 1;
	float y0 = 1 - 2 * py0 / (float)fbHeight;
	float x1 = 2 * px1 / (float)fbWidth - 1;
	float y1 = 1 - 2 * py1 / (float)fbHeight;
	float rx = 2 * prx / (float)fbWidth;
	float ry = 2 * pry / (float)fbHeight;

	// clear tri
	glBegin(GL_TRIANGLES);
	glVertex3f(x1-rx , y0, 0);
	glVertex3f(x1, y0-ry, 0);
	glVertex3f(x1-rx, y0 - ry, 0);
	glEnd();

	// draw lines
	glLineWidth(12);
	glColor4ub(32, 177, 179, 40);
	glBegin(GL_LINE_STRIP);
	glVertex3f(x0, y0, 0);
	glVertex3f(x1 - rx, y0, 0);
	glVertex3f(x1, y0-ry, 0);
	glVertex3f(x1, y1, 0);
	glVertex3f(x0, y1, 0);
	glVertex3f(x0, y0, 0);
	glEnd();
	

	glLineWidth(8);
	glColor4ub(32, 177, 179, 100);
	glBegin(GL_LINE_STRIP);
	glVertex3f(x0, y0, 0);
	glVertex3f(x1 - rx, y0, 0);
	glVertex3f(x1, y0 - ry, 0);
	glVertex3f(x1, y1, 0);
	glVertex3f(x0, y1, 0);
	glVertex3f(x0, y0, 0);
	glEnd();

	glLineWidth(2);
	glColor3ub(42, 226, 228);
	glBegin(GL_LINE_STRIP);
	glVertex3f(x0, y0, 0);
	glVertex3f(x1 - rx, y0, 0);
	glVertex3f(x1, y0 - ry, 0);
	glVertex3f(x1, y1, 0);
	glVertex3f(x0, y1, 0);
	glVertex3f(x0, y0, 0);
	glEnd();


}


void RenderEngine::renderGradientRoundRect2(int px0, int py0, int dx, int dy, int prx, int pry)
{
	// clear rect
	glColor3ub(0, 0, 0);
	renderRect(px0, py0, dx, dy-pry);
	renderRect(px0 + prx, py0 + dy - pry, dx - prx, pry);

	int px1 = px0 + dx;
	int py1 = py0 + dy;

	float x0 = 2 * px0 / (float)fbWidth - 1;
	float y0 = 1 - 2 * py0 / (float)fbHeight;
	float x1 = 2 * px1 / (float)fbWidth - 1;
	float y1 = 1 - 2 * py1 / (float)fbHeight;
	float rx = 2 * prx / (float)fbWidth;
	float ry = 2 * pry / (float)fbHeight;

	// clear tri
	glBegin(GL_TRIANGLES);
	glVertex3f(x0, y1 + ry, 0);
	glVertex3f(x0+rx, y1 + ry, 0);
	glVertex3f(x0+rx, y1, 0);
	glEnd();

	// draw lines
	glLineWidth(12);
	glColor4ub(32, 177, 179, 40);
	glBegin(GL_LINE_STRIP);
	glVertex3f(x0, y0, 0);
	glVertex3f(x1, y0, 0);
	glVertex3f(x1, y1, 0);
	glVertex3f(x0 + rx, y1, 0);
	glVertex3f(x0, y1 + ry, 0);
	glVertex3f(x0, y0, 0);
	glEnd();
	glLineWidth(8);
	glColor4ub(32, 177, 179, 100);
	glBegin(GL_LINE_STRIP);
	glVertex3f(x0, y0, 0);
	glVertex3f(x1, y0, 0);
	glVertex3f(x1, y1, 0);
	glVertex3f(x0 + rx, y1, 0);
	glVertex3f(x0, y1 + ry, 0);
	glVertex3f(x0, y0, 0);
	glEnd();

	glLineWidth(2);
	glColor3ub(42, 226, 228);
	glBegin(GL_LINE_STRIP);
	glVertex3f(x0, y0, 0);
	glVertex3f(x1, y0, 0);
	glVertex3f(x1, y1, 0);
	glVertex3f(x0+rx, y1, 0);
	glVertex3f(x0, y1+ry, 0);
	glVertex3f(x0, y0, 0);
	glEnd();

}


void RenderEngine::renderGradientLine(int px0, int py0, int px1, int py1)
{
	float x0 = 2 * px0 / (float)fbWidth - 1;
	float y0 = 1 - 2 * py0 / (float)fbHeight;
	float x1 = 2 * px1 / (float)fbWidth - 1;
	float y1 = 1 - 2 * py1 / (float)fbHeight;

	glLineWidth(12);
	glColor4ub(32, 177, 179, 40);

	glBegin(GL_LINES);
	glVertex3f(x0, y0, 0);
	glVertex3f(x1, y1, 0);
	glEnd();

	glLineWidth(8);
	glColor4ub(32, 177, 179, 100);
	glBegin(GL_LINES);
	glVertex3f(x0, y0, 0);
	glVertex3f(x1, y1, 0);
	glEnd();

	glLineWidth(2);
	glColor3ub(42, 226, 228);
	glBegin(GL_LINES);
	glVertex3f(x0, y0, 0);
	glVertex3f(x1, y1, 0);
	glEnd();

}

void RenderEngine::renderLine(int px0, int py0, int px1, int py1,BYTE r, BYTE g, BYTE b)
{

	glColor3ub(r, g , b);

	float x0 = 2 * px0 / (float)fbWidth - 1;
	float y0 = 1 - 2 * py0 / (float)fbHeight;
	float x1 = 2 * px1 / (float)fbWidth - 1;
	float y1 = 1 - 2 * py1 / (float)fbHeight;

	glBegin(GL_LINES);
	glVertex3f(x0, y0, 0);
	glVertex3f(x1, y1, 0);
	glEnd();

}


void RenderEngine::renderRect(int px0, int py0, int dx, int dy)
{
    float x0 = 2 * px0 / (float)fbWidth - 1;
    float y0 = 1 - 2 * py0 / (float)fbHeight;
    float x1 = 2 * (px0 + dx) / (float)fbWidth - 1;
    float y1 = 1 - 2 * (py0 + dy) / (float)fbHeight;

    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(x0, y0, 0);
    glVertex3f(x1, y0, 0);
    glVertex3f(x0, y1, 0);
    glVertex3f(x1, y1, 0);
    glEnd();
}

void RenderEngine::renderText(int px, int py, char *text)
{
    glLineWidth(6);
    glColor4f(0.2f, 1.0f, 0.2f, 0.5f);
    this->renderText(0.0017f, px, py, text);

    glLineWidth(2);
    glColor3f(0.5f, 1.0f, 0.5f);
    this->renderText(0.0017f, px, py, text);
}

void RenderEngine::renderText(float K, int px, int py, char *text)
{
    float x0 = 2 * px / (float)fbWidth - 1;
    float y0 = 1 - 2 * py / (float)fbHeight;
    auto len = static_cast<int>(strlen(text));
    auto max_x = -999.f;
    for (int t = 0; t < len; ++t)
    {
        int j = text[t];
        if (j == 32)
        {
            x0 += 24 * K;
            continue;
        }

        Glyph *G = &glyphs[j];
        auto ant_x = 0.f, ant_y = 0.f;
        for (int i = 0; i < G->nNumPoints; i++)
        {
            // Ajusto posicion , origen y escala (en teoria orgien = 0, y escala = 1)
            float x = x0 + G->lpPt[i].x * K;
            float y = y0 - G->lpPt[i].y * K;
            if (x > max_x) {
                max_x = x;
            }

            switch (G->lpB[i])
            {
            case PT_MOVETO:
                ant_x = x;
                ant_y = y;
                break;
            case PT_LINETO | PT_CLOSEFIGURE:
            case PT_LINETO:
                glBegin(GL_LINES);
                glVertex3f(ant_x, ant_y, 0.0);
                glVertex3f(x, y, 0.0);
                glEnd();
                ant_x = x;
                ant_y = y;
                break;
            }
        }

        x0 = max_x + 10 * K;
    }

    glEnd();
}

void RenderEngine::loadShaders(char *vertexShaderSourceCode, char *fragmentShaderSourceCode, GLhandleARB *vertexShaderObject, GLhandleARB *fragmentShaderObject, GLhandleARB *shaderProgram)
{
    *vertexShaderObject = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    *fragmentShaderObject = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

    glShaderSourceARB(*vertexShaderObject, 1, const_cast<const GLcharARB**>(&vertexShaderSourceCode), NULL);
    glShaderSourceARB(*fragmentShaderObject, 1, const_cast<const GLcharARB**>(&fragmentShaderSourceCode), NULL);

    glCompileShaderARB(*vertexShaderObject);
    glCompileShaderARB(*fragmentShaderObject);

    this->CheckCompilationStatus(vertexShaderObject);
    this->CheckCompilationStatus(fragmentShaderObject);

    *shaderProgram = glCreateProgramObjectARB();

    glAttachObjectARB(*shaderProgram, *vertexShaderObject);
    glAttachObjectARB(*shaderProgram, *fragmentShaderObject);

    glLinkProgramARB(*shaderProgram);
}

void RenderEngine::CheckCompilationStatus(GLhandleARB *shaderObject)
{
    GLint status;
    glGetObjectParameterivARB(*shaderObject, GL_OBJECT_COMPILE_STATUS_ARB, &status);
    if (!status)
    {
        GLint maxLength = 0;
        glGetShaderiv(*shaderObject, GL_INFO_LOG_LENGTH, &maxLength);
        auto errorLog = new GLchar[maxLength];
        glGetShaderInfoLog(*shaderObject, maxLength, &maxLength, errorLog);
        AfxMessageBox(CString(errorLog));
        glDeleteShader(*shaderObject);
        delete[] errorLog;
        exit(0);
    }
}

void RenderEngine::setShaders()
{
    // Shaders ray casting
    auto vertexShaderSourceCode = this->textFileRead("../shaders/ray_casting.vs");
    auto fragmentShaderSourceCode = this->textFileRead("../shaders/ray_casting.fs");
    this->loadShaders(vertexShaderSourceCode, fragmentShaderSourceCode, &_vertexShaderRayCasting, &_fragmentShaderRayCasting, &_rayCastingShaderProgram);
    free(vertexShaderSourceCode);
    free(fragmentShaderSourceCode);

    // Shaders texture volumen
    vertexShaderSourceCode = this->textFileRead("../shaders/texture_vr.vs");
    fragmentShaderSourceCode = this->textFileRead("../shaders/texture_vr.fs");
    this->loadShaders(vertexShaderSourceCode, fragmentShaderSourceCode, &_vertexShaderTextureVR, &_fragmentShaderTextureVR, &_textureVRShaderProgram);
    free(vertexShaderSourceCode);
    free(fragmentShaderSourceCode);

	// Shaders fixed pipeline
	vertexShaderSourceCode = this->textFileRead("../shaders/fixed.vs");
	fragmentShaderSourceCode = this->textFileRead("../shaders/fixed.fs");
	this->loadShaders(vertexShaderSourceCode, fragmentShaderSourceCode, &_vertexShaderFixed, &_fragmentShaderFixed, &_fixedShaderProgram);
	free(vertexShaderSourceCode);
	free(fragmentShaderSourceCode);

	// Shaders ray casting Vision X
	vertexShaderSourceCode = this->textFileRead("../shaders/ray_casting.vs");
	fragmentShaderSourceCode = this->textFileRead("../shaders/ray_casting2.fs");
	this->loadShaders(vertexShaderSourceCode, fragmentShaderSourceCode, &_vertexShaderRayCasting2, &_fragmentShaderRayCasting2, &_rayCastingShaderProgram2);
	free(vertexShaderSourceCode);
	free(fragmentShaderSourceCode);


}

char *RenderEngine::textFileRead(char *fn)
{
    FILE *fp;
    char *content = NULL;
    int count = 0;
    if (fn != NULL)
    {
        fopen_s(&fp, fn, "rt");
        if (fp != NULL)
        {
            fseek(fp, 0, SEEK_END);
            count = ftell(fp);
            rewind(fp);
            if (count > 0)
            {
                content = (char *)malloc(sizeof(char) * (count + 1));
                if (content != nullptr)
                {
                    count = static_cast<int>(fread(content, sizeof(char), count, fp));
                    content[count] = '\0';
                }
            }

            fclose(fp);
        }
    }

    return content;
}


void RenderEngine::RenderFullScreenQuad(int texId)
{
	// pantalla de presentacion
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.05f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgramObjectARB(this->_fixedShaderProgram);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texId);

	GLint texLoc = glGetUniformLocation(this->_fixedShaderProgram, "s_texture0");
	glUniform1i(texLoc, 0);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBegin(GL_QUADS);

	glTexCoord2f(0, 0);
	glVertex3f(-1, -1, 0);

	glTexCoord2f(1, 0);
	glVertex3f(1, -1, 0);

	glTexCoord2f(1, 1);
	glVertex3f(1, 1, 0);

	glTexCoord2f(0, 1);
	glVertex3f(-1, 1, 0);

	glEnd();

	glUseProgramObjectARB(0);
	glLoadIdentity();

}


void RenderEngine::RenderQuad(Texture2d *tx, int px , int py, float an)
{
	// pantalla de presentacion
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.05f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgramObjectARB(this->_fixedShaderProgram);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tx->id);

	GLint texLoc = glGetUniformLocation(this->_fixedShaderProgram, "s_texture0");
	glUniform1i(texLoc, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	vec3 p[4] , coords[4];
	p[0] = vec3(px - tx->dx / 2 * E, py - tx->dy / 2 * E, 0);
	p[1] = vec3(px + tx->dx / 2 * E, py - tx->dy / 2 * E, 0);
	p[2] = vec3(px + tx->dx / 2 * E, py + tx->dy / 2 * E, 0);
	p[3] = vec3(px - tx->dx / 2 * E, py + tx->dy / 2 * E, 0);

	coords[0] = vec3(0, 0, 0);
	coords[1] = vec3(1, 0, 0);
	coords[2] = vec3(1, 1, 0);
	coords[3] = vec3(0, 1, 0);

	glBegin(GL_QUADS);
	for (int t = 0; t < 4; ++t)
	{
		p[t].rotar(vec3(px, py, 0), vec3(0, 0, 1), an);
		float x0 = 2 * p[t].x / (float)fbWidth - 1;
		float y0 = 1 - 2 * p[t].y / (float)fbHeight;
		glTexCoord2f(coords[t].x, coords[t].y);
		glVertex3f(x0, y0, 0);
	}
	glEnd();

	glUseProgramObjectARB(0);
	glLoadIdentity();

}

void RenderEngine::initECG()
{
	cant_muestras = 0;
	FILE *fp = fopen("../media/ecg.dat", "rb");
	if (!fp)
		return;

	char buffer[255];
	while(fgets(buffer,sizeof(buffer),fp)!=NULL)
	{ 
		char *p = strchr(buffer, ',');
		if (p != NULL)
			ECG[cant_muestras++] = atof(p + 1);
	}

	fclose(fp);

}
