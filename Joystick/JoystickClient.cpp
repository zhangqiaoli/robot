#include "StdAfx.h"
#include "JoystickClient.h"
JoystickClient::JoystickClient(LPCTSTR lpszID)
{

}

JoystickClient::~JoystickClient(void)
{

}

void JoystickClient::SetJoystickStatus(LPCTSTR lpszsDirect)
{
	CPicaSoftSingleLock sl(&m_csStatus, TRUE);
	m_jsCurrentStatus.sDirect = lpszsDirect;
}

CString JoystickClient::GetJoystickStatus()
{
	CString sDirect;
	CPicaSoftSingleLock sl(&m_csStatus, TRUE);
	sDirect = m_jsCurrentStatus.sDirect;
	return sDirect;
}
