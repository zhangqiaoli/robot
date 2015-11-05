#ifdef _MSC_VER
#pragma once
#endif
#ifndef _INC_ROBOT_H
#define _INC_ROBOT_H
#include "SATCPSocket.h"
#include "rapidjson/document.h"
#include "RobotCommon.h"
#include "TCPCommunication.h"
class Robot;
typedef boost::shared_ptr<Robot> SPRobotPtr;
typedef boost::weak_ptr<Robot> SPRobotWeakPtr;

class Robot
	:public boost::enable_shared_from_this<Robot>,public ITCPProcessor
{
public:
	Robot(const string& id);
	Robot(SATCPConnectionPtr spConn);
	~Robot(void);
public:
	SPRobotPtr GetRobotPtr()
	{
		return shared_from_this();
	}
	string GetRobotID()
	{
		return m_sID;
	}
	string GetRobotName()
	{
		return m_sName;
	}
	void ProcessMessage(const SMsgHeader* header, const string& body);
	// 是否注册
	bool IsRegistered(){ return m_bIsRegistered; }
	// 是否有效
	bool IsValid();
	// 是否活跃
	bool IsActive();
	// 关闭
	bool Close();
	bool isCreateisElapsed(Poco::Int64 timeout);
	bool SendRequest2Robot(const string& sMessage, const string& sMsgid);
	bool SendGetStatusRequest();
	string GetRobotStatus(){
		return m_sRobotStatus;
	}
	void RecvData();
protected:

	void ProcessData();
	void Process_Register(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse);
	void Process_Heartbeat(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse);
	void Process_Status(rapidjson::Document& jsRequest);
	void Process_RequestCustomer(rapidjson::Document& jsRequest);
	bool SendMessage(const string& sMessage, UInt32 nInvokeID);
	void SetValid(bool isVaild);
private:
	SATCPConnectionPtr m_spSocket;
	string m_sID;
	string m_sName;
	string m_sClient;
	// 日志
	SANamedLogger log;
	bool m_bIsRegistered;
	// 创建时间
	Clock m_oCreate;
	// 最后一次收到消息时间
	Clock m_oLastRecvMsg;
	SAMap<int, string> m_req;
	Poco::AtomicCounter m_oAtomicCounter;
	// 状态信息
	string m_sRobotStatus;
};
#endif
