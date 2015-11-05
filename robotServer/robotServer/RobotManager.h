#ifdef _MSC_VER
#pragma once
#endif
#ifndef _INC_ROBOTMANAGER_H
#define _INC_ROBOTMANAGER_H

#include "Robot.h"
#include "SAContainer.h"
#include "SATimer.h"

class RobotManager;
typedef boost::shared_ptr<RobotManager> SPRobotManagerPtr;
typedef boost::weak_ptr<RobotManager> SPRobotManagerWeakPtr;


class RobotManager
{
public:
	RobotManager(void);
	~RobotManager(void);
public:
	// 初始化
	bool Init();
	// 释放
	void Free();
	// 创建Robot
	static SPRobotPtr CreateRobot(const string& id);
	// 添加一个Robot
	void AddRobot(SPRobotPtr spRobot);
	// 移除一个Robot
	void RemoveRobot(SPRobotPtr spRobot);
	// 检查
	void CheckRobot();
	// 查找Robot
	SPRobotPtr FindRobot(const string& sRobotid);
	void GetAllRobot(SAArray<SPRobotPtr>& arrRobot);
	void AddRobotByClient(string sClient, SPRobotPtr spRobot);
	void RequestRobotStatus();
public:
	SAMap<string, SPRobotPtr> m_mapRobot;
private:
	//SAMap<string, SPRobotPtr> m_mapRobot;
	// 获取机器人状态定时器
	SATimer m_tmTimer;
};
#endif

