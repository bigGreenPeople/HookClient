#pragma once


// PopupDlg 对话框

class PopupDlg : public CDialogEx
{
	DECLARE_DYNAMIC(PopupDlg)

public:
	VOID SetProcess(LPCTSTR lpszProcess);
	VOID SetDetail(LPCTSTR lpszDetail);
	VOID SetMyTimer(UINT lefttime);
	PopupDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~PopupDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	DECLARE_MESSAGE_MAP()
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	CString m_szProcess;
	CString m_edtDetail;
	CString m_szTime;
	int m_bAllow;
protected:
	int m_lefttime;
public:
	afx_msg void OnBnClickedOk();
};
