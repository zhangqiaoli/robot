#ifdef _MSC_VER
#pragma once
#endif
#ifndef _INC_ROBOTHTTPSERVER_H
#define _INC_ROBOTHTTPSERVER_H
#include "SAHTTPServer.h"
#include <SAWebSocket.h>
#include "WebSocketSession.h"
#include <Poco/Timer.h>
using namespace SA;

//class SessionVisitor
//{
//public:
//	SessionVisitor(SAWebSocket& socket);;
//	~SessionVisitor();
//public:
//	void ProceeMessage(const TSTLBuffer& buff);
//	void Check(){
//		if (!m_spSession && m_Clock.elapsed()>30000)
//		{
//			m_Socket.Stop(0);
//		}
//	}
//protected:
//private:
//	SAWebSocket& m_Socket;
//	SPClientSessionPtr m_spSession;
//	Clock m_Clock;
//};


class RobotHttpServer
{
public:
	RobotHttpServer(void);
	~RobotHttpServer(void);
public:
	// 初始化
	bool Init();
	// 释放
	void Free();
	// 启动
	bool Start();
	// 停止
	void Stop(UInt32 nTimeout = 3000);
protected:
	// 处理WS连接
	bool HandleWSConnect(HTTPServerRequest& request, HTTPServerResponse& response);

	bool HandleRequest(HTTPServerRequest& req, HTTPServerResponse& resp);
	// 发送http回应
	bool HTTP_SendResponse(HTTPServerResponse& resp, int nStatusCode, const string& sStatusText, const string& sBody);
private:
	// HTTPServer
	SAHTTPServer m_HTTPServer;
};
#endif
