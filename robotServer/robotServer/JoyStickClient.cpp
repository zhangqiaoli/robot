#include "stdafx.h"
#include "JoyStickClient.h"
#include "RobotServer.h"
#include <Poco/Net/SocketAddress.h>
//class MessageTask
//{
//public:
//	MessageTask(JoyStickClient& JoyStickClient, const SJoyStickClientHeader* header,const string& sBody)
//		:m_JoyStickClient(JoyStickClient), m_header(header), m_sBody(sBody){}
//	void operator()()
//	{
//		SA_PROTECTEDCODE_BEGIN;
//		m_JoyStickClient.ProcessMessage(m_header, m_sBody);
//		SA_PROTECTEDCODE_ENDMSG("MessageTask处理消息过程中产生异常！")
//	}
//private:
//	JoyStickClient& m_JoyStickClient;
//	const SJoyStickClientHeader* m_header;
//	const string m_sBody;
//};

JoyStickClient::JoyStickClient(const string& id)
{
	m_sID = id;
}

JoyStickClient::JoyStickClient(SATCPConnectionPtr spConn) :m_spSocket(spConn), log(format("JoyStickClient[%s]", spConn->PeerAddress().toString())), m_bIsRegistered(false)
{
	m_sClient = spConn->PeerAddress().toString();
	m_sID = "";
}

JoyStickClient::~JoyStickClient(void)
{

}

bool JoyStickClient::IsValid()
{
	Clock o;
	if (!m_bIsRegistered && m_oCreate.isElapsed(180 * 1000 * 1000))
	{ // 没注册,并且距离创建超过3分钟,无效连接,需断开
		return false;
	}
	else if (m_oLastRecvMsg.isElapsed(60 * 1000 * 1000))
	{ // 超过1分钟没收到消息
		return false;
	}
	return m_spSocket->IsValid();
}

bool JoyStickClient::IsActive()
{
	return m_spSocket->IsValid();
}

bool JoyStickClient::Close()
{
	SATCPConnectionPtr spSocket = m_spSocket;
	if (spSocket)
	{
		spSocket->Close();
		log.debug("close");
		return true;
	}
	return true;
}

void JoyStickClient::RecvData()
{
	SATCPConnectionPtr spSocket = m_spSocket;
	if (spSocket)
	{
		for (size_t nReadableCount = spSocket->Readable(); nReadableCount >= sizeof(SMsgHeader); nReadableCount = spSocket->Readable())
		{
			SMsgHeader head;
			spSocket->Read(&head, sizeof(head), false);
			head.nProtocolFlag = ntohl(head.nProtocolFlag);
			head.nMessageLength = ntohl(head.nMessageLength);
			head.nInvokeID = ntohl(head.nInvokeID);
			if (head.nProtocolFlag == PROTOCOLFLAG_ROBOTAGENTCONNECTION)
			{
				if (head.nMessageLength <= nReadableCount)
				{
					TSTLBufferPtr spBuffer = TSTLBufferPtr(new TSTLBuffer);
					spBuffer->resize(head.nMessageLength + 1);

					spSocket->Read(&(*spBuffer)[0], head.nMessageLength);
					(*spBuffer)[head.nMessageLength] = 0;
					string sMessage = &(*spBuffer)[sizeof(head)];
					RobotService::instance().m_tpThreadPool.addTask(boost::bind(&JoyStickClient::ProcessMessage, this, &head, sMessage));
				}
				else break;
			}
			else
			{
				log.error("Msg header is error,ingore it!");
				spSocket->Close();
				break;
			}
		}
	}
	else
	{
		SALOG.debug("what?? no readable data??");
	}
}
void JoyStickClient::SetValid(bool isVaild)
{
	m_bIsRegistered = isVaild;
}

void JoyStickClient::Process_Heartbeat(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse)
{
	rapidjson::Document::AllocatorType& allocator = jsResponse.GetAllocator();
	jsResponse.AddMember("errcode", 0, allocator);
	jsResponse.AddMember("errmsg", "ok", allocator);
	sResponse = JsonDocToString(jsResponse);
}

