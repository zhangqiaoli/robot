#pragma once
#include "RobotAgentBase.h"
class CRobotAgent;
typedef boost::shared_ptr<CRobotAgent> CRobotAgentPtr;
typedef boost::weak_ptr<CRobotAgent> CRobotAgentWeakPtr;

class CRobotAgent :
	public CRobotAgentBase
{
public:
	CRobotAgent(void);
	~CRobotAgent(void);
public:
	// ≥ı ºªØ
	virtual void Init();
	//  Õ∑≈
	virtual void Free();
	//void OnChangeStatusAck(S_TCP_MSG2AGT_CHANGE_STATUS_ACK *pAck);

public:

};