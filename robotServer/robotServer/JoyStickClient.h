#ifdef _MSC_VER
#pragma once
#endif
#ifndef _INC_JOYSTICKClient_H
#define _INC_JOYSTICKClient_H
#include "SATCPSocket.h"
#include "rapidjson/document.h"

#include "RobotCommon.h"
#include "TCPCommunication.h"
class JoyStickClient;
typedef boost::shared_ptr<JoyStickClient> SPJoyStickClientPtr;
typedef boost::weak_ptr<JoyStickClient> SPJoyStickClientWeakPtr;

class JoyStickClient
	:public boost::enable_shared_from_this < JoyStickClient >, public ITCPProcessor
{
public:
	JoyStickClient(const string& id);
	JoyStickClient(SATCPConnectionPtr spConn);
	~JoyStickClient(void);
public:
	SPJoyStickClientPtr GetJoyStickClientPtr()
	{
		return shared_from_this();
	}
	string GetJoyStickClientID()
	{
		return m_sID;
	}
	string GetJoyStickClientName()
	{
		return m_sName;
	}
	void ProcessMessage(const SMsgHeader* header, const string& body);
	// �Ƿ���Ч
	bool IsValid();
	// �Ƿ��Ծ
	bool IsActive();
	// �ر�
	bool Close();
	bool isCreateisElapsed(Poco::Int64 timeout);
	bool SendRequest2JoyStickClient(const string& sMessage, const string& sMsgid);
	void RecvData();
protected:

	void ProcessData();
	void Process_Heartbeat(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse);
	void Process_Active(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse);
	void Process_Voice(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse);
	void Process_Stop(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse);
	bool SendMessage(const string& sMessage, UInt32 nInvokeID);
	void SetValid(bool isVaild);
private:
	SATCPConnectionPtr m_spSocket;
	string m_sID;
	string m_sName;
	string m_sClient;
	// ��־
	SANamedLogger log;
	bool m_bIsRegistered;
	// ����ʱ��
	Clock m_oCreate;
	// ���һ���յ���Ϣʱ��
	Clock m_oLastRecvMsg;
	SAMap<int, string> m_req;
	Poco::AtomicCounter m_oAtomicCounter;
};
#endif
