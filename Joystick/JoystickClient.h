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
public:
	struct JoystickStatus
	{
		CString sDirect;
		int speed;
	};

public:
	JoystickStatus m_jsCurrentStatus;
private:
	CString m_ID;
	// ×´Ì¬ÐÅÏ¢Ëø
	CPicaSoftCriticalSection m_csStatus;
};
