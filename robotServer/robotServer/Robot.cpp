#include "stdafx.h"
#include "Robot.h"
#include "RobotServer.h"
#include <Poco/Net/SocketAddress.h>
//class MessageTask
//{
//public:
//	MessageTask(Robot& robot, const SRobotHeader* header,const string& sBody)
//		:m_robot(robot), m_header(header), m_sBody(sBody){}
//	void operator()()
//	{
//		SA_PROTECTEDCODE_BEGIN;
//		m_robot.ProcessMessage(m_header, m_sBody);
//		SA_PROTECTEDCODE_ENDMSG("MessageTask处理消息过程中产生异常！")
//	}
//private:
//	Robot& m_robot;
//	const SRobotHeader* m_header;
//	const string m_sBody;
//};

Robot::Robot(const string& id)
{
	m_sID = id;
}

Robot::Robot(SATCPConnectionPtr spConn) :m_spSocket(spConn), log(format("Robot[%s]", spConn->PeerAddress().toString())), m_bIsRegistered(false)
{
	m_sClient = spConn->PeerAddress().toString();
	m_sID = "";
	//spConn->SetOnData(boost::bind(&Robot::OnReceiveData, this));
}

Robot::~Robot(void)
{

}

bool Robot::IsValid()
{
	Clock o;
	if (!m_bIsRegistered && m_oCreate.isElapsed(180*1000*1000))
	{ // 没注册,并且距离创建超过3分钟,无效连接,需断开
		return false;
	}
	else if (m_oLastRecvMsg.isElapsed(60 * 1000*1000))
	{ // 超过1分钟没收到消息
		return false;
	}
	return m_spSocket->IsValid();
}

bool Robot::IsActive()
{
	return m_spSocket->IsValid();
}

bool Robot::Close()
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

