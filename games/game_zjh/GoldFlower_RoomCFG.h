#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct GoldFlower_RoomCFGData
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
	//加注筹码列表
	std::vector<int> mAddChipList;
	//加注筹码图片
	std::vector<std::string> mAddChipImages;
	//看牌轮数
	int mLookCondition;
	//比牌条件
	int mPkCondition;
	//最大筹码
	int mMaxChip;
	//最大轮数
	int mMaxRound;
	//最大总筹码
	int mMaxPool;
	//桌子数量
	int mTableCount;
	//真人数量
	int mTableManCount;
	//机器人数量
	int mTableRobotCount;
	//是否开放
	bool mIsOpen;
};

class GoldFlower_RoomCFG
{
public:
private:
	static std::auto_ptr<GoldFlower_RoomCFG> msSingleton;
public:
	int GetCount();
	const GoldFlower_RoomCFGData* GetData(int RoomID);
	boost::unordered_map<int, GoldFlower_RoomCFGData>& GetMapData();
	void Reload();
	void Load();
	static GoldFlower_RoomCFG* GetSingleton();
private:
	boost::unordered_map<int, GoldFlower_RoomCFGData> mMapData;
};
