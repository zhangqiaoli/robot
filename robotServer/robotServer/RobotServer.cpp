#include "stdafx.h"
#include "RobotServer.h"
#include <Poco/NamedMutex.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/HelpFormatter.h>
#include <SADatabase.h>
#include "SALogChannel.h"
#include "JoyStickClient.h"

using namespace Poco;
using namespace Poco::Util;
Configure cfg;

RobotService::RobotService(void) : _helpRequested(false)
{
}


RobotService::~RobotService(void)
{
}
// 定义选项
void RobotService::defineOptions(OptionSet& options)
{
	ServerApplication::defineOptions(options);

	options.addOption(
		Option("help", "h", "display help information on command line arguments")
		.required(false)
		.repeatable(false)
		.callback(OptionCallback<RobotService>(this, &RobotService::handleHelp)));
}
// 处理帮助
void RobotService::handleHelp(const std::string& name, const std::string& value)
{
	_helpRequested = true;
	displayHelp();
	stopOptionsProcessing();
}
// 显示帮助
void RobotService::displayHelp()
{
	HelpFormatter helpFormatter(options());
	helpFormatter.setCommand(commandName());
	helpFormatter.setUsage("OPTIONS");
	helpFormatter.setHeader("Robot Service application.");
	helpFormatter.format(std::cout);
}
// 初始化
void RobotService::initialize(Application& self)
{
	if (!_helpRequested)
	{
		//loadConfiguration(); // load default configuration files, if present
		Path oPath;
		oPath.parseDirectory(SAAssistant::GetApplicationPath());
		oPath = oPath.parent();
		oPath.setFileName("robot.ini");
		loadConfiguration(oPath.toString(Path::PATH_NATIVE)); // load default configuration files, if present
		ServerApplication::initialize(self);
		SALogChannel::GetInstance().EnableFileLog("$(ApplicationPath)../log/robot_$(Year4)$(Month2)$(Day2)_$(FileNumber2).log");
		logger().information("RobotService starting...");

		// 重复运行检查
		NamedMutex nm("RobotService");
		if (!nm.tryLock())
		{
			throw ApplicationException("RobotService already running!");
		}
		// 加载配置文件
		if (!ReloadConfig())
		{
			throw ApplicationException("Load Config Failed!");
		}
		m_tcpServer.SetPort(18020);
		m_tcpServer.SetClientCallback(boost::bind(&RobotService::OnClientAccept, this, _1));
		m_tcpServer.Listen();
		m_robotManager.Init();  
		m_HTTPServer.Init();
		m_HTTPServer.Start();
		SAWebSocket::InitializeDefaultReactor();
	}
}

// 入口
int RobotService::main(const std::vector<std::string>& args)
{
	if (!_helpRequested)
	{
		logger().information("RobotService Start Running...");
		m_tmTimer.setStartInterval(10000);
		m_tmTimer.setPeriodicInterval(5000);
		m_tmTimer.start(TimerCallback<RobotService>(*this, &RobotService::OnTimer));
		m_tpThreadPool.start();
		m_wsThreadPool.start();
		waitForTerminationRequest();
		m_tmTimer.stop();
		m_tpThreadPool.stop();
		m_wsThreadPool.stop();
	}
	return Application::EXIT_OK;
}
// 释放
void RobotService::uninitialize()
{
	logger().information("RobotService Cleaning...");
	m_tpThreadPool.stop();
	m_wsThreadPool.stop();
	m_HTTPServer.Stop();
	m_HTTPServer.Free();
	ServerApplication::uninitialize();
}

bool RobotService::ReloadConfig()
{
	cfg.databaseString = config().getString("ROBOT.Database", "");
	cfg.logLevel = config().getString("ROBOT.LogLevel", "debug");
	cfg.HttpListenPort = config().getInt("ROBOT.HttpListenPort", 18030);
	SALOG.setLevel(cfg.logLevel);
	return true;
}

bool RobotService::OnClientAccept(SATCPConnectionPtr spConn)
{
	SALOG.debug(format("OnClientAccept start...remote='%s'",spConn->PeerAddress().toString()));
	string sClient = spConn->PeerAddress().toString();
	CTCPCommunicationPtr spComm = CTCPCommunicationPtr(new CTCPCommunication(spConn));
	if (spComm)
	{
		if (spComm->Init())
		{
			m_mapUnkownList.SetAt(sClient,spComm);
		}
		else
		{
			SALOG.debug("TCP Communication init failed!");
		}
	}
	else
	{
		SALOG.debug("TCP Communication new failed!");
	}
	//SPRobotPtr robot = SPRobotPtr(new Robot(spConn));
	//m_mapUnkownList.SetAt(sClient, robot);
	//spConn->SetOnData(boost::bind(&RobotService::OnReceiveData, this, spConn));
	return true;
}

void RobotService::OnReceiveData(SATCPConnectionPtr spConn)
{
	//SALOG.debug("RobotService OnReceiveData process start...");
	//SATCPConnectionPtr spSocket = spConn;
	//if (spSocket)
	//{
	//	size_t nReadableCount = spSocket->Readable();
	//	if (nReadableCount >= sizeof(SMsgHeader))
	//	{
	//		SMsgHeader head;
	//		spSocket->Read(&head, sizeof(head), false);
	//		head.nProtocolFlag = ntohl(head.nProtocolFlag);
	//		head.nMessageLength = ntohl(head.nMessageLength);
	//		head.nInvokeID = ntohl(head.nInvokeID);
	//		if (head.nProtocolFlag == PROTOCOLFLAG_ROBOTCONNECTION)
	//		{
	//			SPRobotPtr robot = SPRobotPtr(new Robot(spConn));
	//			m_mapUnkownList.SetAt(spConn->PeerAddress().toString(), robot);
	//			if (head.nMessageLength <= nReadableCount)
	//			{
	//				if (robot)
	//				{
	//					robot->RecvData();
	//				}
	//			}
	//		}
	//		else if (head.nProtocolFlag == PROTOCOLFLAG_ROBOTAGENTCONNECTION)
	//		{
	//			SPJoyStickClientPtr spJoystick = SPJoyStickClientPtr(new JoyStickClient(spConn));
	//			if (head.nMessageLength <= nReadableCount)
	//			{
	//				if (spJoystick)
	//				{
	//					spJoystick->RecvData();
	//				}
	//			}
	//		}
	//		else
	//		{
	//			SALOG.error("Msg header is error,ingore it!");
	//			spSocket->Close();
	//		}
	//	}
	//}
	//else
	//{
	//	SALOG.debug("what?? no readable data??");
	//}
}

void RobotService::OnTimer(Timer& timer)
{
	for (SAMap<string, CTCPCommunicationPtr>::Iterator it = m_mapUnkownList.GetIterator(); !it.IsEnd();)
	{
		CTCPCommunicationPtr spClient = it.GetValue();
		if (!spClient->IsValid())
		{
			m_mapUnkownList.RemoveAt(it);
		}
		else
		{
			it.MoveNext();
		}
	}
	m_robotManager.CheckRobot();
	m_SessionManager.CheckSession();
}

RobotService& RobotService::instance()
{
	return (RobotService&)Application::instance();
}




POCO_SERVER_MAIN(RobotService)
