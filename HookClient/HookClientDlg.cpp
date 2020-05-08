
// HookClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "HookClient.h"
#include "HookClientDlg.h"
#include "afxdialogex.h"
#include "PopupDlg.h"
#include <winsvc.h>			//加载驱动的服务
#include <winioctl.h>		//IOCTRL API 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define IOCTL_SEND_RESULT_TO_R0 (ULONG) CTL_CODE(FILE_DEVICE_UNKNOWN, 0x8001, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define DRIVER_NAME _T("HookDrv")
#define DRIVER_PATH _T(".\\HookDrv.sys")

HANDLE gh_Device = INVALID_HANDLE_VALUE;

CWinThread	*g_hReadThread = NULL;
BOOL	g_bToExitThread = FALSE;
HANDLE	g_hOverlappedEvent = NULL;

BOOL LoadDriver(TCHAR* lpszDriverName, TCHAR* lpszDriverPath)
{
	TCHAR szDriverImagePath[256] = { 0 }/*_T("D:\\Popup\\HookDrv.sys")*/;
	//得到完整的驱动路径
	GetFullPathName(lpszDriverPath, 256, szDriverImagePath, NULL);

	BOOL bRet = FALSE;

	SC_HANDLE hServiceMgr = NULL;//SCM管理器的句柄
	SC_HANDLE hServiceDDK = NULL;//NT驱动程序的服务句柄

	//打开服务控制管理器
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hServiceMgr == NULL)
	{
		//OpenSCManager失败
		//printf( "OpenSCManager() Failed %d ! \n", GetLastError() );
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		////OpenSCManager成功
		printf("OpenSCManager() ok ! \n");
	}

	//创建驱动所对应的服务
	hServiceDDK = CreateService(hServiceMgr,
		lpszDriverName, //驱动程序的在注册表中的名字  
		lpszDriverName, // 注册表驱动程序的 DisplayName 值  
		SERVICE_ALL_ACCESS, // 加载驱动程序的访问权限  
		SERVICE_KERNEL_DRIVER,// 表示加载的服务是驱动程序  
		SERVICE_DEMAND_START, // 注册表驱动程序的 Start 值  
		SERVICE_ERROR_IGNORE, // 注册表驱动程序的 ErrorControl 值  
		szDriverImagePath, // 注册表驱动程序的 ImagePath 值  
		NULL,  //GroupOrder HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\GroupOrderList
		NULL,
		NULL,
		NULL,
		NULL);

	DWORD dwRtn;
	//判断服务是否失败
	if (hServiceDDK == NULL)
	{
		dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)
		{
			//由于其他原因创建服务失败
			//printf( "CrateService() Failed %d ! \n", dwRtn );  
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			//服务创建失败，是由于服务已经创立过
			printf("CrateService() Faild Service is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS! \n");
		}

		// 驱动程序已经加载，只需要打开  
		hServiceDDK = OpenService(hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS);
		if (hServiceDDK == NULL)
		{
			//如果打开服务也失败，则意味错误
			dwRtn = GetLastError();
			//printf( "OpenService() Failed %d ! \n", dwRtn );  
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			//printf( "OpenService() ok ! \n" );
		}
	}
	else
	{
		//printf( "CrateService() ok ! \n" );
	}

	//开启此项服务
	bRet = StartService(hServiceDDK, NULL, NULL);
	if (!bRet)
	{
		DWORD dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING)
		{
			//printf( "StartService() Failed %d ! \n", dwRtn );  
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			if (dwRtn == ERROR_IO_PENDING)
			{
				//设备被挂住
				//printf( "StartService() Failed ERROR_IO_PENDING ! \n");
				bRet = FALSE;
				goto BeforeLeave;
			}
			else
			{
				//服务已经开启
				//printf( "StartService() Failed ERROR_SERVICE_ALREADY_RUNNING ! \n");
				bRet = TRUE;
				goto BeforeLeave;
			}
		}
	}
	bRet = TRUE;
	//离开前关闭句柄
