#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct BMW_RobotCFGData
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
	//上庄数量
	int mBankerCount;
	//携带金币系数
	float mBankerMinRate;
	//携带金币系数
	float mBankerMaxRate;
	//凑桌间隔
	int mBankerMinEntry;
	//凑桌间隔
	int mBankerMaxEntry;
	//开放机器人
	int mIsOpen;
};

class BMW_RobotCFG
{
public:
private:
	static std::auto_ptr<BMW_RobotCFG> msSingleton;
public:
	int GetCount();
	const BMW_RobotCFGData* GetData(int RoomID);
	boost::unordered_map<int, BMW_RobotCFGData>& GetMapData();
	void Reload();
	void Load();
	static BMW_RobotCFG* GetSingleton();
private:
	boost::unordered_map<int, BMW_RobotCFGData> mMapData;
};
