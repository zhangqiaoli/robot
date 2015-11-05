#include "stdafx.h"
#include "FQAServer.h"
#include <Poco/NamedMutex.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/HelpFormatter.h>
#include <SADatabase.h>
#include "SALogChannel.h"
#include "FQACommon.h"

using namespace Poco;
using namespace Poco::Util;
Configure cfg;

FQAService::FQAService(void) : _helpRequested(false)
{
}


FQAService::~FQAService(void)
{
}
// 定义选项
void FQAService::defineOptions(OptionSet& options)
{
	ServerApplication::defineOptions(options);

	options.addOption(
		Option("help", "h", "display help information on command line arguments")
		.required(false)
		.repeatable(false)
		.callback(OptionCallback<FQAService>(this, &FQAService::handleHelp)));
}
// 处理帮助
void FQAService::handleHelp(const std::string& name, const std::string& value)
{
	_helpRequested = true;
	displayHelp();
	stopOptionsProcessing();
}
// 显示帮助
void FQAService::displayHelp()
{
	HelpFormatter helpFormatter(options());
	helpFormatter.setCommand(commandName());
	helpFormatter.setUsage("OPTIONS");
	helpFormatter.setHeader("Robot Service application.");
	helpFormatter.format(std::cout);
}
// 初始化
void FQAService::initialize(Application& self)
{
	if (!_helpRequested)
	{
		//loadConfiguration(); // load default configuration files, if present
		Path oPath;
		oPath.parseDirectory(SAAssistant::GetApplicationPath());
		oPath = oPath.parent();
		oPath.setFileName("faq.ini");
		loadConfiguration(oPath.toString(Path::PATH_NATIVE)); // load default configuration files, if present
		ServerApplication::initialize(self);
		SALogChannel::GetInstance().EnableFileLog("$(ApplicationPath)../log/faq_$(Year4)$(Month2)$(Day2)_$(FileNumber2).log");
		logger().information("FaqService starting...");

		// 重复运行检查
		NamedMutex nm("FaqService");
		if (!nm.tryLock())
		{
			throw ApplicationException("FaqService already running!");
		}
		// 加载配置文件
		if (!ReloadConfig())
		{
			throw ApplicationException("Load Config Failed!");
		}
		m_httpServer.m_Options.sListenAddress = F("%d", cfg.httpListenPort);
		m_httpServer.AddRouter("/question-answering/get_answer", boost::bind(&FQAService::OnReceiveHttpRequest, this, _1));
		m_httpServer.Start();
	}
}

