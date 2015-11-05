#include "stdafx.h"
#include "TCPCommunication.h"
#include "RobotCommon.h"
#include "Robot.h"
#include "JoyStickClient.h"
// 默认处理器,啥都不做
class CDefaultProcessor :public ITCPProcessor
{
public:
	CDefaultProcessor(){}
	~CDefaultProcessor(){};

	void RecvData()
	{
	}

	static ITCPProcessorPtr GetDefaultProcessor()
	{
		return ITCPProcessorPtr(new CDefaultProcessor());
	}

};
CTCPCommunication::CTCPCommunication(SATCPConnectionPtr spTCPSocket)
{
	ASSERT(spTCPSocket);
	m_spSocket = spTCPSocket;
	m_spSocket->SetOnData(boost::bind(&CTCPCommunication::OnRecvData, this));
	m_sObjectName = F("TCP[%s]", spTCPSocket->PeerAddress().toString());
}
CTCPCommunication::~CTCPCommunication(void)
{
	Free();
}


// 启动
bool CTCPCommunication::Init()
{
	if (m_spSocket->Readable())
	{
		OnRecvData();
	}
	SALOG.information(F( _T("%s init success!"), m_sObjectName));
	return true;
}
// 释放
void CTCPCommunication::Free()
{
	m_spSocket->Close();
}
// 是否有效
bool CTCPCommunication::IsValid()
{
	if (!m_spProcessor && m_oConnectTime.isElapsed(30000*1000))
	{
		return false;
	}
	return m_spSocket->IsValid();
}

UInt32 CTCPCommunication::OnRecvData()
{
	if (m_spProcessor)
	{
		m_spProcessor->RecvData();
	}
	else
	{
		SATCPConnectionPtr spSocket = m_spSocket;
		if (spSocket)
		{
			size_t nReadableCount = spSocket->Readable();
			if (nReadableCount >= sizeof(SMsgHeader))
			{
				SMsgHeader head;
				spSocket->Read(&head, sizeof(head), false);
				head.nProtocolFlag = ntohl(head.nProtocolFlag);
				head.nMessageLength = ntohl(head.nMessageLength);
				head.nInvokeID = ntohl(head.nInvokeID);
				if (head.nProtocolFlag == PROTOCOLFLAG_ROBOTCONNECTION)
				{
					SPRobotPtr spRobot = SPRobotPtr(new Robot(spSocket));
					m_spProcessor = spRobot;
				}
				else if (head.nProtocolFlag == PROTOCOLFLAG_ROBOTAGENTCONNECTION)
				{
					SPJoyStickClientPtr spJoystick = SPJoyStickClientPtr(new JoyStickClient(spSocket));
					m_spProcessor = spJoystick;
				}
				else
				{
					SALOG.error("Msg header is error,ingore it!");
					spSocket->Close();
				}
				if (m_spProcessor)
				{
					m_spProcessor->RecvData();
				}
			}
		}
		else
		{
			SALOG.debug("what?? no readable data??");
		}

	}
	return 0;
}
