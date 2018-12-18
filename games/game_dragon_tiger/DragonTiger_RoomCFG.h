#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
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
	//筹码列表
	std::vector<int> mChipList;
	//解锁筹码列表
	std::vector<int> mUnlockChipList;
	//筹码图片
	std::vector<std::string> mChipImages;
	//龙和和区域最大压注限制
	std::vector<int> mBetAreaGoldCond;
	//下注条件
	int mBetGoldCondition;
	//是否开放
	bool mIsOpen;
};

class DragonTiger_RoomCFG
{
public:
private:
	static std::auto_ptr<DragonTiger_RoomCFG> msSingleton;
public:
	int GetCount();
	const DragonTiger_RoomCFGData* GetData(int RoomID);
	boost::unordered_map<int, DragonTiger_RoomCFGData>& GetMapData();
	void Reload();
	void Load();
	static DragonTiger_RoomCFG* GetSingleton();
private:
	boost::unordered_map<int, DragonTiger_RoomCFGData> mMapData;
};
