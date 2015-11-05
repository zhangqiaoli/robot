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
	// ��־����
	string logLevel;
	// �������Ӵ�
	string databaseString;
	// http�����˿�
	int httpListenPort;

};

extern Configure cfg;

class FQAService : public ServerApplication
{
public:
	FQAService(void);
	~FQAService(void);
public:	// ȡ��ʵ��
	static FQAService& instance();

protected:
	// ��ʼ��
	void initialize(Application& self);
	// �ͷ�
	void uninitialize();
	// ����ѡ��
	void defineOptions(OptionSet& options);
	// �������
	void handleHelp(const std::string& name, const std::string& value);
	// ��ʾ����
	void displayHelp();
	// ���
	int main(const std::vector<std::string>& args);
	bool ReloadConfig();
	bool ReloadDatabase();
public:
	// ��ʱ��
	Timer m_tmTimer;
	void OnTimer(Timer& timer);
	int OnReceiveHttpRequest(struct mg_connection* conn);
	bool xfyunSemanticProcess(rapidjson::Value& text, string& answer, int& errcode, string& errmsg);
	bool GetAnswerByReqID(const string& sAnswerid, string& sAnswer);
private:
	// �Ƿ��������
	bool _helpRequested;
	// httpserver
	SAMongooseServer m_httpServer;

};
#endif
