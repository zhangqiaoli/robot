#include "StdAfx.h"
#include "JoystickClient.h"
JoystickClient::JoystickClient(LPCTSTR lpszID)
{
	m_jsCurrentStatus.speed = 2;
}

JoystickClient::~JoystickClient(void)
{

}

void JoystickClient::SetJoystickStatus(LPCTSTR lpszsDirect)
{
	CPicaSoftSingleLock sl(&m_csStatus, TRUE);
	m_jsCurrentStatus.sDirect = lpszsDirect;
}

void JoystickClient::SetJoystickSpeed(int nSpeed)
{
	m_jsCurrentStatus.speed = nSpeed;
}

CString JoystickClient::GetJoystickStatus()
{
	CString sDirect;
	CPicaSoftSingleLock sl(&m_csStatus, TRUE);
	sDirect = m_jsCurrentStatus.sDirect;
	return sDirect;
}

int JoystickClient::GetJoystickSpeed()
{
	int nSpeed;
	CPicaSoftSingleLock sl(&m_csStatus, TRUE);
	nSpeed = m_jsCurrentStatus.speed;
	return nSpeed;
}
