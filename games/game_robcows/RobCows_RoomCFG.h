#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct RobCows_RoomCFGData
{
	//房间id
	int mRoomID;
	//房间名
	std::string mRoomName;
	//房间图片
	std::string mRoomImage;
	//金币条件
	int mGoldMinCondition;
	//金币条件
	int mGoldMaxCondition;
	//vip条件
	int mVipCondition;
	//礼券条件
	int mTicketCondition;
	//底分
	int mBaseCondition;
	//抢庄列表
	std::vector<int> mRobBankerList;
	//下注列表
	std::vector<int> mBetList;
	//真实玩家数
	int mRealCount;
	//等待玩家数
	int mWaitCount;
	//桌子数量
	int mTableCount;
	//是否开放
	bool mIsOpen;
};

class RobCows_RoomCFG
{
public:
private:
	static std::auto_ptr<RobCows_RoomCFG> msSingleton;
public:
	int GetCount();
	const RobCows_RoomCFGData* GetData(int RoomID);
	boost::unordered_map<int, RobCows_RoomCFGData>& GetMapData();
	void Reload();
	void Load();
	static RobCows_RoomCFG* GetSingleton();
private:
	boost::unordered_map<int, RobCows_RoomCFGData> mMapData;
};
