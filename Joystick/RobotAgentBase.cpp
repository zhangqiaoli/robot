#include "stdafx.h"
#include "RobotAgentBase.h"
#include "JoystickCommon.h"
#include "JoystickMaster.h"
CRobotAgentRequest::CRobotAgentRequest(CRobotAgentBasePtr spMonitorClient, SMsgHeader* pRequest)
{
	ASSERT(pRequest);
	ASSERT(spMonitorClient);
	m_swpClient = spMonitorClient;
	m_pRequest = pRequest;
	m_pResponse = NULL;
}

CRobotAgentRequest::~CRobotAgentRequest()
{
	if (m_pRequest) delete[](LPBYTE)m_pRequest; m_pRequest = NULL;
	if (m_pResponse) delete[](LPBYTE) m_pResponse; m_pResponse = NULL;
	m_swpClient.reset();
}

BOOL CRobotAgentRequest::WaitResponse(DWORD dwTimeout)
{
	return m_evtResponsed.Lock(dwTimeout) && m_pResponse != 0;
}

void CRobotAgentRequest::Done()
{
	m_evtResponsed.SetEvent();
}
///////////////////////////////////////////////////////////////////////
CRobotAgentBase::CRobotAgentBase(void)
{
	m_nMonitorPort = 4000;
	m_thdWorker.SetThreadProc(this, &CRobotAgentBase::WorkProc, NULL, _T("RobotAgent 工作线程"));
	m_sObjectName = "Joystick1";
	m_SN.SetValueRange(1, 255);
	m_nHeartbeatInterval = 5;
	m_ullLastSendMessage = PicaSoft::GetTickCount64();
	m_ullLastRecvMessage = PicaSoft::GetTickCount64();
}


CRobotAgentBase::~CRobotAgentBase(void)
{
	Free();
}
// 启动
BOOL CRobotAgentBase::Start()
{
	m_evtStopWorker.ResetEvent();
	return m_thdWorker.StartThread();
}

// 停止
BOOL CRobotAgentBase::Stop(DWORD dwTimeout/*=30000*/)
{
	m_evtStopWorker.SetEvent();
	if (!m_thdWorker.WaitThreadStop(dwTimeout))
	{
		PLOG(ELL_ERROR, _T("等待'%s'退出超时!线程没有停止!"), m_thdWorker.m_sThreadName);
	}
	return FALSE;
}

// 是否已经启动
BOOL CRobotAgentBase::IsRuning()
{
	return m_thdWorker.IsRuning();
}
// 初始化
void CRobotAgentBase::Init()
{
}

// 释放
void CRobotAgentBase::Free()
{
	if (Stop())
	{
	}
}

// 断开
BOOL CRobotAgentBase::Disconnect()
{
	if (IsConnected())
	{
		m_tcpSocket.Close();
	}
	return TRUE;
}

// 是否连接
BOOL CRobotAgentBase::IsConnected()
{
	return m_tcpSocket.m_evtConnected.IsSignal();
}


// 处理消息
BOOL CRobotAgentBase::ProcessMessage(SMsgHeader* pHeader, LPCSTR sMessage)
{
	ASSERT(pHeader);
	if (!pHeader) return FALSE;
	m_ullLastRecvMessage = PicaSoft::GetTickCount64();
	
	return TRUE;
	PLOG(ELL_INFORMATION, _T("RECV:'%s'"), sMessage);
}

