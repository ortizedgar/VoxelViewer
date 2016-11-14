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
    auto settingsSection = L"Settings";
    auto filePath = L"./Settings.ini";
    this->time = static_cast<float>(GetPrivateProfileInt(settingsSection, L"time", 60, filePath));
    this->demoMode = GetPrivateProfileInt(settingsSection, L"demoMode", 0, filePath) != 0;
    this->sensibilidad = GetPrivateProfileInt(settingsSection, L"sensibilidad", 1000, filePath);
    this->oldCursorPosition = (LPPOINT)calloc(1, sizeof(this->oldCursorPosition));
    this->newCursorPosition = (LPPOINT)calloc(2, sizeof(this->newCursorPosition));
    this->primeraVez = true;
    this->filterKeyPressed = false;
    this->_demoKeyPressed = false;
    //this->escena.setDemoMode(this->demoMode);
    this->moveForeward = true;
    this->totalFrames = 0;
    this->mouseDemoSignal = 0;
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
    if (this->primeraVez)
    {
        primeraVez = false;
        if (!this->escena.Initialize(dc.m_hDC))
        {
            AfxMessageBox(_T("Error al iniciar opengl"), MB_ICONSTOP);
        }
        else
        {
            this->RenderLoop();
        }
    }
}

// Helper clamp pos
float ChildView::Clamp256(float x)
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
    auto elapsedTime = 0.;
    auto cero = vec3(0, 0, 0);
    auto movimientoHorizontal = 0.l;
    auto movimientoVertical = 0.l;
    MSG Msg;
    while (seguir && (this->time > 0 && this->escena.tex.Anomalies() > 0))
    {
        QueryPerformanceCounter(&T1);
        elapsedTime = static_cast<double>((T1.QuadPart - T0.QuadPart)) / static_cast<double>(F.QuadPart);
        T0 = T1;
        if (this->escena.game_stage != 0)
        {
            this->time -= static_cast<float>(elapsedTime);
        }

        if (this->demoMode)
        {
            this->totalFrames = this->totalFrames < MAXINT ? this->totalFrames + 1 : 0;
            this->escena.setTotalFrames(this->totalFrames);
        }

        frame_time += static_cast<float>(elapsedTime);
        this->escena.time = this->time;
        this->escena.elapsed_time = static_cast<float>(elapsedTime);
        if (frame_time > 1)
        {
            this->escena.fps = cant_frames / frame_time;
            frame_time = 0;
            cant_frames = 0;
        }

        this->MoveCameraWithMouse(cero, movimientoHorizontal, movimientoVertical, elapsedTime);
        this->MoveCameraWithKeyboard(elapsedTime);
        this->SetFiltroWithKeyboard();
		this->SetDemoMode();
		this->escena.lookFrom.x = this->Clamp256(static_cast<float>(this->escena.lookFrom.x));
        this->escena.lookFrom.y = this->Clamp256(static_cast<float>(this->escena.lookFrom.y));
        this->escena.lookFrom.z = this->Clamp256(static_cast<float>(this->escena.lookFrom.z));

        this->escena.Render();
        ++cant_frames;

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
                default:
                    if (this->escena.game_stage == 0)
                    {
                        this->escena.game_stage = 1;
                    }
                }
                break;
			case WM_KEYUP:
				// virtual-key code 
				switch ((int)Msg.wParam)
				{
					// modo navegacion / aceleracion
				case 'A':
					escena.modo_aceleracion = !escena.modo_aceleracion;
					break;

				case 'X':
					// modo vision X 
					escena.modo_visionX  = !escena.modo_visionX;
					break;
					
				}
				break;

            }
        }
    }

    this->escena.game_stage = 2;
    ZeroMemory(&Msg, sizeof(Msg));
    while (this->escena.game_stage == 2)
    {
        this->escena.Render();
        if (PeekMessage(&Msg, NULL, 0U, 0U, PM_REMOVE))
        {
            if (Msg.message == WM_KEYDOWN)
            {
                this->escena.game_stage = 3;
            }
        }
    }

    exit(0);
}

