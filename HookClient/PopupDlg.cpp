// PopupDlg.cpp: 实现文件
//

#include "pch.h"
#include "HookClient.h"
#include "PopupDlg.h"
#include "afxdialogex.h"


// PopupDlg 对话框
#define TIMER_ELAPSE_ID	1000

IMPLEMENT_DYNAMIC(PopupDlg, CDialogEx)

PopupDlg::PopupDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
	, m_szProcess(_T(""))
	, m_edtDetail(_T(""))
	, m_szTime(_T(""))
	, m_bAllow(0)
{
	m_edtDetail = _T("");
	m_bAllow = 1;
	m_szProcess = _T("");
	m_szTime = _T("还剩: 30 秒");
	//}}AFX_DATA_INIT
	m_lefttime = 30;
}

PopupDlg::~PopupDlg()
{
}

void PopupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_Process, m_szProcess);
	DDX_Text(pDX, IDC_Detail, m_edtDetail);
	DDX_Text(pDX, IDC_Timer, m_szTime);
	DDX_Radio(pDX, IDC_RADIO1, m_bAllow);
}

void PopupDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	switch (nIDEvent)
	{
	case TIMER_ELAPSE_ID:
		UpdateData(TRUE);
		m_szTime.Format(_T("还剩: %2d 秒"), --m_lefttime);
		UpdateData(FALSE);
		if (m_lefttime == 0)
		{
			KillTimer(nIDEvent);
			UpdateData(TRUE);
			CDialog::OnOK();
		}
		break;
	default:
		break;
	}

	CDialog::OnTimer(nIDEvent);
}


BEGIN_MESSAGE_MAP(PopupDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &PopupDlg::OnBnClickedOk)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// PopupDlg 消息处理程序


void PopupDlg::OnBnClickedOk()
{
	// TODO: Add extra validation here
	KillTimer(TIMER_ELAPSE_ID);
	CDialogEx::OnOK();
}


BOOL PopupDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	SetTimer(TIMER_ELAPSE_ID, 1 * 1000, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

VOID PopupDlg::SetMyTimer(UINT lefttime)
{
	m_lefttime = lefttime;
	m_szTime = _T("还剩： 30 秒");
	SetTimer(TIMER_ELAPSE_ID, 1 * 1000, NULL);
}

VOID PopupDlg::SetDetail(LPCTSTR lpszDetail)
{
	m_edtDetail = lpszDetail;
}

VOID PopupDlg::SetProcess(LPCTSTR lpszProcess)
{
	m_szProcess = lpszProcess;

}
