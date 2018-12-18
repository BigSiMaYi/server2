#pragma once
#include <map>
struct DragonTiger_RoomCFGData
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
};

class DragonTiger_RoomCFG
{
public:
private:
	static std::auto_ptr<DragonTiger_RoomCFG> msSingleton;
public:
	int GetCount();
	const DragonTiger_RoomCFGData* GetData(int RoomID);
	const std::map<int, DragonTiger_RoomCFGData>& GetMapData();
	void Load();
	void LoadLua();
	void Reload();
	static DragonTiger_RoomCFG* GetSingleton();
private:
	std::map<int, DragonTiger_RoomCFGData> mMapData;
};
