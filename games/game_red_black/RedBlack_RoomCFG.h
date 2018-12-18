#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct RedBlack_RoomCFGData
{
	//房间id
	int mRoomID;
	//房间名
	std::string mRoomName;
	//房间图片
	std::string mRoomImage;
	//金币条件
	int mGoldCondition;
	//vip条件
	int mVipCondition;
	//礼券条件
	int mTicketCondition;
	//最大倍率
	int mMaxRate;
	//筹码列表
	std::vector<int> mChipList;
	//解锁筹码列表
	std::vector<int> mUnlockChipList;
	//筹码图片
	std::vector<std::string> mChipImages;
	//是否开放
	bool mIsOpen;
	//上庄金额
	int mBankerGold;
	//抢庄消耗金额
	int mSnatchGold;
	//强制下庄金额
	int mForceLeaveGold;
	//红，幸运，黑区(下注区域金币限制)
	std::vector<int> mBetAreaGoldCond;
	//房间下注金币限制
	int mBetGoldCondition;
	//机器人幸运区下注
	int mRobotLuckAreaBetCond;
};

class RedBlack_RoomCFG
{
public:
private:
	static std::auto_ptr<RedBlack_RoomCFG> msSingleton;
public:
	int GetCount();
	const RedBlack_RoomCFGData* GetData(int RoomID);
	boost::unordered_map<int, RedBlack_RoomCFGData>& GetMapData();
	void Reload();
	void Load();
	static RedBlack_RoomCFG* GetSingleton();
private:
	boost::unordered_map<int, RedBlack_RoomCFGData> mMapData;
};
