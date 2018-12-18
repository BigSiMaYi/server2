#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct Landlord3_RoomCFGData
{
	//房间id
	int mRoomID;
	//房间名
	std::string mRoomName;
	//房间图片
	std::string mRoomImage;
	//金币条件
	int mKickGoldCondition;
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
	//桌子数量
	int mTableCount;
	//真实玩家最大数量
	int mPlayerMaxCounter;
	//是否开放
	bool mIsOpen;
};

class Landlord3_RoomCFG
{
public:
private:
	static std::auto_ptr<Landlord3_RoomCFG> msSingleton;
public:
	int GetCount();
	const Landlord3_RoomCFGData* GetData(int RoomID);
	boost::unordered_map<int, Landlord3_RoomCFGData>& GetMapData();
	void Reload();
	void Reload(const std::string& path);
	void Load(const std::string& path);
	void Load();
	static Landlord3_RoomCFG* GetSingleton();
private:
	boost::unordered_map<int, Landlord3_RoomCFGData> mMapData;
};
