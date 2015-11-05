#include "stdafx.h"
#include "WebSocketSession.h"
#include "RobotServer.h"
#include "RobotCommon.h"

//class WSMessageTask
//{
//public:
//	WSMessageTask(WebSocketSession& wsSession, const string& sMsg)
//		:m_wsSession(wsSession), m_sMsg(sMsg){}
//	void operator()()
//	{
//		SA_PROTECTEDCODE_BEGIN;
//		m_wsSession.ProcessWebSocketMessage(m_sMsg);
//		SA_PROTECTEDCODE_ENDMSG("WSMessageTask处理消息过程中产生异常！")
//	}
//private:
//	WebSocketSession& m_wsSession;
//	const string m_sMsg;
//};

WebSocketSession::WebSocketSession(SAWebSocketPtr spWebSocket)
	:m_spWebSocket(spWebSocket), m_sSessionID(toLower(SAAssistant::GetGUID32())), m_sSessionName(format("WS[%s]", spWebSocket->PeerAddress().toString())), m_bValid(true), m_bLogined(false), log(format("WS[%s]", spWebSocket->PeerAddress().toString()), true)
{
	log.debug("session create");
	spWebSocket->SetTextMessageCallback(boost::bind(&WebSocketSession::OnMessage, this, _1));
	m_tsCreateTime.update();
	m_mapCommandProcessor.SetAt("login", boost::bind(&WebSocketSession::Execute_Login, this, _1, _2, _3));
	m_mapCommandProcessor.SetAt("logout", boost::bind(&WebSocketSession::Execute_Logout, this, _1, _2, _3));
	m_mapCommandProcessor.SetAt("get_robot_status", boost::bind(&WebSocketSession::Execute_GetRobotStatus, this, _1, _2, _3));
	m_mapCommandProcessor.SetAt("active_robot", boost::bind(&WebSocketSession::Execute_ActiveRobot, this, _1, _2, _3));
	m_mapCommandProcessor.SetAt("voice_robot", boost::bind(&WebSocketSession::Execute_VoiceRobot, this, _1, _2, _3));
	m_mapCommandProcessor.SetAt("stop_robot", boost::bind(&WebSocketSession::Execute_StopRobot, this, _1, _2, _3));
}


WebSocketSession::~WebSocketSession(void)
{
	log.info("WebSocketSession Destory");
}