void ChildView::SetFiltroWithKeyboard()
{
    if (this->demoMode && this->totalFrames % 60 == 0)
    {
        this->escena.filtro = this->escena.filtro == 0 ? 1 : 0;
    }

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

void ChildView::SetDemoMode()
{
    if (GetAsyncKeyState(VK_SPACE))
    {
        if (this->_demoKeyPressed == false)
        {
            this->_demoKeyPressed = true;
            this->demoMode = this->demoMode == true ? false : true;
            this->escena.setDemoMode(this->demoMode);
        }
    }
    else
    {
        this->_demoKeyPressed = false;
    }
}


void ChildView::MoveCameraWithKeyboard(double elapsed_time)
{
    if (this->demoMode)
    {
        if ((int)this->totalFrames % 90 == 0)
        {
            this->moveForeward = !this->moveForeward;
        }
        if (this->moveForeward)
        {
            this->escena.lookFrom = this->escena.lookFrom + this->escena.viewDir*(static_cast<float>(elapsed_time)*this->escena.vel_tras);
        }
        else
        {
            this->escena.lookFrom = this->escena.lookFrom - this->escena.viewDir*(static_cast<float>(elapsed_time)*this->escena.vel_tras);
        }
    }
    else
    {
		if (escena.modo_aceleracion)
		{

			// modo acelaracion
			float acel = 10;
			if (GetAsyncKeyState('W'))
			{
				escena.vel_tras += elapsed_time*acel;
				if (escena.vel_tras > escena.max_vel_tras)
					escena.vel_tras = escena.max_vel_tras;

			}

			if (GetAsyncKeyState('S'))
			{
				escena.vel_tras -= elapsed_time*acel;
				if (escena.vel_tras < 0)
					escena.vel_tras = 0;
			}

			this->escena.lookFrom = this->escena.lookFrom + this->escena.viewDir*(static_cast<float>(elapsed_time)*this->escena.vel_tras);

		}
		else
		{
			// modo velocidad
			if (GetAsyncKeyState('W'))
			{
				this->escena.lookFrom = this->escena.lookFrom + this->escena.viewDir*(static_cast<float>(elapsed_time)*this->escena.vel_tras);
			}

			if (GetAsyncKeyState('S'))
			{
				this->escena.lookFrom = this->escena.lookFrom - this->escena.viewDir*(static_cast<float>(elapsed_time)*this->escena.vel_tras);
			}
		}

		if (GetAsyncKeyState(VK_ADD))
			if (GetAsyncKeyState(VK_SHIFT))
				escena.voxel_step0 *= 1.01f;
			else
				escena.voxel_step *= 1.01f;

		if (GetAsyncKeyState(VK_SUBTRACT))
			if (GetAsyncKeyState(VK_SHIFT))
				escena.voxel_step0 /= 1.01f;
			else
				escena.voxel_step /= 1.01f;

    }
}

void ChildView::MoveCameraWithMouse(vec3 &cero, long double &movimientoHorizontal, long double &movimientoVertical, double elapsedTime)
{
    GetCursorPos(this->newCursorPosition);
    cero = vec3(0, 0, 0);
    movimientoHorizontal = 0.l;
    movimientoVertical = 0.l;
    if (this->demoMode)
    {
        if ((int)this->totalFrames % 90 == 0)
        {
            this->mouseDemoSignal = this->mouseDemoSignal < 4 ? this->mouseDemoSignal + 1 : 0;
        }

        switch (this->mouseDemoSignal)
        {
        case 0:
            movimientoVertical = elapsedTime;
            movimientoHorizontal = 0;
            break;
        case 1:
            movimientoVertical = 0;
            movimientoHorizontal = elapsedTime;
            break;
        case 2:
            movimientoVertical = -elapsedTime;
            movimientoHorizontal = 0;
            break;
        case 3:
            movimientoVertical = 0;
            movimientoHorizontal = -elapsedTime;
            break;
        }

        this->escena.viewDir.rotar(cero, this->escena.U, static_cast<float>(movimientoHorizontal));
        this->escena.V.rotar(cero, this->escena.U, static_cast<float>(movimientoHorizontal));
        this->escena.viewDir.rotar(cero, this->escena.V, static_cast<float>(movimientoVertical));
        this->escena.U.rotar(cero, this->escena.V, static_cast<float>(movimientoVertical));
    }
    else
    {
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
}