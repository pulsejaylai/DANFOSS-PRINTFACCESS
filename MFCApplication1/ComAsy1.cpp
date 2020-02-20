#include "stdafx.h"
#include "ComAsy1.h"



CString out;
ComAsy1::ComAsy1() :
	m_hCom(INVALID_HANDLE_VALUE),
	m_IsOpen(false),
	m_Thread(NULL)
{

	memset(&m_ovWait, 0, sizeof(m_ovWait));
	//memset(&m_ovWrite, 0, sizeof(m_ovWrite));
	memset(&m_ovRead, 0, sizeof(m_ovRead));

}
ComAsy1::~ComAsy1()
{
	UninitCOM();
}

void ComAsy1::UninitCOM()
{
	m_IsOpen = false;

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

	if (NULL != m_Thread)
	{
		WaitForSingleObject(m_Thread, 5000);//�ȴ��߳̽���
		CloseHandle(m_Thread);
		m_Thread = NULL;
	}



}
bool ComAsy1::InitCOM()
{
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
	m_hCom = CreateFile(CString(THadd),//COM3��
		GENERIC_READ | GENERIC_WRITE, //�������д
		0, //��ռ��ʽ
		NULL,
		OPEN_EXISTING, //�򿪶����Ǵ���
		FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL,//�����첽��ʶ
		NULL);
	if (m_hCom == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(_T("Open COM Fail!"));
	}
	SetupComm(m_hCom, 4096, 4096); //���뻺����������������Ĵ�С����1893
	COMMTIMEOUTS ct;
	ct.ReadIntervalTimeout = MAXDWORD;//��ȡ����ʱ����Ϊ��WaitCommEvent�ȴ�����	
	ct.ReadTotalTimeoutConstant = 0;  //	
	ct.ReadTotalTimeoutMultiplier = 0;//
	ct.WriteTotalTimeoutMultiplier = 500;
	ct.WriteTotalTimeoutConstant = 5000;
	SetCommTimeouts(m_hCom, &ct);

	DCB dcb;
	GetCommState(m_hCom, &dcb);
	dcb.DCBlength = sizeof(dcb);
	dcb.BaudRate = 11520; //������Ϊ11520
	//dcb.BaudRate = CBR_115200;
	dcb.ByteSize = 8; //ÿ���ֽ���8λ
	dcb.Parity = NOPARITY; //����żУ��λ
	dcb.StopBits = ONESTOPBIT; //1��ֹͣλ
	SetCommState(m_hCom, &dcb);
	PurgeComm(m_hCom, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT);
	m_ovRead.hEvent = CreateEvent(NULL, false, false, NULL);
	//m_ovWrite.hEvent = CreateEvent(NULL, false, false, NULL);
	m_ovWait.hEvent = CreateEvent(NULL, false, false, NULL);
	SetCommMask(m_hCom, EV_ERR | EV_RXCHAR);//���ý����¼� 	
	//������ȡ�߳�	
	m_Thread = (HANDLE)_beginthreadex(NULL, 0, &ComAsy1::OnRecv, this, 0, NULL);
	m_IsOpen = true;

	return true;
}

unsigned int __stdcall ComAsy1::OnRecv(void* LPParam)
{
	
	ComAsy1 *obj = static_cast<ComAsy1*>(LPParam); 	
	DWORD WaitEvent = 0, Bytes = 0;	
	BOOL Status = FALSE;	
	BYTE ReadBuf[4096];	
	DWORD Error;	
	COMSTAT cs = { 0 };
	while (obj->m_IsOpen)
	{
		AfxMessageBox(_T("Enter Thead"));
		WaitEvent = 0;
		obj->m_ovWait.Offset = 0;
		Status = WaitCommEvent(obj->m_hCom, &WaitEvent, &obj->m_ovWait);
		if (FALSE == Status && GetLastError() == ERROR_IO_PENDING)//		
		{			//����������������̻߳�ͣ�ڴˣ����hCom�رջ���������False			
			Status = GetOverlappedResult(obj->m_hCom, &obj->m_ovWait,  &Bytes, TRUE);		
		}
		ClearCommError(obj->m_hCom, &Error, &cs);
		if (TRUE == Status //�ȴ��¼��ɹ�

			&& WaitEvent&EV_RXCHAR//�����������ݵ���

			&& cs.cbInQue > 0)//
		{
			AfxMessageBox(_T("Sucess"));
			Bytes = 0;
			obj->m_ovRead.Offset = 0;
			memset(ReadBuf, 0, sizeof(ReadBuf));
			Status = ReadFile(obj->m_hCom, ReadBuf, sizeof(ReadBuf), &Bytes, &obj->m_ovRead);
			if (Status != FALSE)
			{ 
				out= (LPCSTR)ReadBuf; 
				AfxMessageBox(out);
			}			
			PurgeComm(obj->m_hCom, PURGE_RXCLEAR | PURGE_RXABORT);
			
		}
	
	
	}

	return 0;
}

CString result()
{
	CString ee;
	ee = out;
		return ee;
}
bool ComAsy1::ComWrite(LPBYTE buf, int &len)
{
	BOOL rtn = FALSE;	
	DWORD WriteSize = 0; 	
	PurgeComm(m_hCom, PURGE_TXCLEAR | PURGE_TXABORT);	
	m_ovWait.Offset = 0;	
	rtn = WriteFile(m_hCom, buf, len, &WriteSize, &m_ovWrite);
	
	len = 0;	
	if (FALSE == rtn && GetLastError() == ERROR_IO_PENDING)//��̨��ȡ	
	{		//�ȴ�����д�����		
		if (FALSE == ::GetOverlappedResult(m_hCom, &m_ovWrite, &WriteSize, TRUE))		
		{		
			return false;		
		}	
	}
		
	len = WriteSize;

	return rtn != FALSE;



}