bool WebSocketSession::Execute_Login(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse)
{
	log.trace("Execute_Login start...");
	string useid = "";
	string password = "";
	int nErrcode = 0;
	string sErrmsg = "ok";
	rapidjson::Document::AllocatorType& allocator = jsResponse.GetAllocator();
	if (jsRequest.HasMember("userid") && jsRequest["userid"].IsString())
	{
		useid = jsRequest["userid"].GetString();
	}
	if (jsRequest.HasMember("password") && jsRequest["password"].IsString())
	{
		password = jsRequest["password"].GetString();
	}
	if (useid == "robot_test" && password == "smartac2015")
	{
		// 判断用户名和密码是否正确
		m_bLogined = true;
	}
	else
	{
		nErrcode = 3;
		sErrmsg = "user or password is wrong!";
	}
	jsResponse.AddMember("errcode", nErrcode, allocator);
	jsResponse.AddMember("errmsg", sErrmsg.c_str(), allocator);
	sResponse = JsonDocToString(jsResponse);
	log.trace("Execute_Login end...");
	return true;
}
bool WebSocketSession::Execute_Logout(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse)
{
	log.trace("Execute_Logout start...");
	int nErrcode = 0;
	string sErrmsg = "ok";
	rapidjson::Document::AllocatorType& allocator = jsResponse.GetAllocator();

	jsResponse.AddMember("errcode", nErrcode, allocator);
	jsResponse.AddMember("errmsg", sErrmsg.c_str(), allocator);
	sResponse = JsonDocToString(jsResponse);
	log.trace("Execute_Logout end...");

	return true;
}
bool WebSocketSession::Execute_GetRobotStatus(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse)
{
	log.trace("Execute_GetRobotStatus start...");
	int nErrcode = 0;
	string sErrmsg = "ok";
	bool isAll = false;
	vector<string> robotidArray;
	rapidjson::Document status;
	status.SetObject();
	if (jsRequest.HasMember("robots") && jsRequest["robots"].IsArray())
	{
		rapidjson::Value &dataArray = jsRequest["robots"];
		for (rapidjson::SizeType i = 0; i < dataArray.Size(); i++)
		{
			const rapidjson::Value& robotidItem = dataArray[i];
			string robotid = robotidItem.GetString();
			if (robotid == "all")
			{
				isAll = true;
				break;
			}
			else
			{
				robotidArray.push_back(robotid);
			}
		}
	}

	rapidjson::Document::AllocatorType& allocator = jsResponse.GetAllocator();
	rapidjson::Value jsArrRoot(rapidjson::kArrayType);
	if (isAll)
	{
		SAArray<SPRobotPtr> robotArr;
		RobotService::instance().m_robotManager.GetAllRobot(robotArr);
		for (SAArray<SPRobotPtr>::Iterator it = robotArr.GetIterator(); !it.IsEnd();it.MoveNext())
		{
			SPRobotPtr robot = it.Get();
			if (robot)
			{
				// 获取robot状态
				//rapidjson::Value jsRobot(rapidjson::kObjectType);
				//rapidjson::Value jstmp;
				//jstmp.SetString(robot->GetRobotID().c_str(), allocator);
				//jsRobot.AddMember("robot_id", jstmp, allocator);
				//jsRobot.AddMember("location_x", "server", allocator);
				//jsRobot.AddMember("location_y", "server.ctl.active", allocator);
				//jsRobot.AddMember("speed", 2, allocator);
				//jsRobot.AddMember("acceleration", 2, allocator);
				//jsRobot.AddMember("temperature", 2, allocator);
				//jsRobot.AddMember("electric_quantity", 0, allocator);
				//jsRobot.AddMember("active_status", "moving", allocator);
				string sRobotStatus = robot->GetRobotStatus();
				if (sRobotStatus.length() > 0)
				{

					if (!status.Parse<0>(sRobotStatus.c_str()).HasParseError()
						&& status.IsObject())
					{
						rapidjson::Value& jstmp = status;
						jsArrRoot.PushBack(jstmp, allocator);
					}
				}
				else
				{
					// robot无状态信息
					rapidjson::Value jsRobot(rapidjson::kObjectType);
					rapidjson::Value jstmp;
					jstmp.SetString(robot->GetRobotID().c_str(), allocator);
					jsRobot.AddMember("robot_id", jstmp, allocator);
					jsArrRoot.PushBack(jsRobot, allocator);
				}
			}
		}
	}
	else
	{
		for (size_t i = 0; i < robotidArray.size();i++)
		{
			string sRobot = robotidArray[i];
			SPRobotPtr robot = RobotService::instance().m_robotManager.FindRobot(sRobot);
			if (robot)
			{
				string sRobotStatus = robot->GetRobotStatus();
				if (sRobotStatus.length() > 0)
				{

					if (!status.Parse<0>(sRobotStatus.c_str()).HasParseError()
						&& status.IsObject())
					{
						rapidjson::Value& jstmp = status;
						jsArrRoot.PushBack(jstmp, allocator);
					}
				}
				else
				{
					// robot无状态信息
					rapidjson::Value jsRobot(rapidjson::kObjectType);
					rapidjson::Value jstmp;
					jstmp.SetString(robot->GetRobotID().c_str(), allocator);
					jsRobot.AddMember("robot_id", jstmp, allocator);
					jsArrRoot.PushBack(jsRobot, allocator);
				}
			}
		}
	}
	jsResponse.AddMember("robots", jsArrRoot, allocator);
	jsResponse.AddMember("errcode", nErrcode, allocator);
	jsResponse.AddMember("errmsg", sErrmsg.c_str(), allocator);
	sResponse = JsonDocToString(jsResponse);
	log.trace("Execute_GetRobotStatus end...");
	return true;
}
bool WebSocketSession::Execute_ActiveRobot(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse)
{
	log.trace("Execute_ActiveRobot start...");
	string sRobotID = "";
	int nErrcode = 0;
	string sErrmsg = "ok";
	rapidjson::Document::AllocatorType& allocator = jsResponse.GetAllocator();
	if (jsRequest.HasMember("robot_id") && jsRequest["robot_id"].IsString())
	{
		sRobotID = jsRequest["robot_id"].GetString();
	}
	if (jsRequest.HasMember("active_mode") && jsRequest["active_mode"].IsObject())
	{
		rapidjson::Value &active_mode = jsRequest["active_mode"];
		SPRobotPtr robot = RobotService::instance().m_robotManager.FindRobot(sRobotID);
		if (robot)
		{
			// 发送控制机器人运动请求
			rapidjson::Document request2Robot;
			request2Robot.SetObject();
			rapidjson::Document::AllocatorType& allocator2R = request2Robot.GetAllocator();
			request2Robot.AddMember("sender", "server1", allocator2R);
			request2Robot.AddMember("sendertype", "server", allocator2R);
			request2Robot.AddMember("msgid", "server.ctl.active", allocator2R);
			request2Robot.AddMember("body", active_mode, allocator2R);
			string sToRobotMsg = JsonDocToString(request2Robot);
			robot->SendRequest2Robot(sToRobotMsg, "server.ctl.active");
		}
		else
		{
			nErrcode = 4;
			sErrmsg = "Can't find robot by robotid";
		}
	}
	jsResponse.AddMember("errcode", nErrcode, allocator);
	jsResponse.AddMember("errmsg", sErrmsg.c_str(), allocator);
	sResponse = JsonDocToString(jsResponse);
	log.trace("Execute_ActiveRobot end...");
	return true;
}
bool WebSocketSession::Execute_VoiceRobot(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse)
{
	log.trace("Execute_VoiceRobot start...");
	string sRobotID = "";
	int nErrcode = 0;
	string sErrmsg = "ok";
	rapidjson::Document::AllocatorType& allocator = jsResponse.GetAllocator();
	if (jsRequest.HasMember("robot_id") && jsRequest["robot_id"].IsString())
	{
		sRobotID = jsRequest["robot_id"].GetString();
	}
	if (jsRequest.HasMember("voice_mode") && jsRequest["voice_mode"].IsObject())
	{
		rapidjson::Value &voice_mode = jsRequest["voice_mode"];
		SPRobotPtr robot = RobotService::instance().m_robotManager.FindRobot(sRobotID);
		if (robot)
		{
			// 发送控制机器人说话
			rapidjson::Document request2Robot;
			request2Robot.SetObject();
			rapidjson::Document::AllocatorType& allocator2R = request2Robot.GetAllocator();
			request2Robot.AddMember("sender", "server1", allocator2R);
			request2Robot.AddMember("sendertype", "server", allocator2R);
			request2Robot.AddMember("msgid", "server.ctl.voice", allocator2R);
			request2Robot.AddMember("body", voice_mode, allocator2R);
			string sToRobotMsg = JsonDocToString(request2Robot);
			robot->SendRequest2Robot(sToRobotMsg, "server.ctl.voice");
		}
		else
		{
			nErrcode = 4;
			sErrmsg = "Can't find robot by robotid";
		}
	}
	jsResponse.AddMember("errcode", nErrcode, allocator);
	jsResponse.AddMember("errmsg", sErrmsg.c_str(), allocator);
	sResponse = JsonDocToString(jsResponse);
	log.trace("Execute_VoiceRobot end...");
	return true;
}
bool WebSocketSession::Execute_StopRobot(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse)
{
	log.trace("Execute_StopRobot start...");
	string sRobotID = "";
	int nErrcode = 0;
	string sErrmsg = "ok";
	rapidjson::Document::AllocatorType& allocator = jsResponse.GetAllocator();
	if (jsRequest.HasMember("robot_id") && jsRequest["robot_id"].IsString())
	{
		sRobotID = jsRequest["robot_id"].GetString();
	}

	rapidjson::Value body(rapidjson::kObjectType);
	SPRobotPtr robot = RobotService::instance().m_robotManager.FindRobot(sRobotID);
	if (robot)
	{
		// 发送停止机器人运动请求
		rapidjson::Document request2Robot;
		request2Robot.SetObject();
		rapidjson::Document::AllocatorType& allocator2R = request2Robot.GetAllocator();
		request2Robot.AddMember("sender", "server1", allocator2R);
		request2Robot.AddMember("sendertype", "server", allocator2R);
		request2Robot.AddMember("msgid", "server.stop.active", allocator2R);
		request2Robot.AddMember("body", body, allocator2R);
		string sToRobotMsg = JsonDocToString(request2Robot);
		robot->SendRequest2Robot(sToRobotMsg, "server.stop.active");
	}
	else
	{
		nErrcode = 4;
		sErrmsg = "Can't find robot by robotid";
	}

	jsResponse.AddMember("errcode", nErrcode, allocator);
	jsResponse.AddMember("errmsg", sErrmsg.c_str(), allocator);
	sResponse = JsonDocToString(jsResponse);
	log.trace("Execute_StopRobot end...");
	return true;
}

