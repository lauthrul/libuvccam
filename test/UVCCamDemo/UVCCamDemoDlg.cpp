
// UVCCamDemoDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "UVCCamDemo.h"
#include "UVCCamDemoDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx {
public:
    CAboutDlg();

    // 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CUVCCamDemoDlg 对话框



CUVCCamDemoDlg::CUVCCamDemoDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_UVCCAMDEMO_DIALOG, pParent) {
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_nMoveAction = libuvccam::None;
    m_bMouseHold = FALSE;
    m_bMoving = FALSE;
}

void CUVCCamDemoDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_DEVICES, m_cmbDevices);
    DDX_Control(pDX, IDC_STATIC_STATUS, m_stcStatus);
    DDX_Control(pDX, IDC_BUTTON_LEFT, m_btnLeft);
    DDX_Control(pDX, IDC_BUTTON_UP, m_btnUp);
    DDX_Control(pDX, IDC_BUTTON_RIGHT, m_btnRight);
    DDX_Control(pDX, IDC_BUTTON_DOWN, m_btnDown);
    DDX_Control(pDX, IDC_BUTTON_ZOOMIN, m_btnZoomIn);
    DDX_Control(pDX, IDC_BUTTON_ZOOMOUT, m_btnZoomOut);
    DDX_Control(pDX, IDC_BUTTON_RESET, m_btnReset);
}

BEGIN_MESSAGE_MAP(CUVCCamDemoDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_TIMER()
    ON_CBN_SELCHANGE(IDC_COMBO_DEVICES, &CUVCCamDemoDlg::OnSelchangeComboDevices)
    ON_BN_CLICKED(IDC_BUTTON_RESET, &CUVCCamDemoDlg::OnBnClickedButtonReset)
END_MESSAGE_MAP()


// CUVCCamDemoDlg 消息处理程序

BOOL CUVCCamDemoDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    // 将“关于...”菜单项添加到系统菜单中。

    // IDM_ABOUTBOX 必须在系统命令范围内。
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != nullptr) {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty()) {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
    //  执行此操作
    SetIcon(m_hIcon, TRUE);			// 设置大图标
    SetIcon(m_hIcon, FALSE);		// 设置小图标

    // TODO: 在此添加额外的初始化代码
    m_cmbDevices.GetWindow(GW_CHILD)->SendMessage(EM_SETREADONLY, true, 0);

    map<int, TString> devices;
    m_uvcCamera.ListDevices(devices);
    for (auto dev : devices) {
        m_cmbDevices.AddString(dev.second.c_str());
    }

    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

BOOL PtInCtrlRect(CPoint pt, CWnd* ctrl) {
    CRect rc;
    ctrl->GetWindowRect(&rc);
    return PtInRect(rc, pt);
}

BOOL CUVCCamDemoDlg::PreTranslateMessage(MSG* pMsg) {
    switch (pMsg->message) {
    case WM_LBUTTONDOWN:
        {
            CPoint pt;
            GetCursorPos(&pt);

            m_nMoveAction = libuvccam::None;
            if (PtInCtrlRect(pt, &m_btnLeft)) {
                m_nMoveAction = libuvccam::MoveLeft;
            } else if (PtInCtrlRect(pt, &m_btnUp)) {
                m_nMoveAction = libuvccam::MoveUp;
            } else if (PtInCtrlRect(pt, &m_btnRight)) {
                m_nMoveAction = libuvccam::MoveRight;
            } else if (PtInCtrlRect(pt, &m_btnDown)) {
                m_nMoveAction = libuvccam::MoveDown;
            } else if (PtInCtrlRect(pt, &m_btnZoomIn)) {
                m_nMoveAction = libuvccam::ZoomIn;
            } else if (PtInCtrlRect(pt, &m_btnZoomOut)) {
                m_nMoveAction = libuvccam::ZoomOut;
            }
            if (m_nMoveAction != 0) {
                SetTimer(1, 200, NULL);
                m_bMouseHold = TRUE;
            }
        }
        break;
    case WM_LBUTTONUP:
        {
            if (m_uvcCamera.IsConnected()) {
                if (m_bMoving) {
                    KillTimer(1);
                    m_uvcCamera.RelativeStop();
                    m_bMoving = FALSE;
                } else {
                    // 短按
                    if (m_nMoveAction != libuvccam::None) {
                        m_uvcCamera.AbsoluteMove(m_nMoveAction);
                    }
                }
                m_bMouseHold = FALSE;
                m_nMoveAction = libuvccam::None;
            }
        }
        break;
    }
    return CDialogEx::PreTranslateMessage(pMsg);
}

VOID CUVCCamDemoDlg::OnTimer(UINT_PTR nIDEvent) {
    switch (nIDEvent) {
    case 1:
        if (m_uvcCamera.IsConnected()) {
            if (m_bMouseHold && m_nMoveAction != 0) {
                // 长按
                m_uvcCamera.RelativeMove(m_nMoveAction);
                m_bMoving = TRUE;
            }
        }
        break;
    }

    CDialogEx::OnTimer(nIDEvent);
}

void CUVCCamDemoDlg::OnSysCommand(UINT nID, LPARAM lParam) {
    if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    } else {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CUVCCamDemoDlg::OnPaint() {
    if (IsIconic()) {
        CPaintDC dc(this); // 用于绘制的设备上下文

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 使图标在工作区矩形中居中
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 绘制图标
        dc.DrawIcon(x, y, m_hIcon);
    } else {
        CDialogEx::OnPaint();
    }
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CUVCCamDemoDlg::OnQueryDragIcon() {
    return static_cast<HCURSOR>(m_hIcon);
}


void CUVCCamDemoDlg::OnSelchangeComboDevices() {
    CString text;
    m_cmbDevices.GetLBText(m_cmbDevices.GetCurSel(), text);
    auto ret = m_uvcCamera.Connect(text.GetBuffer());
    text.Format(L"[%s]连接%s", text, ret ? L"成功" : L"失败");
    m_stcStatus.SetWindowText(text);
}

void CUVCCamDemoDlg::OnBnClickedButtonReset() {
    if (m_uvcCamera.IsConnected()) {
        m_uvcCamera.Reset();
    }
}
