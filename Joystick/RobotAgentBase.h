#pragma once
#include <PicaSoftTCPSocket.h>
#include "PicaSoftMultiThread.h"
class CRobotAgentBase;
typedef boost::shared_ptr<CRobotAgentBase> CRobotAgentBasePtr;
typedef boost::weak_ptr<CRobotAgentBase> CRobotAgentBaseWeakPtr;

class CRobotAgentRequest;
typedef boost::shared_ptr<CRobotAgentRequest> CRobotAgentRequestPtr;
typedef boost::weak_ptr<CRobotAgentRequest> CRobotAgentRequestWeakPtr;

typedef struct _tag_Robot_MsgHeader
{
	DWORD nProtocolFlag;
	DWORD nMessageLength;
	DWORD nInvokeID;
}SMsgHeader, *LPSMsgHeader;
#define PROTOCOLFLAG_ROBOTAGENTCONNECTION 0x01332AD4
class CRobotAgentRequest
{
public:
	CRobotAgentRequest(CRobotAgentBasePtr spClient, SMsgHeader* pRequest);
	~CRobotAgentRequest();
public:
	BOOL WaitResponse(DWORD dwTimeout);
	SMsgHeader* m_pRequest;
	SMsgHeader* m_pResponse;
	void Done();
private:
	CPicaSoftEvent m_evtResponsed;
	CRobotAgentBaseWeakPtr m_swpClient;
};

class CRobotAgentBase :public boost::enable_shared_from_this < CRobotAgentBase >
{
public:
	CRobotAgentBase(void);
	virtual ~CRobotAgentBase(void);
public:
	// 初始化
	virtual void Init();
	// 释放
	virtual void Free();
	// 启动
	BOOL Start();
	// 停止
	BOOL Stop(DWORD dwTimeout = 30000);

	BOOL Disconnect();

	// 是否已经启动
	BOOL IsRuning();
	// 是否连接
	BOOL IsConnected();
public:
	// 发送消息
	//BOOL SendMessage(SMsgHeader* pMessage);
	// 发送心跳
	//BOOL SendMessage_Heartbeat();

public:
	// RobotServer地址
	CString m_sMonitorAddress;
	// RobotServer端口
	USHORT m_nMonitorPort;
	CString m_sObjectName;
public:
	// 处理心跳
	void ProcessHeartbeat();
	// 读取消息
	UINT ReadMessage();
	// 工作线程
	UINT WorkProc(LPVOID);
	BOOL ProcessMessage(SMsgHeader* pHeader,LPCSTR sMessage);
	void SendMessage(LPCTSTR sMessage, DWORD nInvokeID);
	bool SendRequest2RobotServer(LPCTSTR lpszMessage, LPCTSTR lpszMsgid);
	void SendMessage_Active(LPCTSTR lpszOpera, int speed);
	BOOL SendMessage_Heartbeat();
protected:
	// TCP连接
	CPicaSoftTCPSocket m_tcpSocket;
	// 工作线程
	CPicaSoftMemfunWorkThread<CRobotAgentBase> m_thdWorker;
	// 停止信号
	CPicaSoftEvent m_evtStopWorker;
	CPicaSoftCriticalSection m_csRequest;
	// 心跳计时
	ULONGLONG m_ullLastRecvMessage;
	ULONGLONG m_ullLastSendMessage;
	CPicaSoftSequenceNumber m_SN;
	int m_nHeartbeatInterval;
};