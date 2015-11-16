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
		// ѭ����λ(1,2,3�ֱ����1,3,5��)
		int speed;
	};

public:
	JoystickStatus m_jsCurrentStatus;
private:
	CString m_ID;
	// ״̬��Ϣ��
	CPicaSoftCriticalSection m_csStatus;
};