// 工作线程
UINT CRobotAgentBase::WorkProc(LPVOID)
{
	BOOL bContinue = TRUE;
	CPicaSoftMultiLock ml;
	ml.Add(&m_evtStopWorker);
	ml.Add(&m_tcpSocket.m_evtReadable);
	ml.Add(&m_tcpSocket.m_evtDisconnected);
	do
	{
		PICASOFT_PROTECTEDCODE_BEGIN;
		if (!m_tcpSocket.m_evtConnected.IsSignal())
		{
			PLOG(ELL_INFORMATION, _T("%s正在连接RobotServer[%s:%d]..."), m_sObjectName, m_sMonitorAddress, m_nMonitorPort);
			if (m_tcpSocket.Connect(m_sMonitorAddress, m_nMonitorPort))
			{
				PLOG(ELL_INFORMATION, _T("%s成功连接到RobotServer!"), m_sObjectName);
				//SendMessage_Register(m_sUsername, m_sPassword);
				SendMessage_Heartbeat();
				PICASOFT_PROTECTEDCODE_BEGIN;
				//OnConnected();
				PICASOFT_PROTECTEDCODE_ENDMSG(__FUNCTION__ _T("调用OnConnected时产生异常!"));
			}
			else
			{
				PLOG(ELL_ERROR, _T("%s连接到RobotServer失败!"), m_sObjectName);
			}
		}
		if (m_tcpSocket.m_evtConnected.IsSignal())
		{
			ULONGLONG ullNextProcessHeartbeat = PicaSoft::GetTickCount64();
			for (;;)
			{
				DWORD dwLock = ml.Lock(5000);
				if (ml.GetItem(dwLock) == &m_tcpSocket.m_evtReadable)
				{
					m_tcpSocket.m_evtReadable.ResetEvent();
					ReadMessage();
					if (ullNextProcessHeartbeat < PicaSoft::GetTickCount64())
					{
						ProcessHeartbeat();
						ullNextProcessHeartbeat = PicaSoft::GetTickCount64() + 5000;
					}
				}
				else if (ml.GetItem(dwLock) == &m_tcpSocket.m_evtDisconnected)
				{
					PLOG(ELL_ERROR, _T("%s连接已经断开!"), m_sObjectName);
					PICASOFT_PROTECTEDCODE_BEGIN;
					//m_bIsRegistered = FALSE;
					//OnDisconnected();
					PICASOFT_PROTECTEDCODE_ENDMSG(__FUNCTION__ _T("调用OnDisconnected时产生异常!"));
					break;
				}
				else if (ml.GetItem(dwLock) == &m_evtStopWorker)
				{
					PLOG(ELL_ERROR, _T("%s收到停止信号,工作线程开始退出!"), m_sObjectName);
					m_tcpSocket.Close();
					//m_bIsRegistered = FALSE;
					//OnDisconnected();
					bContinue = FALSE;
					break;
				}
				else if (dwLock == WAIT_TIMEOUT)
				{
					if (ullNextProcessHeartbeat < PicaSoft::GetTickCount64())
					{
						ProcessHeartbeat();
						ullNextProcessHeartbeat = PicaSoft::GetTickCount64() + 5000;
					}
				}
				else
				{
					PLOG(ELL_ERROR, _T("%s锁定失败!"), m_sObjectName);
					break;
				}
			}
		}
		PICASOFT_PROTECTEDCODE_ENDMSG("MessageQueueBase工作线程执行过程中产生异常!");
	} while (!m_evtStopWorker.Lock(10000));
	return 0;
}
// 处理心跳
void CRobotAgentBase::ProcessHeartbeat()
{
	if (!m_tcpSocket.m_evtConnected.IsSignal()) return;
	ULONGLONG ullNow = PicaSoft::GetTickCount64();
	if (m_ullLastRecvMessage + m_nHeartbeatInterval * 1000 * 3 < ullNow)
	{
		PLOG(ELL_ERROR, _T("%s心跳超时！断开连接"), m_sObjectName);
		m_tcpSocket.Close();
		return;
	}
	if (m_ullLastSendMessage + m_nHeartbeatInterval * 1000 < ullNow)
	{
		SendMessage_Heartbeat();
	}
}

// 发送心跳
BOOL CRobotAgentBase::SendMessage_Heartbeat()
{
	PLOG(ELL_TRACE, _T("%s heartbeat..."), m_sObjectName);
	rapidjson::Document request2Robot;
	request2Robot.SetObject();
	rapidjson::Document::AllocatorType& allocator2R = request2Robot.GetAllocator();
	request2Robot.AddMember("sender", "joystick1", allocator2R);
	request2Robot.AddMember("sendertype", "joystick", allocator2R);
	request2Robot.AddMember("msgid", "joystick.heartbeat", allocator2R);
	rapidjson::Value body(rapidjson::kObjectType);
	request2Robot.AddMember("body", body, allocator2R);
	CString sToRobotMsg = JsonDocToString(request2Robot);
	SendRequest2RobotServer(sToRobotMsg, "joystick.heartbeat");
	return TRUE;
}

