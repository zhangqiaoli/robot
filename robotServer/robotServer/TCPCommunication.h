#ifdef _MSC_VER
#pragma once
#endif
#ifndef _INC_TCPCOMMUNICATION_H
#define _INC_TCPCOMMUNICATION_H
#include <SATCPSocket.h>
interface ITCPProcessor;
typedef boost::shared_ptr<ITCPProcessor> ITCPProcessorPtr;
typedef boost::weak_ptr<ITCPProcessor> ITCPProcessorWeakPtr;
interface ITCPProcessor
{
	virtual void RecvData() = 0;
};
class CTCPCommunication;
typedef boost::shared_ptr<CTCPCommunication> CTCPCommunicationPtr;
typedef boost::weak_ptr<CTCPCommunication> CTCPCommunicationWeakPtr;

class CTCPCommunication
{
public:
	CTCPCommunication(SATCPConnectionPtr spTCPSocket);
	~CTCPCommunication(void);
public:
	// 初始化
	bool Init();
	// 释放
	void Free();
	// 是否有效
	bool IsValid();
private:
	UInt32 OnRecvData();
	UInt32 OnDisconnect();
private:
	SATCPConnectionPtr m_spSocket;
	ITCPProcessorPtr m_spProcessor;
	string m_sObjectName;
	Clock m_oConnectTime;
};
#endif

