#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct Cows_RoomCFGData
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
	//最大玩家数
	int mMaxPlayerCnt;
	//期望盈利比例
	int mExpectEarnRate;
	//最大盈利比例
	int mMaxEarnRate;
	//最小盈利比例
	int mMinEarnRate;
	//杀分开关
	bool mKillPointsSwtch;
};

class Cows_RoomCFG
{
public:
private:
	static std::auto_ptr<Cows_RoomCFG> msSingleton;
public:
	int GetCount();
	const Cows_RoomCFGData* GetData(int RoomID);
	boost::unordered_map<int, Cows_RoomCFGData>& GetMapData();
	void Reload();
	void Load();
	static Cows_RoomCFG* GetSingleton();
private:
	boost::unordered_map<int, Cows_RoomCFGData> mMapData;
};
