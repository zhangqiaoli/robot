#ifdef _MSC_VER
#pragma once
#endif
#ifndef _INC_SESSIONMANAGER_H
#define _INC_SESSIONMANAGER_H

#include "WebSocketSession.h"

class SessionManager
{
public:
	SessionManager(void);
	~SessionManager(void);
public:
	// 初始化
	bool Init();
	// 释放
	void Free();
	// 创建WebSocketSession
	static WebSocketSessionPtr CreateSession(SAWebSocketPtr spWebSocket);
	// 创建HTTPSession
	static WebSocketSessionPtr CreateSession(const string& id);
	// 添加一个Session
	void AddSession(WebSocketSessionPtr spSession);
	// 移除一个Session
	void RemoveSession(WebSocketSessionPtr spSession);
	// 检查
	void CheckSession();
	// 查找Session
	WebSocketSessionPtr FindSession(const string& sSessionID);
	// 获取所有Session
	void GetAllSession(vector<WebSocketSessionPtr>& arrSession);
private:
	SAMap<string, WebSocketSessionPtr> m_mapSession;
};
#endif

