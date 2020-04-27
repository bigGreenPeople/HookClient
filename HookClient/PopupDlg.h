#pragma once


// PopupDlg 对话框

class PopupDlg : public CDialogEx
{
	DECLARE_DYNAMIC(PopupDlg)

public:
	PopupDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~PopupDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_szProcess;
	CString m_edtDetail;
	CString m_szTime;
	int m_bAllow;
};
