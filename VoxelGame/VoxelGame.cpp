// VoxelGame.cpp : Defines the class behaviors for the application.

#include "stdafx.h"
#include "VoxelGame.h"
#include "CAboutDlg.h"
#include "MainFrm.h"
#include "Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(VoxelGame, CWinApp)
    ON_COMMAND(ID_APP_ABOUT, &VoxelGame::OnAppAbout)
END_MESSAGE_MAP()

// CVoxelGameApp construction
VoxelGame::VoxelGame()
{
    // TODO: replace application ID string below with unique ID string; recommended
    // format for string is CompanyName.ProductName.SubProduct.VersionInformation
    SetAppID(_T("VoxelGame.AppID.NoVersion"));

    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}

// The one and only CVoxelGameApp object
VoxelGame theApp;

// CVoxelGameApp initialization
BOOL VoxelGame::InitInstance()
{
    CWinApp::InitInstance();
    EnableTaskbarInteraction(FALSE);

    // AfxInitRichEdit2() is required to use RichEdit control    
    // AfxInitRichEdit2();

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need
    // Change the registry key under which our settings are stored
    // TODO: You should modify this string to be something appropriate
    // such as the name of your company or organization
    SetRegistryKey(_T("Local AppWizard-Generated Applications"));

    // To create the main window, this code creates a new frame window
    // object and then sets it as the application's main window object
    MainFrame* pFrame = new MainFrame;
    if (!pFrame)
    {
        return FALSE;
    }

    m_pMainWnd = pFrame;

    // Create and load the frame with its resources
    pFrame->LoadFrame(IDR_MAINFRAME,
        WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
        NULL);

    // The one and only window has been initialized, so show and update it
    pFrame->ShowWindow(SW_SHOWMAXIMIZED);
    //pFrame->ShowWindow(SW_SHOW);
    pFrame->UpdateWindow();
    return TRUE;
}

int VoxelGame::ExitInstance()
{
    //TODO: handle additional resources you may have added
    return CWinApp::ExitInstance();
}

// App command to run the dialog
void VoxelGame::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}