BeforeLeave:
	if (hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if (hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;
}

//卸载驱动程序  
BOOL UnloadDriver(TCHAR * szSvrName)
{
	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr = NULL;//SCM管理器的句柄
	SC_HANDLE hServiceDDK = NULL;//NT驱动程序的服务句柄
	SERVICE_STATUS SvrSta;
	//打开SCM管理器
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		//带开SCM管理器失败
		printf("OpenSCManager() Failed %d ! \n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		//带开SCM管理器失败成功
		printf("OpenSCManager() ok ! \n");
	}
	//打开驱动所对应的服务
	hServiceDDK = OpenService(hServiceMgr, szSvrName, SERVICE_ALL_ACCESS);

	if (hServiceDDK == NULL)
	{
		//打开驱动所对应的服务失败
		printf("OpenService() Failed %d ! \n", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		printf("OpenService() ok ! \n");
	}
	//停止驱动程序，如果停止失败，只有重新启动才能，再动态加载。  
	if (!ControlService(hServiceDDK, SERVICE_CONTROL_STOP, &SvrSta))
	{
		printf("ControlService() Failed %d !\n", GetLastError());
	}
	else
	{
		//打开驱动所对应的失败
		printf("ControlService() ok !\n");
	}
	//动态卸载驱动程序。  
	if (!DeleteService(hServiceDDK))
	{
		//卸载失败
		printf("DeleteSrevice() Failed %d !\n", GetLastError());
	}
	else
	{
		//卸载成功
		printf("DelServer:eleteSrevice() ok !\n");
	}
	bRet = TRUE;
BeforeLeave:
	//离开前关闭打开的句柄
	if (hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if (hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;
}

HANDLE OpenDevice()
{
	//测试驱动程序  
	HANDLE hDevice = CreateFile(_T("\\\\.\\HookDrv"),
		GENERIC_WRITE | GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (hDevice != INVALID_HANDLE_VALUE)
	{
		printf("Create Device ok ! \n");
	}
	else
	{
		printf("Create Device faild %d ! \n", GetLastError());
		return NULL;
	}

	return hDevice;
}

typedef struct _R3_REPLY
{
	ULONG	m_ulWaitID;
	ULONG	m_ulBlocked;
}R3_REPLY;

typedef struct _OP_INFO
{
	WCHAR     m_ProcessName[MAX_PATH];
	DWORD	  m_ulProcessID;
	ULONG     m_ulWaitID;

} OP_INFO, *POP_INFO;


VOID  SendResultToR0(ULONG ulWaitID, BOOL bBlocked)
{
	if (gh_Device == INVALID_HANDLE_VALUE)
	{
		return;
	}

	R3_REPLY R3Reply;
	R3Reply.m_ulWaitID = ulWaitID;
	R3Reply.m_ulBlocked = bBlocked;

	ULONG ulRet = 0;
	::DeviceIoControl(gh_Device, IOCTL_SEND_RESULT_TO_R0, &R3Reply, sizeof(R3_REPLY), NULL, 0, &ulRet, NULL);

	return;
}
BOOL  HandleData(OP_INFO *pOpInfoData)
{

	PopupDlg dlg;

	dlg.SetProcess(pOpInfoData->m_ProcessName);
	dlg.SetDetail(_T("有进程正在非法攻击"));

	dlg.DoModal();

	if (dlg.m_bAllow == 0)
	{
		return FALSE;
	}

	return TRUE;
}

void  PopupInfoToUser(OP_INFO *pOpInfo, int Num)
{
	OP_INFO * currData = pOpInfo;
	CString szNum;

	for (int i = 0; i < Num; i++)
	{
		BOOL bResult = HandleData(currData);  // 此处可以弹框获得用户的结果
		if (bResult)
		{
			SendResultToR0(pOpInfo->m_ulWaitID, TRUE);
		}
		else
		{
			SendResultToR0(pOpInfo->m_ulWaitID, FALSE);
		}
		currData++;
	}
}

UINT ReadThreadProc(LPVOID lpContext)
{
	OVERLAPPED Overlapped;


	g_hOverlappedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (g_hOverlappedEvent == NULL || gh_Device == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	memset(&Overlapped, 0, sizeof(OVERLAPPED));

	ULONG ulReturn = 0;
	ULONG ulBytesReturn = 0;

	OP_INFO OpInfo;
	Overlapped.hEvent = g_hOverlappedEvent;

	::SleepEx(1, TRUE);

	while (TRUE)
	{
		ulReturn = ReadFile(gh_Device, &OpInfo, sizeof(OP_INFO), &ulBytesReturn, &Overlapped);

		if (g_bToExitThread == TRUE)
		{
			break;
		}

		if (ulReturn == 0)
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				ULONG ulApiReturn = WaitForSingleObject(Overlapped.hEvent, INFINITE);

				if (ulApiReturn == WAIT_FAILED)
				{
					break;
				}
				if (g_bToExitThread == TRUE)
				{
					break;
				}
			}
			else
			{
				continue;
			}
		}
		if (ulBytesReturn == sizeof(OP_INFO))
		{
			PopupInfoToUser(&OpInfo, 1);
		}

	}

	return 0;
}
// CHookClientDlg 对话框



CHookClientDlg::CHookClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HOOKCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHookClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHookClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CHookClientDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CHookClientDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, &CHookClientDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CHookClientDlg 消息处理程序

BOOL CHookClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CHookClientDlg::OnPaint()
{
	if (IsIconic())
	{
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
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CHookClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CHookClientDlg::OnBnClickedOk()
{
	// TODO: Add extra validation here

	DWORD dwThreadID = 0;
	g_bToExitThread = FALSE;
	//加载驱动
	BOOL bRet = LoadDriver(DRIVER_NAME, DRIVER_PATH);
	if (!bRet)
	{
		MessageBox(_T("加载驱动失败"), _T("Error"), MB_OK);
		return;
	}


	gh_Device = OpenDevice();
	if (gh_Device == NULL)
	{
		MessageBox(_T("打开设备失败"), _T("Error"), MB_OK);
		return;
	}

	g_hReadThread = AfxBeginThread(ReadThreadProc, this);

	g_hReadThread->SuspendThread();
	g_hReadThread->m_bAutoDelete = FALSE;
	g_hReadThread->ResumeThread();


	if (g_hReadThread == NULL)
	{
		CloseHandle(gh_Device);
		gh_Device = INVALID_HANDLE_VALUE;
		UnloadDriver(DRIVER_NAME);
		return;
	}

	GetDlgItem(IDOK)->EnableWindow(FALSE);
	GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
}


void CHookClientDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	g_bToExitThread = TRUE;
	if (g_hOverlappedEvent != NULL)
	{
		ResetEvent(g_hOverlappedEvent);

		if (g_hReadThread != NULL)
		{
			if (WaitForSingleObject(g_hReadThread->m_hThread, 3000) == WAIT_TIMEOUT)
			{
				TerminateThread(g_hReadThread->m_hThread, 0);
			}
			delete g_hReadThread;
			g_hReadThread = NULL;
		}

		CloseHandle(g_hOverlappedEvent);
		g_hOverlappedEvent = NULL;
	}
	if (gh_Device != INVALID_HANDLE_VALUE)
	{
		CloseHandle(gh_Device);
		gh_Device = INVALID_HANDLE_VALUE;
	}
	//UnloadDriver(DRIVER_NAME);
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
}


void CHookClientDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	PopupDlg dlg;

	dlg.SetDetail(_T("有进程正在非法攻击"));

	dlg.DoModal();

	dlg.m_bAllow;
}

void CHookClientDlg::OnClose()
{
	CDialog::OnCancel();
}