bool WebSocketSession::OnMessage(const string& text)
{
	string msg(text.begin(), text.end());
	log.info("RECV:" + msg);
	RobotService::instance().m_wsThreadPool.addTask(boost::bind(&WebSocketSession::ProcessWebSocketMessage, this, msg));
	return true;
}

bool WebSocketSession::ProcessWebSocketMessage(const string& msg)
{
	rapidjson::Document request;
	string sRes = "";
	string sInvokeid = "";
	string sErrorMsg = "ok";
	int nErrCode = 0;
	string sMsgID = "";
	rapidjson::Document response;
	response.SetObject();

	if (!request.Parse<0>(msg.c_str()).HasParseError()
		&& request.IsObject())
	{
		if (request.HasMember("msgid") && request["msgid"].IsString())
		{
			sMsgID = request["msgid"].GetString();
		}
		if (request.HasMember("invoke_id") && request["invoke_id"].IsString())
		{
			sInvokeid = request["invoke_id"].GetString();
		}
		response.AddMember("msgid", sMsgID.c_str(), response.GetAllocator());
		response.AddMember("invoke_id", sInvokeid.c_str(), response.GetAllocator());
		TCommandProcessor fCommandProc;
		m_mapCommandProcessor.Lookup(sMsgID, fCommandProc);
		if (m_bLogined || sMsgID == "login")
		{
			if (fCommandProc)
			{
				log.debug(format("process msg %s...", sMsgID));
				fCommandProc(request, response, sRes);
			}
			else
			{
				nErrCode = 1;
				sErrorMsg = "Unkown msgid";
				log.warning(format("Unkown msgid,%s", sMsgID));
				response.AddMember("errcode", nErrCode, response.GetAllocator());
				response.AddMember("errmsg", sErrorMsg.c_str(), response.GetAllocator());
			}
		}
		else
		{
			nErrCode = 10;
			sErrorMsg = "Has not login";
			log.warning("Has not login");
			response.AddMember("errcode", nErrCode, response.GetAllocator());
			response.AddMember("errmsg", sErrorMsg.c_str(), response.GetAllocator());
		}

	}
	else
	{
		nErrCode = 2;
		sErrorMsg = "invalid json format!";
		log.warning("ws request format error,invalid json format");
		response.AddMember("errcode", nErrCode, response.GetAllocator());
		response.AddMember("errmsg", sErrorMsg.c_str(), response.GetAllocator());
	}
	if (sRes.empty())
	{
		sRes = JsonDocToString(response);
	}
	SendMessage(sRes);
	if (sMsgID == "logout" && nErrCode == 0)
	{
		SetValid(false);
	}
	return true;
}

