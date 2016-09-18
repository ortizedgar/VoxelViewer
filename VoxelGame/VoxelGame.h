
// VoxelGame.h : main header file for the VoxelGame application
//
#pragma once

#ifndef __AFXWIN_H__
    #error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CVoxelGameApp:
// See VoxelGame.cpp for the implementation of this class
//

class CVoxelGameApp : public CWinApp
{
public:
    CVoxelGameApp();


// Overrides
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();

// Implementation

public:
    afx_msg void OnAppAbout();
    DECLARE_MESSAGE_MAP()
};

extern CVoxelGameApp theApp;
