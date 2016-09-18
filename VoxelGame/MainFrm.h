#pragma once

#include "ChildView.h"

// MainFrm.h : interface of the CMainFrame class
class CMainFrame : public CFrameWnd
{
public:
    CMainFrame();
    
    // Overrides
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

    // Implementation
    virtual ~CMainFrame();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    DECLARE_DYNAMIC(CMainFrame)

    // Control bar embedded members
    CToolBar m_wndToolBar;
    CChildView m_wndView;

    // Generated message map functions
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSetFocus(CWnd *pOldWnd);
    DECLARE_MESSAGE_MAP()
};