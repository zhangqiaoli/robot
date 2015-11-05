#ifdef _MSC_VER
#pragma once
#endif
#ifndef _INC_WEBSOCKETSESSION_H
#define _INC_WEBSOCKETSESSION_H
#include "WebSocketSession.h"
#include <SAWebSocket.h>
#include "rapidjson/document.h"

class WebSocketSession;
typedef boost::shared_ptr<WebSocketSession> WebSocketSessionPtr;
typedef boost::weak_ptr<WebSocketSession> WebSocketSessionWeakPtr;


class WebSocketSession
	:public boost::enable_shared_from_this < WebSocketSession >
{
public:
	WebSocketSession(SAWebSocketPtr spWebSocket);
	~WebSocketSession(void);
public:
	WebSocketSessionPtr GetSessionPtr()
	{
		return shared_from_this();
	}
	// 获取sessionID
	string GetSessionID()
	{
		return m_sSessionID;
	}
	string GetSessionName()
	{
		return m_sSessionName;
	}

	// 是否有效
	bool IsValid();
	// 是否活跃
	bool IsActive();
	// 关闭
	bool Close();
	// 发送消息
	bool SendMessage(const string& msg);
	void SendNotify(const string& sNotify);
	// 收到数据处理
	bool OnMessage(const string& text);
	// 处理消息
	void ProcessMesasge(rapidjson::Document& jsRequest, const string& sMsgID, string& sResponse);
	bool Execute_Login(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse);
	bool Execute_Logout(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse);
	bool Execute_GetRobotStatus(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse);
	bool Execute_ActiveRobot(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse);
	bool Execute_VoiceRobot(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse);
	bool Execute_StopRobot(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse);
	bool SetValid(bool bValid);
	bool ProcessWebSocketMessage(const string& text);
private:
	SAWebSocketPtr m_spWebSocket;
	Timestamp m_tsCreateTime;
	string m_sSessionID;
	string m_sSessionName;
	bool m_bValid;
	bool m_bLogined;
	SANamedLogger log;
	typedef boost::function<bool(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse)> TCommandProcessor;
	// 处理函数映射
	SAMap<string, TCommandProcessor> m_mapCommandProcessor;
};
#endif



