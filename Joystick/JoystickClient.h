#pragma once
#include "RobotAgent.h"
class JoystickClient;
typedef boost::shared_ptr<JoystickClient> JoystickClientPtr;
typedef boost::weak_ptr<JoystickClient> JoystickClientWeakPtr;
class JoystickClient
{
public:
	JoystickClient(LPCTSTR lpszID);
	~JoystickClient(void);
	void SetJoystickStatus(LPCTSTR sDirect);
	CString GetJoystickStatus();
	int GetJoystickSpeed();
	void SetJoystickSpeed(int nSpeed);
public:
	struct JoystickStatus
	{
		CString sDirect;
		// 循环档位(1,2,3分别代表1,3,5档)
		int speed;
	};

public:
	JoystickStatus m_jsCurrentStatus;
private:
	CString m_ID;
	// 状态信息锁
	CPicaSoftCriticalSection m_csStatus;
};
