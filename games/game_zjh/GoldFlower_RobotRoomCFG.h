#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct GoldFlower_RobotRoomCFGData
{
	//房间id
	int mRoomID;
	//最小开桌数
	int mOpenTableMin;
	//最大开桌数
	int mOpenTableMax;
	//最小桌子数
	int mRobotTableMin;
	//最大桌子数
	int mRobotTableMax;
	//机器人数量
	int mRobotCountMin;
	//机器人数量
	int mRobotCountMax;
	//间隔
	int mElapseTime;
	//启动
	bool mIsOpen;
};

class GoldFlower_RobotRoomCFG
{
public:
private:
	static std::auto_ptr<GoldFlower_RobotRoomCFG> msSingleton;
public:
	int GetCount();
	const GoldFlower_RobotRoomCFGData* GetData(int RoomID);
	boost::unordered_map<int, GoldFlower_RobotRoomCFGData>& GetMapData();
	void Reload();
	void Load();
	static GoldFlower_RobotRoomCFG* GetSingleton();
private:
	boost::unordered_map<int, GoldFlower_RobotRoomCFGData> mMapData;
};
