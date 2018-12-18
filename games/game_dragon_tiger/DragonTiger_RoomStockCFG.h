#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct DragonTiger_RoomStockCFGData
{
	//房间id
	int mRoomID;
	//赢钱抽水
	float mDeduct_1;
	//输钱抽水
	float mDeduct_2;
	//开放机器人
	int mIsOpen;
	//机器人数量
	int mRobotMinCount;
	//机器人数量
	int mRobotMaxCount;
	//机器人VIP
	int mRobotMinVip;
	//机器人VIP
	int mRobotMaxVip;
	//机器人时间
	int mRobotMinLifeTime;
	//机器人时间
	int mRobotMaxLifeTime;
	//机器人最小下注
	int mRobotSystemMinBet;
	//机器人最大下注
	int mRobotSystemMaxBet;
	//机器人每次下注时间间隔
	int mRobotMinBetTime;
	//机器人每次下注时间间隔
	int mRobotMaxBetTime;
	//机器人不下注的概率（百分比）
	int mRobotCannotBet;
};

class DragonTiger_RoomStockCFG
{
public:
private:
	static std::auto_ptr<DragonTiger_RoomStockCFG> msSingleton;
public:
	int GetCount();
	const DragonTiger_RoomStockCFGData* GetData(int RoomID);
	boost::unordered_map<int, DragonTiger_RoomStockCFGData>& GetMapData();
	void Reload();
	void Load();
	static DragonTiger_RoomStockCFG* GetSingleton();
private:
	boost::unordered_map<int, DragonTiger_RoomStockCFGData> mMapData;
};