void Robot::RecvData()
{
	log.debug("OnReceiveData callback process start...");
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
			if (head.nProtocolFlag == PROTOCOLFLAG_ROBOTCONNECTION)
			{
				if (head.nMessageLength <= nReadableCount)
				{
					TSTLBufferPtr spBuffer = TSTLBufferPtr(new TSTLBuffer);
					spBuffer->resize(head.nMessageLength + 1);

					spSocket->Read(&(*spBuffer)[0], head.nMessageLength);
					(*spBuffer)[head.nMessageLength] = 0;
					string sMessage = &(*spBuffer)[sizeof(head)];
					RobotService::instance().m_tpThreadPool.addTask(boost::bind(&Robot::ProcessMessage, this, &head, sMessage));
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
void Robot::SetValid(bool isVaild)
{
	m_bIsRegistered = isVaild;
}

void Robot::Process_Register(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse)
{
	rapidjson::Document::AllocatorType& allocator = jsResponse.GetAllocator();

	int nErrcode = 0;
	string sErrmsg = "ok";
	if (!IsRegistered())
	{
		jsResponse.AddMember("question_bank_version", "1", allocator);
		string sRobotid = "";
		if (jsRequest.HasMember("body") && jsRequest["body"].IsObject())
		{
			rapidjson::Value& body = jsRequest["body"];
			if (body.HasMember("robot_id") && body["robot_id"].IsString())
			{
				sRobotid = body["robot_id"].GetString();
			}
			else if (jsRequest.HasMember("mac") && jsRequest["mac"].IsObject())
			{
				sRobotid = body["mac"].GetString();
			}
			if (!sRobotid.empty())
			{
				// 清除robot_id对应的旧链接
				SAContainerSingleLock csl(&RobotService::instance().m_robotManager.m_mapRobot, true);
				SPRobotPtr spRobot = RobotService::instance().m_robotManager.FindRobot(sRobotid);
				bool bSuccess = true;
				if (spRobot)
				{
					if (spRobot->isCreateisElapsed(10000))
					{  // 旧链接创建超过10s,可踢掉
						log.error( format("break session '%s'",spRobot->GetRobotID()));
						spRobot->Close();
						spRobot->SetValid(false);
					}
					else
					{
						log.warning(format("connect frequency too high,reserve old connection'%s'!", spRobot->GetRobotID()));
						SATCPConnectionPtr spSocket = m_spSocket;
						spSocket->Close();
						bSuccess = false;
						nErrcode = 6;
						sErrmsg = "connect frequency too high,reserve old connection!";
					}
				}
				if (bSuccess)
				{
					m_bIsRegistered = true;
					m_sID = sRobotid;
					RobotService::instance().m_robotManager.AddRobot(GetRobotPtr());
					//RobotService::instance().m_mapUnkownList.RemoveKey(m_sClient);
				}
				csl.Unlock();
			}
			else
			{
				nErrcode = 8;
				sErrmsg = "invalid msg data,miss robot_id param!";
				log.warning("invalid msg data,miss robot_id param!");
			}
		}
		else
		{
			nErrcode = 7;
			sErrmsg = "invalid msg data,miss necessary param!";
			log.warning("invalid msg data,miss necessary param!");
		}
	}
	else
	{
		nErrcode = 5;
		sErrmsg = "Process register failed,already registered!";
		log.error("Process register failed,already registered!");
	}
	jsResponse.AddMember("errcode", 0, allocator);
	jsResponse.AddMember("errmsg", "ok", allocator);
	sResponse = JsonDocToString(jsResponse);
}

void Robot::Process_Heartbeat(rapidjson::Document& jsRequest, rapidjson::Document& jsResponse, string& sResponse)
{
	rapidjson::Document::AllocatorType& allocator = jsResponse.GetAllocator();
	jsResponse.AddMember("errcode", 0, allocator);
	jsResponse.AddMember("errmsg", "ok", allocator);
	sResponse = JsonDocToString(jsResponse);
}

void Robot::Process_RequestCustomer(rapidjson::Document& jsRequest)
{
	if (jsRequest.HasMember("body") && jsRequest["body"].IsObject())
	{
		rapidjson::Value& body = jsRequest["body"];
		if (body.HasMember("question") && body["question"].IsObject())
		{
			rapidjson::Value& question = body["question"];
			rapidjson::Document request2Session;
			request2Session.SetObject();
			rapidjson::Document::AllocatorType& allocator2A = request2Session.GetAllocator();
			request2Session.AddMember("msgid", "request_customer", allocator2A);
			rapidjson::Value jstmp;
			jstmp.SetString(m_sID.c_str(), allocator2A);
			request2Session.AddMember("robot_id", jstmp, allocator2A);
			request2Session.AddMember("question", question, allocator2A);
			string sNotify = JsonDocToString(request2Session);

			vector<WebSocketSessionPtr> arrSession;
			RobotService::instance().m_SessionManager.GetAllSession(arrSession);
			for (vector<WebSocketSessionPtr>::iterator it = arrSession.begin(); it!=arrSession.end(); it++)
			{
				WebSocketSessionPtr spSession = *it;
				spSession->SendNotify(sNotify);
			}
		}
		else
		{
			log.debug("Process_RequestCustomer failed,no question node!");
		}
	}

}

void Robot::ProcessMessage(const SMsgHeader* header,const string& body)
{
	log.debug("ProcessMessage start...");
	SATCPConnectionPtr spSocket = m_spSocket;
	log.debug(format("Process robot msg='%s'", body));
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
			if (IsRegistered())
			{
				m_oLastRecvMsg.update();
				if (sMsgID == "robot.heartbeat")
				{
					Process_Heartbeat(oDocument, response,sRes);
				}
				else if (sMsgID == "server.get.robot_status")
				{
					// 更新状态信息
					Process_Status(oDocument);
					return;
				}
				else if (sMsgID == "robot.request.customer")
				{
					 // 请求人工服务
					Process_RequestCustomer(oDocument);
				}
			}
			else if (sMsgID == "robot.register")
			{
				Process_Register(oDocument, response, sRes);
			}
			else
			{
				log.error(format("Can't process msg,has't registered!msgid='%s'", sMsgID));
				nErrcode = 1;
				sErrorMsg = "Has't registered!";
			}
		}
	}
	else
	{
		nErrcode = 3;
		sErrorMsg = "invalid json format!";
		log.warning("robot request format error,invalid json format");
	}
	if (sRes.empty())
	{
		sRes = Poco::format(sFailedResTemple,sMsgID, nErrcode, sErrorMsg);
	}
	if (sRes.length() > 0)
	{
		SendMessage(sRes, header->nInvokeID);
	}
	if (sMsgID == "robot.register")
	{
		if (m_bIsRegistered)
		{
			SendGetStatusRequest();
		}
	}
}

bool Robot::SendMessage(const string& sMessage, UInt32 nInvokeID)
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
		pHeader->nProtocolFlag = htonl(PROTOCOLFLAG_ROBOTCONNECTION);
		memcpy(pHeader + 1, sMessage.c_str(), sMessage.length());
		spSocket->Write(&vBuff[0], vBuff.size());

		log.debug(format("Send to robot:%s,length=%d", sMessage, nMessageLength));
		return true;
	}
	else
	{
		log.error("Send msg failed,connection may has broken!");
	}
	return false;
}

bool Robot::SendRequest2Robot(const string& sMessage,const string& sMsgid)
{
	++m_oAtomicCounter;
	SendMessage(sMessage, m_oAtomicCounter.value());
	return true;
}

bool Robot::SendGetStatusRequest()
{
	rapidjson::Document request2Robot;
	request2Robot.SetObject();
	rapidjson::Value body(rapidjson::kObjectType);
	rapidjson::Document::AllocatorType& allocator2R = request2Robot.GetAllocator();
	request2Robot.AddMember("sender", "server1", allocator2R);
	request2Robot.AddMember("sendertype", "server", allocator2R);
	request2Robot.AddMember("msgid", "server.get.robot_status", allocator2R);
	request2Robot.AddMember("body", body, allocator2R);
	string sGetStatusMsg = JsonDocToString(request2Robot);
	SendRequest2Robot(sGetStatusMsg, "server.get.robot_status");
	return true;
}

bool Robot::isCreateisElapsed(Poco::Int64 timeout)
{
	return m_oCreate.elapsed() / 1000 > timeout;
}

void Robot::Process_Status(rapidjson::Document& jsRequest)
{
	if (jsRequest.HasMember("robot_info") && jsRequest["robot_info"].IsObject())
	{
		m_sRobotStatus = JsonValueToString(jsRequest["robot_info"]);
	}
}

