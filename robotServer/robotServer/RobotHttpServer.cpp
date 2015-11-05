#include "stdafx.h"
#include "RobotHttpServer.h"
#include <SABoost.h>
#include <SAWebSocket.h>
#include "RobotServer.h"
#include "SessionManager.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

RobotHttpServer::RobotHttpServer(void)
{

}


RobotHttpServer::~RobotHttpServer(void)
{
	Free();
}

bool RobotHttpServer::Init()
{
	m_HTTPServer.SetListenPort(cfg.HttpListenPort);
	m_HTTPServer.SetDefaultHandler(boost::bind(&RobotHttpServer::HandleRequest, this, _1, _2));
	m_HTTPServer.AddHandler("/ws", boost::bind(&RobotHttpServer::HandleWSConnect, this, _1, _2));
	return true;
}

void RobotHttpServer::Free()
{
	Stop(3000);
}

bool RobotHttpServer::Start()
{
	return m_HTTPServer.Start();
}

void RobotHttpServer::Stop(UInt32 nTimeout)
{
	m_HTTPServer.Stop(nTimeout);
}

bool RobotHttpServer::HandleWSConnect(HTTPServerRequest& request, HTTPServerResponse& response)
{
	SAWebSocketPtr ws = SAWebSocket::CreateWebSocket(request, response);
	if (ws)
	{
		WebSocketSessionPtr spSession = SessionManager::CreateSession(ws);
		if (spSession)
		{
			RobotService::instance().m_SessionManager.AddSession(spSession);
			return true;
		}
	}

	return false;
}

bool RobotHttpServer::HTTP_SendResponse(HTTPServerResponse& resp, int nStatusCode, const string& sStatusText, const string& sBody)
{
	//resp.setStatus(nStatusCode); 
	string sRes = format(
		"HTTP/1.1 %d %s\r\n"
		"Content-Type: text/json; charset=utf-8\r\n"
		"Content-Length: %u\r\n"
		"Connection: close\r\n"
		"\r\n"
		"%s",
		nStatusCode, sStatusText, sBody.size(), sBody);
	resp.sendBuffer(sRes.c_str(), sRes.size());
	return true;
}

bool RobotHttpServer::HandleRequest(HTTPServerRequest& req, HTTPServerResponse& resp)
{
	string sURI = req.getURI();
	string sCommandPath = "";
	size_t nIndex = sURI.find('/');
	if (nIndex != string::npos)
	{
		if (nIndex == 0)
		{
			sURI = sURI.substr(1);
		}
		string sClient = sURI.substr(0, sURI.find('/'));
		bool bParamAsBody = false;
		string sRes = "";
		sURI = sURI.substr(sURI.find('/') + 1);
		sCommandPath = sURI;
		size_t nParamstart = sURI.find("?");
		string sParamstr;
		string sCallback;
		if (nParamstart != string::npos)
		{
			sCommandPath = sCommandPath.substr(0, nParamstart);
			sParamstr = SAAssistant::DecodeUrl(sURI.substr(nParamstart + 1));
		}
		if (sURI.find("callback=jQuery") != string::npos)
		{
			// GetJson支持
			bParamAsBody = true;
			vector<string> arrParam;
			string sBody;
			SAAssistant::SplitString(sParamstr, "&", arrParam);
			for (size_t i = 0; i < arrParam.size(); i++)
			{
				string sParam = arrParam[i];
				if (sParam.substr(0, 9) == "callback=")sCallback = sParam.substr(9);
				else if (sParam[0] == '{' || sParam[0] == '}') sParamstr = sParam;
				else if (sParam[0] == '[')
				{
					sParamstr = sParam;
				}
			}
		}
		if (toLower(sClient) == "")
		{
			HTTP_SendResponse(resp, 200, "", "empty uri");
		}
		else
		{
			HTTP_SendResponse(resp, 400, "Bad Request", "Not support uri");
			return false;
		}
	}
	else
	{
		HTTP_SendResponse(resp, 400, "Bad Request", "Uri error");
	}
	return true;
}

