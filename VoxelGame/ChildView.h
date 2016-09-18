#pragma once

// ChildView.h : interface of the CChildView class
// CChildView window
class CChildView : public CWnd
{
public:
    // Construction
    CChildView();
    virtual ~CChildView();
    bool primera_vez;

protected:
    // Overrides
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    afx_msg void OnPaint();
    void RenderLoop();
    DECLARE_MESSAGE_MAP()
};