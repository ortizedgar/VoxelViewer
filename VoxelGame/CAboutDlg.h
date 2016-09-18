#pragma once

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

    // Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

protected:
    // DDX/DDV support
    virtual void DoDataExchange(CDataExchange* pDX);

    // Implementation
    DECLARE_MESSAGE_MAP()
};