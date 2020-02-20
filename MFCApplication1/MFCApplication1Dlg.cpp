
// MFCApplication1Dlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"
//#include "ComAsy1.h"
#include <exception>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
HANDLE m_hCom;
CWinThread* MyThread;
CWinThread* MyThread2;
void delay(int time);
OVERLAPPED m_ovRead;//用于读取数据	
OVERLAPPED m_ovWait;//用于等待数据	
volatile bool m_IsOpen, m_IsOpen2;//串口是否打开	
HANDLE m_Thread;//读取线程句柄
//CString out;
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
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCApplication1Dlg dialog



CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION1_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_MYUPDATEDATA, OnUpdateMyData)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication1Dlg::OnBnClickedButton1)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CMFCApplication1Dlg message handlers

BOOL CMFCApplication1Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	CFont *m_font4;
	m_font4 = new CFont;
	m_font4->CreateFont(45, 30, 0, 0, 700, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Calibri"));
	GetDlgItem(IDC_STATIC)->SetFont(m_font4);
	
	try 
	{ 
		AfxOleInit();
		CoInitialize(NULL);		
		m_pConnection = _ConnectionPtr(__uuidof(Connection));		
		m_pConnection->ConnectionString = "Provider=Microsoft.ACE.OLEDB.12.0; Data Source=D:\\new\\test.accdb;jet oledb:database password = jay123456;";		
	//	Provider = Microsoft.ACE.OLEDB.12.0; Data Source = \\129.202.101.220\new\test.accdb; Jet OLEDB : database password = jay123456; Persist Security Info = False
		m_pConnection->Open("", "", "", adConnectUnspecified); 
		
	}
	catch 
		(_com_error e)
	{ 
		AfxMessageBox(e.Description());
	return FALSE; 
	}
	//AfxMessageBox(_T("11"));
	LPCTSTR p1;
	char * userc;
	CString thadd;
	char THadd[256];
	thadd = "com3";
	p1 = thadd.GetBuffer(0);
	thadd.ReleaseBuffer();
	userc = new char[thadd.GetLength() + 1];
	strcpy_s(userc, thadd.GetLength() + 1, CT2CA(p1));
	strcpy_s(THadd, userc);
	//AfxMessageBox(CString(THadd));
	m_hCom = CreateFile(CString(THadd),//COM3口
		GENERIC_READ | GENERIC_WRITE, //允许读和写
		0, //独占方式
		NULL,
		OPEN_EXISTING, //打开而不是创建
		FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL,//设置异步标识
		NULL);
	if (m_hCom == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(_T("Open COM Fail!"));
	}
	SetupComm(m_hCom, 4096, 4096); //输入缓冲区和输出缓冲区的大小都是1893
	COMMTIMEOUTS ct;
	ct.ReadIntervalTimeout = MAXDWORD;//读取无延时，因为有WaitCommEvent等待数据	
	ct.ReadTotalTimeoutConstant = 0;  //	
	ct.ReadTotalTimeoutMultiplier = 0;//
	ct.WriteTotalTimeoutMultiplier = 500;
	ct.WriteTotalTimeoutConstant = 5000;
	SetCommTimeouts(m_hCom, &ct);

	DCB dcb;
	GetCommState(m_hCom, &dcb);
	dcb.DCBlength = sizeof(dcb);
	dcb.BaudRate = 115200; //波特率为115200
	//dcb.BaudRate = CBR_115200;
	dcb.ByteSize = 8; //每个字节有8位
	dcb.Parity = NOPARITY; //无奇偶校验位
	dcb.StopBits = ONESTOPBIT; //1个停止位
	SetCommState(m_hCom, &dcb);
	PurgeComm(m_hCom, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT);
	m_ovRead.hEvent = CreateEvent(NULL, false, false, NULL);
	//m_ovWrite.hEvent = CreateEvent(NULL, false, false, NULL);
	m_ovWait.hEvent = CreateEvent(NULL, false, false, NULL);
	SetCommMask(m_hCom, EV_ERR | EV_RXCHAR);//设置接受事件 	
	//ComAsy1 c1;
	
	//c1.InitCOM();
	MyThread = AfxBeginThread(MyThreadFunction, NULL, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
	MyThread2 = AfxBeginThread(MyThreadFunction2, NULL, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
	
	
	
	
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMFCApplication1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMFCApplication1Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



UINT MyThreadFunction(LPVOID pParam)
{
	// ComAsy1 c1;
	DWORD WaitEvent = 0, Bytes = 0;
	BOOL Status = FALSE;
	char ReadBuf[100];
	DWORD Error;
	COMSTAT cs = { 0 };
	CString xresult;
	HWND hwnd;
	char buf[256];
	m_IsOpen = true;
	//c1.InitCOM();
hwnd= ::FindWindow(NULL, _T("Monitor"));
	while (m_IsOpen)
	{
		WaitEvent = 0;
		m_ovWait.Offset = 0;
		Status = WaitCommEvent(m_hCom, &WaitEvent, &m_ovWait);
		if (FALSE == Status && GetLastError() == ERROR_IO_PENDING)//		
		{			//如果缓存中无数据线程会停在此，如果hCom关闭会立即返回False			
			Status = GetOverlappedResult(m_hCom, &m_ovWait, &Bytes, TRUE);
		}
		ClearCommError(m_hCom, &Error, &cs);
		if (TRUE == Status //等待事件成功

			&& WaitEvent&EV_RXCHAR//缓存中有数据到达

			&& cs.cbInQue > 0)//
		{
			//AfxMessageBox(_T("Sucess"));
			delay(500);
			xresult = "";
			Bytes = 100;
			m_ovRead.Offset = 0;
			memset(ReadBuf, '\0', 100);
			Status = ReadFile(m_hCom, ReadBuf, Bytes, &Bytes, &m_ovRead);
			if (Status != FALSE)
			{
				/*for (int i = 0; i < 4096; i++)
				{
					sprintf_s(buf, "%02X", ReadBuf[i]);
					xresult = xresult + CString(buf);
				}
				*/
				//sprintf_s(buf, "%d", Bytes);
				//AfxMessageBox(CString(buf));
				//AfxMessageBox(CString(ReadBuf));
				SendMessage(hwnd, // 获得主窗口在句柄   
					WM_MYUPDATEDATA,  // 发送自己的窗口消息   
					(WPARAM)&ReadBuf,     // 设置发送的内容   
					NULL);
				
			
			
			
			
			
			}
			PurgeComm(m_hCom, PURGE_RXCLEAR | PURGE_RXABORT);

		}
	
	
	
		delay(20);
	}
	
	
	
	
	
	
	
	
	//AfxMessageBox(_T("END"));
	return 0;
}

void delay(int Time)
{
	long t1, t2;
	t1 = GetTickCount();
	do {
		t2 = GetTickCount();
	} while (t2 - t1 < Time);
}

LRESULT CMFCApplication1Dlg::OnUpdateMyData(WPARAM wParam, LPARAM lParam)
{
	m_pRecordset.CreateInstance(__uuidof(Recordset));
	HWND hWnd;
	hWnd = ::FindWindow(NULL, _T("GCOptical"));
	char *str22 = (char*)wParam;
	//AfxMessageBox(CString(str22));
	GetDlgItem(SNbox)->SetWindowText(CString(str22));
	CString sqlHasRecord, sn, result;
	sn = CString(str22);
	sqlHasRecord.Format(_T("SELECT * FROM S7TEST2 WHERE SN = '%s'"), sn);
	m_pRecordset = ((CMFCApplication1Dlg*)(AfxGetMainWnd()))->m_pConnection->Execute(sqlHasRecord.AllocSysString(), NULL, adCmdText);
	if (!m_pRecordset->adoEOF)
	{
		_variant_t vIndex = (long)0;
		_variant_t vCount = m_pRecordset->GetCollect(vIndex);//取得第一个字段的值放入
		if (vCount.iVal < 1)
		{
			GetDlgItem(IDC_STATIC)->SetWindowText(_T("SN不存在"));
			::SendMessage(hWnd, WM_CLOSE, 0, 0);
		}
		else
		{
			result = m_pRecordset->GetCollect("Result").bstrVal;
			if (result == "OK")
			{
				GetDlgItem(IDC_STATIC)->SetWindowText(_T("PASS"));
			}
			else
			{
				GetDlgItem(IDC_STATIC)->SetWindowText(_T("NG"));
				::SendMessage(hWnd, WM_CLOSE, 0, 0);
			}

			//AfxMessageBox(result);
		}
	}
	else
	{
		GetDlgItem(IDC_STATIC)->SetWindowText(_T("SN不存在"));
		::SendMessage(hWnd, WM_CLOSE, 0, 0);
	}
	m_pRecordset->Close();
	m_pRecordset = NULL;
	return 0;
}




void CMFCApplication1Dlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	m_IsOpen = false;
	m_IsOpen2 = false;
	HWND hWnd;
	hWnd = ::FindWindow(NULL, _T("GCOptical"));
	if (INVALID_HANDLE_VALUE != m_hCom)
	{
		CloseHandle(m_hCom);
		m_hCom = INVALID_HANDLE_VALUE;
	}

	if (NULL != m_ovRead.hEvent)
	{
		CloseHandle(m_ovRead.hEvent);
		m_ovRead.hEvent = NULL;
	}
	/*
		if (NULL != m_ovWrite.hEvent)
		{
			CloseHandle(m_ovWrite.hEvent);
			m_ovWrite.hEvent = NULL;
		}
		*/
	if (NULL != m_ovWait.hEvent)
	{
		CloseHandle(m_ovWait.hEvent);
		m_ovWait.hEvent = NULL;
	}
		
	::SendMessage(hWnd, WM_CLOSE, 0, 0);
	if (m_pConnection->State)
	{
		m_pConnection->Close();
	}
	
	CDialogEx::OnClose();
}

UINT MyThreadFunction2(LPVOID pParam)
{
	m_IsOpen2 = true;
	while (m_IsOpen2)
	{
		HWND hWnd;
		hWnd = ::FindWindow(NULL, _T("GCOptical"));
		if (hWnd != NULL)
		{
			
			::SendMessage(hWnd, WM_SHOWWINDOW, SW_HIDE, SW_PARENTCLOSING);
		//	::SendMessage(hWnd, WM_SHOWWINDOW, SW_SHOW, SW_PARENTCLOSING);
			
			//::SendMessage(hWnd,WM_CLOSE,0,0);
		   //b=0;
		}



	}



	return 0;
}

void CMFCApplication1Dlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	CString sqlHasRecord,sn,result;
	sn = "ED00000005E28182300007";
	sqlHasRecord.Format(_T("SELECT * FROM S7TEST2 WHERE SN = '%s'"),sn);
	m_pRecordset = ((CMFCApplication1Dlg*)(AfxGetMainWnd()))->m_pConnection->Execute(sqlHasRecord.AllocSysString(), NULL, adCmdText);
	_variant_t vIndex = (long)0;
	_variant_t vCount = m_pRecordset->GetCollect(vIndex);//取得第一个字段的值放入
	if (vCount.iVal < 1)
	{
		AfxMessageBox(_T("No SN"));

	}
	else
	{
		result = m_pRecordset->GetCollect("Result").bstrVal;
		AfxMessageBox(result);
	}



}


HBRUSH CMFCApplication1Dlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	CString tr;
	GetDlgItemText(IDC_STATIC, tr);
	
	if (tr == "PASS")
	{
		pDC->SetTextColor(RGB(0, 255, 0));
	}
	else
	{
		pDC->SetTextColor(RGB(255, 0, 0));
	}
	
	
	
	
	
	return hbr;
}
