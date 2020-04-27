// PopupDlg.cpp: 实现文件
//

#include "pch.h"
#include "HookClient.h"
#include "PopupDlg.h"
#include "afxdialogex.h"


// PopupDlg 对话框

IMPLEMENT_DYNAMIC(PopupDlg, CDialogEx)

PopupDlg::PopupDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
	, m_szProcess(_T(""))
	, m_edtDetail(_T(""))
	, m_szTime(_T(""))
	, m_bAllow(0)
{

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


BEGIN_MESSAGE_MAP(PopupDlg, CDialogEx)
END_MESSAGE_MAP()


// PopupDlg 消息处理程序