bool WebSocketSession::IsValid()
{
	if (m_bValid &&  m_spWebSocket->IsRuning())
	{
		return true;
	}
	return false;
}

bool WebSocketSession::IsActive()
{
	if (m_bValid &&  m_spWebSocket->IsRuning())
	{
		return true;
	}
	return false;
}

bool WebSocketSession::Close()
{
	SAWebSocketPtr spWebSocket = m_spWebSocket;
	if (spWebSocket)
	{
		spWebSocket->CloseWebSocket();
		return spWebSocket->Stop(0);
	}
	return true;
}

bool WebSocketSession::SendMessage(const string& msg)
{
	if (m_spWebSocket->IsRuning())
	{
		log.debug(format("SEND: %s", msg));
		return m_spWebSocket->SendMessage(msg);
	}
	return false;
}

void WebSocketSession::ProcessMesasge(rapidjson::Document& jsRequest, const string& sMsgID, string& sResponse)
{
	//WebSocketSession::ProcessMesasge(jsRequest, sMsgID, sResponse);
}

void WebSocketSession::SendNotify(const string& sNotify)
{
	if (m_spWebSocket)
	{
		SendMessage(sNotify);
	}
	else
	{
		SALOG.warning("send notify failed,maybe connection break!");
	}
}

bool WebSocketSession::SetValid(bool bValid)
{
	m_bValid = bValid;
	return true;
}