int FQAService::OnReceiveHttpRequest(struct mg_connection* conn)
{
	string sContent = "";
	string sRemoteip = "";
	if (conn)
	{
		if (conn->remote_ip)
		{
			sRemoteip = conn->remote_ip;
		}
		if (conn->content_len > 0)
		{
			char* content = (char *)malloc(conn->content_len + 1);
			memset(content, 0, conn->content_len + 1);
			strncpy(content, conn->content, conn->content_len);
			sContent = content;
		}
		else
		{
			SALOG.debug(format("OnReceiveHttpRequest start...remote=%s:%u,content is empty!", sRemoteip, conn->remote_port));
			return 1;
		}
	}
	SALOG.debug(format("OnReceiveHttpRequest start...remote=%s:%u,RECV='%s'", sRemoteip, conn->remote_port, sContent));
	bool ret = false;
	string sAnswer = "";
	int nErrcode = 0;
	string sErrmsg = "ok";
	if (!sContent.empty())
	{
		rapidjson::Document oDocument;
		if (!oDocument.Parse<0>(sContent.c_str()).HasParseError()
			&& oDocument.IsObject())
		{
			if (oDocument.HasMember("source") && oDocument["source"].IsString())
			{
				string sSource = oDocument["source"].GetString();
				if (Poco::toLower(sSource) == "xfyun")
				{
					if (oDocument.HasMember("text") && oDocument["text"].IsObject())
					{
						rapidjson::Value& text = oDocument["text"];
						ret = xfyunSemanticProcess(text, sAnswer, nErrcode, sErrmsg);
						if (!ret)
						{
							nErrcode = 6;
							sErrmsg = "xfyunSemanticProcess failed!";
							SALOG.debug(sErrmsg);
						}
					}
					else
					{
						nErrcode = 1;
						sErrmsg = "data format invalid,miss text node";
						SALOG.debug(sErrmsg);
					}
				}
			}
			else
			{
				nErrcode = 1;
				sErrmsg = "data format invalid,miss source node";
				SALOG.debug(sErrmsg);
			}
		}
		else
		{
			nErrcode = 2;
			sErrmsg = "invalid request data!";
			SALOG.debug("invalid request data,not json format!");
		}
	}
	rapidjson::Document response;
	response.SetObject();
	rapidjson::Document::AllocatorType& allocator = response.GetAllocator();
	response.AddMember("errcode", nErrcode, allocator);
	response.AddMember("errmsg", sErrmsg.c_str(), allocator);
	response.AddMember("answer", sAnswer.c_str(), allocator);
	string sResponse = JsonDocToString(response);
	m_httpServer.HTTP_SendResponse(conn, 200, "OK", sResponse);
	SALOG.debug(F("Send http response:%s", sResponse));
	return 0;
}
bool FQAService::xfyunSemanticProcess(rapidjson::Value& text, string& answer, int& errcode, string& errmsg)
{
	bool ret = false;
	if (text.HasMember("rc") && text["rc"].IsInt())
	{
		int rc = text["rc"].GetInt();
		if (rc == 0)
		{
			if (text.HasMember("service") && text["service"].IsString())
			{
				string sReqid = text["service"].GetString();
				bool result = GetAnswerByReqID(sReqid, answer);
				if (!result)
				{
					errcode = 5;
					errmsg = "GetAnswerByID failed!";
					SALOG.debug(F("GetAnswerByID '%s' failed!", sReqid));
				}
				else
				{
					ret = true;
				}
			}
			//if (text.HasMember("semantic") && text["semantic"].IsObject())
			//{
			//	rapidjson::Value& semantic = text["semantic"];
			//	if (semantic.HasMember("slots") && semantic["slots"].IsObject())
			//	{
			//		rapidjson::Value& slots = semantic["slots"];
			//		if (slots.HasMember("reqid") && slots["reqid"].IsString())
			//		{
			//			string sReqid = slots["reqid"].GetString();
			//			bool result = GetAnswerByReqID(sReqid, answer);
			//			if (!result)
			//			{
			//				errcode = 5;
			//				errmsg = "GetAnswerByID failed!";
			//				SALOG.debug(F("GetAnswerByID '%s' failed!", sReqid));
			//			}
			//			else
			//			{
			//				ret = true;
			//			}
			//		}
			//	}
			//}
		}
		else
		{
			errcode = 4;
			errmsg = "xfyun Semantic parse failed!";
			SALOG.debug(errmsg);
		}
	}
	else
	{
		errcode = 3;
		errmsg = "xfyun Semantic lack node rc,invalid data!";
		SALOG.debug(errmsg);
	}
	return ret;
}

bool FQAService::GetAnswerByReqID(const string& sAnswerid, string& sAnswer)
{
	SADBConnectorPtr db = SADBConnector::CreateConnector();
	db->SetConnectString(cfg.databaseString);
	SASQLMaker sm(F("select answer.content from answer left join question on question.answerid =answer.id where question.id = '%s'", sAnswerid));
	db->ExecuteSQL(sm);
	SADBRecordSet rs(db);
	if (rs.SelectSQL(sm))
	{
		if (!rs.IsEmpty())
		{
			rs.GetField("content", sAnswer);
			SALOG.debug(F("get answer:%s by id %s", sAnswer, sAnswerid));
			return true;
		}
	}
	else
	{
		SALOG.error("GetAnswerByID, SelectSQL failed");
		return false;
	}
	return false;
}

// 入口
int FQAService::main(const std::vector<std::string>& args)
{
	if (!_helpRequested)
	{
		logger().information("FaqService Start Running...");
		m_tmTimer.setStartInterval(10000);
		m_tmTimer.setPeriodicInterval(5000);
		m_tmTimer.start(TimerCallback<FQAService>(*this, &FQAService::OnTimer));
		waitForTerminationRequest();
		m_tmTimer.stop();
	}
	return Application::EXIT_OK;
}
// 释放
void FQAService::uninitialize()
{
	logger().information("FaqService Cleaning...");
	ServerApplication::uninitialize();
}

bool FQAService::ReloadConfig()
{
	cfg.databaseString = config().getString("FAQ.Database", "");
	cfg.logLevel = config().getString("FAQ.LogLevel", "debug");
	cfg.httpListenPort = config().getInt("FAQ.HttpListenPort", 18030);
	SALOG.setLevel(cfg.logLevel);
	return true;
}

void FQAService::OnTimer(Timer& timer)
{
}

FQAService& FQAService::instance()
{
	return (FQAService&)Application::instance();
}




POCO_SERVER_MAIN(FQAService)
