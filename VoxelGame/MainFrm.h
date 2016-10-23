#pragma once

#include "ChildView.h"

// MainFrm.h : interface of the CMainFrame class
class MainFrame : public CFrameWnd
{
public:
    MainFrame();
    
    // Overrides
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

    // Implementation
    virtual ~MainFrame();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    DECLARE_DYNAMIC(MainFrame)

    // Control bar embedded members
    CToolBar m_wndToolBar;
    ChildView m_wndView;

    // Generated message map functions
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSetFocus(CWnd *pOldWnd);
    DECLARE_MESSAGE_MAP()
};