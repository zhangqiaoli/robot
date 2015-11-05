#include "StdAfx.h"
#include <PicaSoftINIFile.h>
#include "JoystickMaster.h"
SJoystick g_JoystickStatus;
SSetting g_Setting;

JoystickMaster* JoystickMaster::pInstance = 0;
CPicaSoftCriticalSection JoystickMaster::m_STLock;


JoystickMaster::JoystickMaster()
{
	m_thdWorkThread.SetThreadProc(this, &JoystickMaster::WrokProc, NULL, _T("InputSignal �����߳�"));
	m_ullLastSendDirect = PicaSoft::GetTickCount64();
}

JoystickMaster::~JoystickMaster(void)
{
}

JoystickMaster* JoystickMaster::instance()
{
	CPicaSoftSingleLock lock(&m_STLock, TRUE);
	try
	{
		if (pInstance == NULL)
		{
			pInstance = new JoystickMaster();
			if (pInstance)
			{
				pInstance->initialize();
			}
		}
	}
	catch (...)
	{
		PLOG(ELL_ERROR, _T("JoystickClient::GetInstance Exception!"));
	}
	return pInstance;
}
// ����
BOOL JoystickMaster::Start()
{
	m_evtStopWorker.ResetEvent();
	return m_thdWorkThread.StartThread();
}

// ֹͣ
BOOL JoystickMaster::Stop(DWORD dwTimeout/*=30000*/)
{
	m_evtStopWorker.SetEvent();
	if (!m_thdWorkThread.WaitThreadStop(dwTimeout))
	{
		PLOG(ELL_ERROR, _T("�ȴ�'%s'�˳���ʱ!�߳�û��ֹͣ!"), m_thdWorkThread.m_sThreadName);
	}
	return FALSE;
}

UINT JoystickMaster::WrokProc(LPVOID)
{
	CPicaSoftMultiLock ml;
	ml.Add(&m_evtStopWorker);
	ml.Add(&m_evtChange);
	for (;;)
	{
		DWORD dwLock = ml.Lock(1000);
		ULONGLONG ullNow = PicaSoft::GetTickCount64();
		if (ml.GetItem(dwLock) == &m_evtStopWorker)
		{
			break;
		}
		else if (ml.GetItem(dwLock) == &m_evtChange)
		{
			m_evtChange.ResetEvent();
			// �յ�ҡ�˸ı��¼�
			CString sDirect = m_spJoystick->GetJoystickStatus();
			if (m_sLastDirect != sDirect || m_ullLastSendDirect + 1000 > ullNow)
			{
				m_tcpRobotAgent.SendMessage_Active(sDirect, g_Setting.nSpeed);
				m_sLastDirect = sDirect;
				m_ullLastSendDirect = ullNow;
			}
		}
		else
		{
			// �ȴ��źų�ʱ
			CString sDirect = m_spJoystick->GetJoystickStatus();
			if (sDirect.GetLength() > 0 && sDirect != "stop")
			{
				m_tcpRobotAgent.SendMessage_Active(sDirect, g_Setting.nSpeed);
				m_sLastDirect = sDirect;
				m_ullLastSendDirect = ullNow;
			}
		}
	}
	return 1;
}

void JoystickMaster::initialize()
{
	PICASOFT_DEBUG_TRACK;//���ö�ջ����
	PICASOFT_SET_GLOBALLOGPOINTER(&m_iLog);
	m_iLog.EnableControlLog(TRUE);
	m_iLog.SetLevel(ELL_TRACE);
	m_iLog.SetMultithreadMode(FALSE);
	m_iLog.EnableFileLog(_T("$(ApplicationPath)Log\\RJS_$(Year4)$(Month2)$(Day2)$(FileNumber2).Log"));
}

// ���¼���ϵͳ����
BOOL JoystickMaster::ReloadSystemParameter()
{
	// ���ز���
	CPicaSoftINIFile ini;
	CString sIniFilename = CPicaSoftAssistant::FormatString(_T("$(ApplicationFullPath)"));
	CPicaSoftAssistant::ChangeFileExtName(sIniFilename, ".ini");
	ini.Open(sIniFilename, _T("System"));
	PICASOFT_SETTINGS_STR(ini, _T("ServerIP"), g_Setting.sServerIP);
	PICASOFT_SETTINGS_STR(ini, _T("RobotID"), g_Setting.sRobotid);
	PICASOFT_SETTINGS_INT(ini, _T("Port"), g_Setting.nPort);
	PICASOFT_SETTINGS_INT(ini, _T("Speed"), g_Setting.nSpeed);
	return TRUE;
}