// 发送请求 发送文字消息
void CRobotAgentBase::SendMessage(LPCTSTR lpszMessage, DWORD nInvokeID)
{
	m_ullLastSendMessage = PicaSoft::GetTickCount64();
	CString sMessage = lpszMessage;
	int nMessageLength = sizeof(SMsgHeader) + sMessage.GetLength();
	TSTLBuffer vBuff;
	vBuff.resize(nMessageLength, 0);
	SMsgHeader* pHeader = (SMsgHeader*)&vBuff[0];
	pHeader->nInvokeID = htonl(nInvokeID);
	pHeader->nMessageLength = htonl(nMessageLength);
	pHeader->nProtocolFlag = htonl(PROTOCOLFLAG_ROBOTAGENTCONNECTION);
	memcpy(pHeader + 1, lpszMessage, sMessage.GetLength());
	m_tcpSocket.Write(&vBuff[0], vBuff.size());
	PLOG(ELL_TRACE, _T("SEND:Content=%s, length = %d"), lpszMessage, nMessageLength);
}

bool CRobotAgentBase::SendRequest2RobotServer(LPCTSTR lpszMessage, LPCTSTR lpszMsgid)
{
	SendMessage(lpszMessage, m_SN.Next());
	return true;
}

// 发送控制运动
void CRobotAgentBase::SendMessage_Active(LPCTSTR lpszOpera,int speed)
{
	// 发送控制机器人运动请求
	rapidjson::Document request2Robot;
	request2Robot.SetObject();
	rapidjson::Document::AllocatorType& allocator2R = request2Robot.GetAllocator();
	request2Robot.AddMember("sender", "joystick1", allocator2R);
	request2Robot.AddMember("sendertype", "joystick", allocator2R);
	request2Robot.AddMember("msgid", "joystick.ctl.active", allocator2R);
	rapidjson::Value body(rapidjson::kObjectType);
	body.AddMember("robot_id", g_Setting.sRobotid, allocator2R);
	rapidjson::Value activeMode(rapidjson::kObjectType);
	activeMode.AddMember("operation", lpszOpera, allocator2R);
	activeMode.AddMember("distance", 10, allocator2R);
	activeMode.AddMember("angle", 180, allocator2R);
	activeMode.AddMember("speed", speed, allocator2R);
	body.AddMember("active_mode", activeMode, allocator2R);
	request2Robot.AddMember("body", body, allocator2R);
	CString sToRobotMsg = JsonDocToString(request2Robot);
	SendRequest2RobotServer(sToRobotMsg, "joystick.ctl.active");
}


// 读取消息
UINT CRobotAgentBase::ReadMessage()
{
	PICASOFT_PROTECTEDCODE_BEGIN;
	for (ULONG nReadableCount = m_tcpSocket.GetReadableCount(); nReadableCount; nReadableCount = m_tcpSocket.GetReadableCount())
	{
		if (nReadableCount >= sizeof(SMsgHeader))
		{
			SMsgHeader header;
			m_tcpSocket.Read(&header, sizeof(header), FALSE);
			header.nProtocolFlag = ntohl(header.nProtocolFlag);
			header.nMessageLength = ntohl(header.nMessageLength);
			header.nInvokeID = ntohl(header.nInvokeID);
			if (header.nProtocolFlag == PROTOCOLFLAG_ROBOTAGENTCONNECTION)
			{
				if (header.nMessageLength <= nReadableCount)
				{
					TSTLBuffer vBuffer;
					vBuffer.resize(header.nMessageLength + 1);
					m_tcpSocket.Read(&vBuffer[0], header.nMessageLength);					

					(vBuffer)[header.nMessageLength] = 0;
					CString sMessage = CString(&vBuffer[sizeof(header)]);

					ProcessMessage((SMsgHeader*)&vBuffer[0], sMessage);
					//// 使用线程池处理
					//LPBYTE pBuffer=new BYTE[header.PackageLen];
					//m_tcpSocket.Read(pBuffer,header.PackageLen);
					//g_iThreadPool.AddTask(new CPCSMessageTask(shared_from_this(),pBuffer));

				}
				else break;
			}
			else
			{
				PLOG(ELL_ERROR, _T("%s 消息头部标识错误!对方可能不是PCS系统,断开系统连接!"), m_sObjectName);
				m_tcpSocket.Close();
				break;
			}
		}
		else break;
	}
	PICASOFT_PROTECTEDCODE_END;
	return 0;
}