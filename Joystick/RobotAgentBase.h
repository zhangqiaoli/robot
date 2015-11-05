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
	// ��ʼ��
	virtual void Init();
	// �ͷ�
	virtual void Free();
	// ����
	BOOL Start();
	// ֹͣ
	BOOL Stop(DWORD dwTimeout = 30000);

	BOOL Disconnect();

	// �Ƿ��Ѿ�����
	BOOL IsRuning();
	// �Ƿ�����
	BOOL IsConnected();
public:
	// ������Ϣ
	//BOOL SendMessage(SMsgHeader* pMessage);
	// ��������
	//BOOL SendMessage_Heartbeat();

public:
	// RobotServer��ַ
	CString m_sMonitorAddress;
	// RobotServer�˿�
	USHORT m_nMonitorPort;
	CString m_sObjectName;
public:
	// ��������
	void ProcessHeartbeat();
	// ��ȡ��Ϣ
	UINT ReadMessage();
	// �����߳�
	UINT WorkProc(LPVOID);
	BOOL ProcessMessage(SMsgHeader* pHeader,LPCSTR sMessage);
	void SendMessage(LPCTSTR sMessage, DWORD nInvokeID);
	bool SendRequest2RobotServer(LPCTSTR lpszMessage, LPCTSTR lpszMsgid);
	void SendMessage_Active(LPCTSTR lpszOpera, int speed);
	BOOL SendMessage_Heartbeat();
protected:
	// TCP����
	CPicaSoftTCPSocket m_tcpSocket;
	// �����߳�
	CPicaSoftMemfunWorkThread<CRobotAgentBase> m_thdWorker;
	// ֹͣ�ź�
	CPicaSoftEvent m_evtStopWorker;
	CPicaSoftCriticalSection m_csRequest;
	// ������ʱ
	ULONGLONG m_ullLastRecvMessage;
	ULONGLONG m_ullLastSendMessage;
	CPicaSoftSequenceNumber m_SN;
	int m_nHeartbeatInterval;
};