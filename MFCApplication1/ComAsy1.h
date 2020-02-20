#pragma once
class ComAsy1
{
public:
	ComAsy1();
	~ComAsy1();
	bool InitCOM();//
	void UninitCOM(); //关闭串口并清理
	//写入数据
	bool ComWrite(LPBYTE buf, int &len);
	//读取线程
	static unsigned int __stdcall OnRecv(void*);
	CString result();
private:
	HANDLE m_hCom;
	OVERLAPPED m_ovWrite;//用于写入数据	
	OVERLAPPED m_ovRead;//用于读取数据	
	OVERLAPPED m_ovWait;//用于等待数据	
	volatile bool m_IsOpen;//串口是否打开	
	HANDLE m_Thread;//读取线程句柄

};

