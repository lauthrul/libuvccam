
// UVCCamDemoDlg.h: 头文件
//

#pragma once
#include "libuvccam.h"

// CUVCCamDemoDlg 对话框
class CUVCCamDemoDlg : public CDialogEx
{
// 构造
public:
	CUVCCamDemoDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UVCCAMDEMO_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    virtual VOID OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
    afx_msg void OnSelchangeComboDevices();
    afx_msg void OnBnClickedButtonReset();
    afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
    CComboBox m_cmbDevices;
    CStatic m_stcStatus;
    CButton m_btnLeft;
    CButton m_btnUp;
    CButton m_btnRight;
    CButton m_btnDown;
    CButton m_btnZoomIn;
    CButton m_btnZoomOut;
    CButton m_btnReset;

    libuvccam::UVCCamera m_uvcCamera;
    libuvccam::EMoveAction m_nMoveAction;
    BOOL m_bMouseHold;
    BOOL m_bMoving;
};
