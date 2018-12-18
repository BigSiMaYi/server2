#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct GoldFlower_RobotGameCFGData
{
	//房间id
	int mRoomID;
	//准备时间
	float mReadyTimeMin;
	//准备时间
	float mReadyTimeMax;
	//操作时间
	float mOperaTimeMin;
	//操作时间
	float mOperaTimeMax;
	//结算时间
	float mRobotResultTime;
	//看牌参数
	float mParam1;
	//看牌参数
	float mParam2;
	//看牌参数
	float mParam3;
	//看牌参数
	float mParam4;
};

class GoldFlower_RobotGameCFG
{
public:
private:
	static std::auto_ptr<GoldFlower_RobotGameCFG> msSingleton;
public:
	int GetCount();
	const GoldFlower_RobotGameCFGData* GetData(int RoomID);
	boost::unordered_map<int, GoldFlower_RobotGameCFGData>& GetMapData();
	void Reload();
	void Load();
	static GoldFlower_RobotGameCFG* GetSingleton();
private:
	boost::unordered_map<int, GoldFlower_RobotGameCFGData> mMapData;
};
