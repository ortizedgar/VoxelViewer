// ChildView.cpp : implementation of the ChildView class

#include "stdafx.h"
#include "VoxelGame.h"
#include "ChildView.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ChildView
ChildView::ChildView()
{
    this->primera_vez = true;
    this->sensibilidad = 1000;
    this->oldCursorPosition = (LPPOINT)calloc(1, sizeof(this->oldCursorPosition));
    this->newCursorPosition = (LPPOINT)calloc(2, sizeof(this->newCursorPosition));
    this->filterKeyPressed = false;
    this->time = 60;
}

ChildView::~ChildView()
{
    this->escena.Release();
}

BEGIN_MESSAGE_MAP(ChildView, CWnd)
    ON_WM_PAINT()
END_MESSAGE_MAP()

// ChildView message handlers
BOOL ChildView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CWnd::PreCreateWindow(cs))
    {
        return FALSE;
    }

    cs.dwExStyle |= WS_EX_CLIENTEDGE;
    cs.style &= ~WS_BORDER;
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, ::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), NULL);

    return TRUE;
}

void ChildView::OnPaint()
{
    // device context for painting
    CPaintDC dc(this);
    if (this->primera_vez)
    {
        primera_vez = false;
        if (!this->escena.Initialize(dc.m_hDC))
        {
            AfxMessageBox(_T("Error al iniciar opengl"), MB_ICONSTOP);
        }
        else
        {
            RenderLoop();
        }
    }
}

// Helper clamp pos
float clamp256(float x)
{
    if (x < -128)
    {
        x += 256;
    }
    else
    {
        if (x > 128)
        {
            x -= 256;
        }
    }

    return x;
}

void ChildView::RenderLoop()
{
    auto seguir = static_cast<BOOL>(TRUE);

    // Address of current frequency
    LARGE_INTEGER F, T0, T1;
    QueryPerformanceFrequency(&F);
    QueryPerformanceCounter(&T0);

    auto cant_frames = 0;
    auto frame_time = 0.f;
    auto elapsed_time = 0.;
    auto cero = vec3(0, 0, 0);
    auto movimientoHorizontal = 0.l;
    auto movimientoVertical = 0.l;

    while (seguir && this->time > 0)
    {
        QueryPerformanceCounter(&T1);
        elapsed_time = static_cast<double>((T1.QuadPart - T0.QuadPart)) / static_cast<double>(F.QuadPart);
        T0 = T1;
        this->time -= static_cast<float>(elapsed_time);
        frame_time += static_cast<float>(elapsed_time);
        this->escena.time = this->time;
        this->escena.elapsed_time = static_cast<float>(elapsed_time);
        if (frame_time > 1)
        {
            this->escena.fps = cant_frames / frame_time;
            frame_time = 0;
            cant_frames = 0;
        }

        this->MoveCameraWithMouse(cero, movimientoHorizontal, movimientoVertical);
        this->MoveCameraWithKeyboard(elapsed_time);
        this->SetFiltroWithKeyboard();
        this->escena.lookFrom.x = clamp256(static_cast<float>(this->escena.lookFrom.x));
        this->escena.lookFrom.y = clamp256(static_cast<float>(this->escena.lookFrom.y));
        this->escena.lookFrom.z = clamp256(static_cast<float>(this->escena.lookFrom.z));

        this->escena.Render();
        ++cant_frames;

        MSG Msg;
        ZeroMemory(&Msg, sizeof(Msg));
        if (PeekMessage(&Msg, NULL, 0U, 0U, PM_REMOVE))
        {
            if (Msg.message == WM_QUIT || Msg.message == WM_CLOSE)
            {
                seguir = FALSE;
                break;
            }

            // Dejo que windows procese el mensaje
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);

            switch (Msg.message)
            {
            case WM_KEYDOWN:
                // virtual-key code 
                switch ((int)Msg.wParam)
                {
                case VK_NEXT:
                    break;
                case VK_PRIOR:
                    break;
                case VK_ESCAPE:
                    seguir = FALSE;
                    break;
                }

                break;
            }
        }
    }

    exit(0);
}

void ChildView::SetFiltroWithKeyboard()
{
    if (GetAsyncKeyState('F'))
    {
        if (this->filterKeyPressed == false)
        {
            this->filterKeyPressed = true;
            this->escena.filtro = this->escena.filtro == 0 ? 1 : 0;
        }
    }
    else
    {
        this->filterKeyPressed = false;
    }
}

void ChildView::MoveCameraWithKeyboard(double elapsed_time)
{
    if (GetAsyncKeyState('W'))
    {
        this->escena.lookFrom = this->escena.lookFrom + this->escena.viewDir*(static_cast<float>(elapsed_time)*this->escena.vel_tras);
    }

    if (GetAsyncKeyState('S'))
    {
        this->escena.lookFrom = this->escena.lookFrom - this->escena.viewDir*(static_cast<float>(elapsed_time)*this->escena.vel_tras);
    }
}

void ChildView::MoveCameraWithMouse(vec3 &cero, long double &movimientoHorizontal, long double &movimientoVertical)
{
    GetCursorPos(this->newCursorPosition);
    cero = vec3(0, 0, 0);
    movimientoHorizontal = 0.l;
    movimientoVertical = 0.l;
    if ((this->newCursorPosition->x - this->oldCursorPosition->x) != 0 || (this->newCursorPosition->y - this->oldCursorPosition->y) != 0)
    {
        movimientoHorizontal = this->oldCursorPosition->x - this->newCursorPosition->x;
        if (movimientoHorizontal != 0)
        {
            movimientoHorizontal /= sensibilidad;
            this->escena.viewDir.rotar(cero, this->escena.U, static_cast<float>(movimientoHorizontal));
            this->escena.V.rotar(cero, this->escena.U, static_cast<float>(movimientoHorizontal));
        }

        movimientoVertical = this->oldCursorPosition->y - this->newCursorPosition->y;
        if (movimientoVertical != 0)
        {
            movimientoVertical /= sensibilidad;
            this->escena.viewDir.rotar(cero, this->escena.V, static_cast<float>(movimientoVertical));
            this->escena.U.rotar(cero, this->escena.V, static_cast<float>(movimientoVertical));
        }

        SetCursorPos(this->escena.fbWidth / 2, this->escena.fbHeight / 2);
        GetCursorPos(this->oldCursorPosition);
    }
}
