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
public:	// ȡ��ʵ��
	static JoystickMaster* instance();
	static CPicaSoftCriticalSection m_STLock;
	static JoystickMaster *pInstance;

public:
	// ��ʼ��
	void initialize();
	// �ͷ�
	void uninitialize();
	// �����߳�
	UINT WrokProc(LPVOID);
	// ����
	BOOL Start();
	// ֹͣ
	BOOL Stop(DWORD dwTimeout = 30000);
	BOOL ReloadSystemParameter();
public:
	// TCPClient
	CRobotAgent m_tcpRobotAgent;

	CString m_sLastDirect;
	ULONGLONG m_ullLastSendDirect;
	// �����߳�
	CPicaSoftMemfunWorkThread<JoystickMaster> m_thdWorkThread;
	CPicaSoftEvent m_evtChange;
	CPicaSoftEvent m_evtStopWorker;
	JoystickClientPtr m_spJoystick;
private:
	CPicaSoftLog m_iLog;
};
