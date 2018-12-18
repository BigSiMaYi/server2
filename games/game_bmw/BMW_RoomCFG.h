#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct BMW_RoomCFGData
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
	//礼券条件
	int mBetCondition;
	//上庄条件
	int mBankerGold;
	//上庄条件
	int mBankerGoldExtra;
	//上庄局数
	int mBankerRound;
	//上庄局数
	int mBankerRoundExtra;
	//下注限制
	int mPlayerLimit;
	//区域限制
	int mAreaLimit;
	//下注基数
	std::vector<int> mPlaceJetton;
	//加注筹码图片
	std::vector<std::string> mChipImages;
	//历史记录
	int mRecordHistory;
	//系统庄
	bool mIsSysBanker;
	//人数
	int mMaxPlayers;
	//桌子数量
	int mTableCount;
	//是否开放
	bool mIsOpen;
};

class BMW_RoomCFG
{
public:
private:
	static std::auto_ptr<BMW_RoomCFG> msSingleton;
public:
	int GetCount();
	const BMW_RoomCFGData* GetData(int RoomID);
	boost::unordered_map<int, BMW_RoomCFGData>& GetMapData();
	void Reload();
	void Load();
	static BMW_RoomCFG* GetSingleton();
private:
	boost::unordered_map<int, BMW_RoomCFGData> mMapData;
};
