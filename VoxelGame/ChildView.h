#pragma once

#include "RenderEngine.h"

// ChildView.h : interface of the ChildView class
// ChildView window
class ChildView : public CWnd
{
public:
    // Construction
    ChildView();
    virtual ~ChildView();

protected:
    // Overrides
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    afx_msg void OnPaint();
    void RenderLoop();
    DECLARE_MESSAGE_MAP()

private:
    LPPOINT oldCursorPosition;
    LPPOINT newCursorPosition;
    RenderEngine escena;
    bool primera_vez;
    int sensibilidad;
    void MoveCameraWithMouse(vec3 &cero, long double &movimientoHorizontal, long double &movimientoVertical);
    void MoveCameraWithKeyboard(double elapsed_time);
    void SetFiltroWithKeyboard();
    bool filterKeyPressed;
    float time;
};