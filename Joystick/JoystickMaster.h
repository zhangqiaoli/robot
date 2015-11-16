#pragma once
#include "RobotAgent.h"
#include "JoystickClient.h"

typedef struct _tagJoystick
{
	_tagJoystick()
	{
		speed = 3;
	}
	CString sDirect;
	int speed;
}SJoystick;
extern SJoystick g_JoystickStatus;

typedef struct _tagSetting
{
	_tagSetting()
	{
		nSpeedInterval = 2;
		nPort = 18020;
	}
	CString sServerIP;
	CString sRobotid;
	int nSpeedInterval;
	int nPort;
}SSetting;
extern SSetting g_Setting;

class JoystickMaster
{
public:
	JoystickMaster(void);
	~JoystickMaster(void);
public:	// 取得实例
	static JoystickMaster* instance();
	static CPicaSoftCriticalSection m_STLock;
	static JoystickMaster *pInstance;

public:
	// 初始化
	void initialize();
	// 释放
	void uninitialize();
	// 工作线程
	UINT WrokProc(LPVOID);
	// 启动
	BOOL Start();
	// 停止
	BOOL Stop(DWORD dwTimeout = 30000);
	BOOL ReloadSystemParameter();
public:
	// TCPClient
	CRobotAgent m_tcpRobotAgent;

	CString m_sLastDirect;
	ULONGLONG m_ullLastSendDirect;
	// 工作线程
	CPicaSoftMemfunWorkThread<JoystickMaster> m_thdWorkThread;
	CPicaSoftEvent m_evtChange;
	CPicaSoftEvent m_evtStopWorker;
	JoystickClientPtr m_spJoystick;
private:
	CPicaSoftLog m_iLog;
};
