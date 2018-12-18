#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct RobCows_RobotCFGData
{
	//房间id
	int mRoomID;
	//最大金币
	int mRobotMaxGold;
	//入场最小金币
	int mRobotMinTake;
	//入场最大金币
	int mRobotMaxTake;
	//机器人数量
	int mRobotMinCount;
	//机器人数量
	int mRobotMaxCount;
	//机器人VIP
	int mRobotMinVip;
	//机器人VIP
	int mRobotMaxVip;
	//最少局数
	int mRobotMinRound;
	//最多局数
	int mRobotMaxRound;
	//凑桌间隔
	int mRobotMinEntry;
	//凑桌间隔
	int mRobotMaxEntry;
	//开放机器人
	int mIsOpen;
};

class RobCows_RobotCFG
{
public:
private:
	static std::auto_ptr<RobCows_RobotCFG> msSingleton;
public:
	int GetCount();
	const RobCows_RobotCFGData* GetData(int RoomID);
	boost::unordered_map<int, RobCows_RobotCFGData>& GetMapData();
	void Reload();
	void Load();
	static RobCows_RobotCFG* GetSingleton();
private:
	boost::unordered_map<int, RobCows_RobotCFGData> mMapData;
};
