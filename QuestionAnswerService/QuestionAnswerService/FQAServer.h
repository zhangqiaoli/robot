#ifdef _MSC_VER
#pragma once
#endif
#ifndef _INC_FQASERVER_H
#define _INC_FQASERVER_H
#include "Poco/Util/ServerApplication.h"
#include "Poco/Timer.h"
#include <SAThreadPool.h>
#include <SAMongooseServer.h>
#include "rapidjson/document.h"
using namespace Poco::Util;
using namespace SA;

struct Configure
{
	// 日志级别
	string logLevel;
	// 数据连接串
	string databaseString;
	// http监听端口
	int httpListenPort;

};

extern Configure cfg;

class FQAService : public ServerApplication
{
public:
	FQAService(void);
	~FQAService(void);
public:	// 取得实例
	static FQAService& instance();

protected:
	// 初始匿
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
	// 定时器
	Timer m_tmTimer;
	void OnTimer(Timer& timer);
	int OnReceiveHttpRequest(struct mg_connection* conn);
	bool xfyunSemanticProcess(rapidjson::Value& text, string& answer, int& errcode, string& errmsg);
	bool GetAnswerByReqID(const string& sAnswerid, string& sAnswer);
private:
	// 是否帮助请求
	bool _helpRequested;
	// httpserver
	SAMongooseServer m_httpServer;

};
#endif
