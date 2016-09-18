#pragma once

// VoxelGame.h : main header file for the VoxelGame application
#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif

// CVoxelGameApp:
// See VoxelGame.cpp for the implementation of this class
class CVoxelGameApp : public CWinApp
{
public:
    CVoxelGameApp();

    // Overrides
    virtual BOOL InitInstance();
    virtual int ExitInstance();

    afx_msg void OnAppAbout();
    DECLARE_MESSAGE_MAP()
};

extern CVoxelGameApp theApp;