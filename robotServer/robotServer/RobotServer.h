#ifdef _MSC_VER
#pragma once
#endif
#ifndef _INC_ROBOTSERVER_H
#define _INC_ROBOTSERVER_H
#include "Poco/Util/ServerApplication.h"
#include "Poco/Timer.h"
#include <SATCPSocket.h>
#include <SAThreadPool.h>
#include "RobotManager.h"
#include "RobotHttpServer.h"
#include "SessionManager.h"
#include "TCPCommunication.h"
using namespace Poco::Util;
using namespace SA;

struct Configure
{
	// 日志级别
	string logLevel;
	// 数据连接串
	string databaseString;
	// http监听端口
	int HttpListenPort;

};

extern Configure cfg;

class RobotService : public ServerApplication
{
public:
	RobotService(void);
	~RobotService(void);
public:	// 取得实例
	static RobotService& instance();

protected:
	// 初始化
	void initialize(Application& self);
	// 释放
	void uninitialize();
	// 定义选项
	void defineOptions(OptionSet& options);
	// 处理帮助
	void handleHelp(const std::string& name, const std::string& value);
	// 显示帮助
	void displayHelp();
	// 入口
	int main(const std::vector<std::string>& args);
	bool ReloadConfig();
	bool ReloadDatabase();
public:
	// Robot TCP处理线程池
	SAThreadPool m_tpThreadPool;
	// websocket处理线程池
	SAThreadPool m_wsThreadPool;
	// 会话管理器
	RobotManager m_robotManager;
	// 定时器
	Timer m_tmTimer;
	// TCPServer
	SATCPServer m_tcpServer;
	// HTTP服务器
	RobotHttpServer m_HTTPServer;
	// Session 管理器
	SessionManager m_SessionManager;
	SAMap<string, CTCPCommunicationPtr> m_mapUnkownList;
	void OnTimer(Timer& timer);
	bool OnClientAccept(SATCPConnectionPtr spConn);
	void OnReceiveData(SATCPConnectionPtr spConn);
private:
	// 是否帮助请求
	bool _helpRequested;

};
#endif
