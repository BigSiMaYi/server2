#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct BMW_LotteryCFGData
{
	//房间id
	int mRoomID;
	//概率事件
	int mRandType;
	//大奖概率
	int mBigRate;
	//浮动时间间隔
	int mRefreshMinute;
};

class BMW_LotteryCFG
{
public:
private:
	static std::auto_ptr<BMW_LotteryCFG> msSingleton;
public:
	int GetCount();
	const BMW_LotteryCFGData* GetData(int RoomID);
	boost::unordered_map<int, BMW_LotteryCFGData>& GetMapData();
	void Reload();
	void Load();
	static BMW_LotteryCFG* GetSingleton();
private:
	boost::unordered_map<int, BMW_LotteryCFGData> mMapData;
};
