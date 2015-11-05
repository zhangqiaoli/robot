#include "stdafx.h"
#include "RobotManager.h"


RobotManager::RobotManager(void)
{
	m_tmTimer.AddTimer("RobotStatusTimer", boost::bind(&RobotManager::RequestRobotStatus, this), 5000, 60*1000);
}


RobotManager::~RobotManager(void)
{
	Free();
}

bool RobotManager::Init()
{
	return true;
}
void RobotManager::RequestRobotStatus()
{
	SALOG.trace(format("RequestRobotStatus,current Robot count=%z", m_mapRobot.GetCount()));
	for (SAMap<string, SPRobotPtr>::Iterator it = m_mapRobot.GetIterator(); !it.IsEnd(); it.MoveNext())
	{
		SPRobotPtr spClient = it.GetValue();
		if (spClient->IsValid())
		{
			spClient->SendGetStatusRequest();
		}
	}
}

void RobotManager::Free()
{
	m_mapRobot.RemoveAll();
}

SPRobotPtr RobotManager::CreateRobot(const string& id)
{
	return SPRobotPtr(new Robot(id));
}

void RobotManager::AddRobot(SPRobotPtr spRobot)
{
	if (spRobot)
	{
		m_mapRobot.SetAt(spRobot->GetRobotID(), spRobot);
	}
}

SPRobotPtr RobotManager::FindRobot(const string& sRobotID)
{
	SPRobotPtr spRobot;
	m_mapRobot.Lookup(sRobotID, spRobot);
	return spRobot;
}

void RobotManager::GetAllRobot(SAArray<SPRobotPtr>& arrRobot)
{
	for (SAMap<string, SPRobotPtr>::Iterator it = m_mapRobot.GetIterator(); !it.IsEnd(); it.MoveNext())
	{
		SPRobotPtr spClient = it.GetValue();
		arrRobot.Add(spClient);
	}
}

void RobotManager::RemoveRobot(SPRobotPtr spRobot)
{
	if (spRobot)
	{
		m_mapRobot.RemoveKey(spRobot->GetRobotID());
		SALOG.debug(format("SPRobotManager::RemoveRobot remove Robot %s", spRobot->GetRobotName()));
	}
}

void RobotManager::CheckRobot()
{
	SALOG.trace(format("start check Robot,current Robot count=%z", m_mapRobot.GetCount()));
	SAArray<SPRobotPtr, SAContainerFast> arrRemove;
	for (SAMap<string, SPRobotPtr>::Iterator it = m_mapRobot.GetIterator(); !it.IsEnd(); it.MoveNext())
	{
		SPRobotPtr spClient = it.GetValue();
		if (!spClient->IsValid())
		{			
			arrRemove.Add(spClient);
		}
	}
	for (SAArray<SPRobotPtr, SAContainerFast>::Iterator it = arrRemove.GetIterator(); !it.IsEnd(); it.MoveNext())
	{
		SPRobotPtr spClient = it.Get();
		spClient->Close();
		RemoveRobot(it.Get());
		SALOG.debug(format("remove Robot %s",spClient->GetRobotID()));
	}
}