void JoyStickClient::ProcessMessage(const SMsgHeader* header, const string& body)
{
	SATCPConnectionPtr spSocket = m_spSocket;
	log.debug(format("Process JoyStickClient msg='%s'", body));
	rapidjson::Document oDocument;
	string sRes = "";
	const string sFailedResTemple = "{\"sender\": \"server1\",\"sendertype\": \"server\",\"msgid\": \"%s\",\"errcode\": %d,\"errmsg\": \"%s\"}";
	string sErrorMsg = "";
	int nErrcode = 0;
	string sMsgID = "";

	if (!oDocument.Parse<0>(body.c_str()).HasParseError()
		&& oDocument.IsObject())
	{
		string sSender = "";
		string sSendertype = "";
		if (oDocument.HasMember("sender") && oDocument["sender"].IsString())
		{
			sSender = oDocument["sender"].GetString();
		}
		if (oDocument.HasMember("sendertype") && oDocument["sendertype"].IsString())
		{
			sSendertype = oDocument["sendertype"].GetString();
		}
		if (oDocument.HasMember("msgid") && oDocument["msgid"].IsString())
		{
			sMsgID = oDocument["msgid"].GetString();
		}
		if (sMsgID.empty() || sSender.empty() || sSendertype.empty())
		{
			nErrcode = 2;
			sErrorMsg = "request msg format error!";
			log.warning("request msg format error!");
		}
		else
		{
			rapidjson::Document response;
			response.SetObject();
			rapidjson::Document::AllocatorType& allocator = response.GetAllocator();
			response.AddMember("sender", "server1", allocator);
			response.AddMember("sendertype", "server", allocator);
			response.AddMember("msgid", sMsgID.c_str(), allocator);

			m_oLastRecvMsg.update();
			if (sMsgID == "joystick.heartbeat")
			{
				Process_Heartbeat(oDocument, response, sRes);
			}
			else if (sMsgID == "joystick.ctl.active")
			{
				// 控制机器人运动
				Process_Active(oDocument, response, sRes);
			}
			else if (sMsgID == "joystick.ctl.voice")
			{
				Process_Voice(oDocument, response, sRes);
			}
			else if (sMsgID == "joystick.stop.active")
			{
				Process_Stop(oDocument, response, sRes);
			}
			else
			{
				log.error(format("Can't process msg,unkown msgid='%s'", sMsgID));
				nErrcode = 1;
				sErrorMsg = "unkown msgid";
			}
		}
	}
	else
	{
		nErrcode = 3;
		sErrorMsg = "invalid json format!";
		log.warning("JoyStickClient request format error,invalid json format");
	}
	if (sRes.empty())
	{
		sRes = Poco::format(sFailedResTemple, sMsgID, nErrcode, sErrorMsg);
	}
	if (sRes.length() > 0)
	{
		SendMessage(sRes, header->nInvokeID);
	}
}

bool JoyStickClient::SendMessage(const string& sMessage, UInt32 nInvokeID)
{
	SATCPConnectionPtr spSocket = m_spSocket;
	if (spSocket)
	{
		int nMessageLength = sizeof(SMsgHeader) + sMessage.length();
		TSTLBuffer vBuff;
		vBuff.resize(nMessageLength, 0);
		SMsgHeader* pHeader = (SMsgHeader*)&vBuff[0];
		pHeader->nInvokeID = htonl(nInvokeID);
		pHeader->nMessageLength = htonl(nMessageLength);
		pHeader->nProtocolFlag = htonl(PROTOCOLFLAG_ROBOTAGENTCONNECTION);
		memcpy(pHeader + 1, sMessage.c_str(), sMessage.length());
		spSocket->Write(&vBuff[0], vBuff.size());

		log.debug(format("Send to Agent:%s,length=%d", sMessage, nMessageLength));
		return true;
	}
	else
	{
		log.error("Send msg failed,connection may has broken!");
	}
	return false;
}

bool JoyStickClient::SendRequest2JoyStickClient(const string& sMessage, const string& sMsgid)
{
	++m_oAtomicCounter;
	SendMessage(sMessage, m_oAtomicCounter.value());
	return true;
}


bool JoyStickClient::isCreateisElapsed(Poco::Int64 timeout)
{
	return m_oCreate.elapsed() / 1000 > timeout;
}

void JoyStickClient::Process_Active(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse)
{
	log.trace("Process_Active start...");
	string sRobotID = "";
	int nErrcode = 0;
	string sErrmsg = "ok";
	if (jsRequest.HasMember("body") && jsRequest["body"].IsObject())
	{
		rapidjson::Value& jsBody = jsRequest["body"];
		rapidjson::Document::AllocatorType& allocator = jsResponse.GetAllocator();
		if (jsBody.HasMember("robot_id") && jsBody["robot_id"].IsString())
		{
			sRobotID = jsBody["robot_id"].GetString();
		}
		if (jsBody.HasMember("active_mode") && jsBody["active_mode"].IsObject())
		{
			rapidjson::Value &active_mode = jsBody["active_mode"];
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
	}
	log.trace("Process_Active end...");
}

void JoyStickClient::Process_Voice(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse)
{
	log.trace("Process_Voice start...");
	string sRobotID = "";
	int nErrcode = 0;
	string sErrmsg = "ok";
	if (jsRequest.HasMember("body") && jsRequest["body"].IsObject())
	{
		rapidjson::Value& jsBody = jsRequest["body"];
		rapidjson::Document::AllocatorType& allocator = jsResponse.GetAllocator();
		if (jsBody.HasMember("robot_id") && jsBody["robot_id"].IsString())
		{
			sRobotID = jsBody["robot_id"].GetString();
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
	}
	log.trace("Process_Voice end...");
}

void JoyStickClient::Process_Stop(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse)
{
	log.trace("Process_Stop start...");
	string sRobotID = "";
	int nErrcode = 0;
	string sErrmsg = "ok";
	if (jsRequest.HasMember("body") && jsRequest["body"].IsObject())
	{
		rapidjson::Value& jsBody = jsRequest["body"];
		rapidjson::Document::AllocatorType& allocator = jsResponse.GetAllocator();
		if (jsBody.HasMember("robot_id") && jsBody["robot_id"].IsString())
		{
			sRobotID = jsBody["robot_id"].GetString();
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
	}
	log.trace("Process_Stop end...");
}

