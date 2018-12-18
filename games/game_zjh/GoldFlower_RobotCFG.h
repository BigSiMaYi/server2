#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct GoldFlower_RobotCFGData
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
	//机器人系数
	float mRobotCoeffi;
	//开放机器人
	int mIsOpen;
};

class GoldFlower_RobotCFG
{
public:
private:
	static std::auto_ptr<GoldFlower_RobotCFG> msSingleton;
public:
	int GetCount();
	const GoldFlower_RobotCFGData* GetData(int RoomID);
	boost::unordered_map<int, GoldFlower_RobotCFGData>& GetMapData();
	void Reload();
	void Load();
	static GoldFlower_RobotCFG* GetSingleton();
private:
	boost::unordered_map<int, GoldFlower_RobotCFGData> mMapData;
};
