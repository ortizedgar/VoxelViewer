// ChildView.cpp : implementation of the CChildView class

#include "stdafx.h"
#include "VoxelGame.h"
#include "ChildView.h"
#include "RenderEngine.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

RenderEngine escena;

// CChildView
CChildView::CChildView()
{
    this->primera_vez = true;
}

CChildView::~CChildView()
{
    escena.Release();
}

BEGIN_MESSAGE_MAP(CChildView, CWnd)
    ON_WM_PAINT()
END_MESSAGE_MAP()

// CChildView message handlers
BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CWnd::PreCreateWindow(cs))
    {
        return FALSE;
    }

    cs.dwExStyle |= WS_EX_CLIENTEDGE;
    cs.style &= ~WS_BORDER;
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
        ::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), NULL);

    return TRUE;
}

void CChildView::OnPaint()
{
    // device context for painting
    CPaintDC dc(this);
    if (primera_vez)
    {
        primera_vez = false;
        if (!escena.Initialize(dc.m_hDC))
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

void CChildView::RenderLoop()
{
    auto seguir = static_cast<BOOL>(TRUE);
    auto time = 0.f;

    // Address of current frequency
    LARGE_INTEGER F, T0, T1;
    QueryPerformanceFrequency(&F);
    QueryPerformanceCounter(&T0);

    auto cant_frames = 0;
    auto frame_time = 0.f;

    while (seguir)
    {
        QueryPerformanceCounter(&T1);
        auto elapsed_time = static_cast<double>((T1.QuadPart - T0.QuadPart)) / static_cast<double>(F.QuadPart);
        T0 = T1;
        time += static_cast<float>(elapsed_time);
        frame_time += static_cast<float>(elapsed_time);
        escena.time = time;
        escena.elapsed_time = static_cast<float>(elapsed_time);
        if (frame_time > 1)
        {
            escena.fps = cant_frames / frame_time;
            frame_time = 0;
            cant_frames = 0;
        }

        auto vel_rot = static_cast<float>(elapsed_time*1.5);
        auto cero = vec3(0, 0, 0);
        if (GetAsyncKeyState('D'))
        {
            escena.viewDir.rotar(cero, escena.U, -vel_rot);
            escena.V.rotar(cero, escena.U, -vel_rot);
        }

        if (GetAsyncKeyState('A'))
        {
            escena.viewDir.rotar(cero, escena.U, vel_rot);
            escena.V.rotar(cero, escena.U, vel_rot);
        }

        if (GetAsyncKeyState(VK_UP))
        {
            escena.viewDir.rotar(cero, escena.V, vel_rot);
            escena.U.rotar(cero, escena.V, vel_rot);
        }

        if (GetAsyncKeyState(VK_DOWN))
        {
            escena.viewDir.rotar(cero, escena.V, -vel_rot);
            escena.U.rotar(cero, escena.V, -vel_rot);
        }

        if (GetAsyncKeyState(VK_ADD)) {
            escena.filtro = 1;
        }

        if (GetAsyncKeyState(VK_SUBTRACT)) {
            escena.filtro = 0;
        }

        if (GetAsyncKeyState('W')) {
            escena.lookFrom = escena.lookFrom + escena.viewDir*(static_cast<float>(elapsed_time)*escena.vel_tras);
        }

        if (GetAsyncKeyState('S')) {
            escena.lookFrom = escena.lookFrom - escena.viewDir*(static_cast<float>(elapsed_time)*escena.vel_tras);
        }

        escena.lookFrom.x = clamp256(static_cast<float>(escena.lookFrom.x));
        escena.lookFrom.y = clamp256(static_cast<float>(escena.lookFrom.y));
        escena.lookFrom.z = clamp256(static_cast<float>(escena.lookFrom.z));

        escena.Render();
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