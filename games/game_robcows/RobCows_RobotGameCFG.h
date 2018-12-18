#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct RobCows_RobotGameCFGData
{
	//房间id
	int mRoomID;
	//准备时间
	int mReadyTime;
	//抢庄时间
	int mBankerTime;
	//抢庄列表
	std::vector<int> mRobList;
	//下注时间
	int mBetTime;
	//抢庄列表
	std::vector<int> mBetList;
	//拼牛时间
	int mOperaTime;
	//结算时间
	int mResultTime;
};

class RobCows_RobotGameCFG
{
public:
private:
	static std::auto_ptr<RobCows_RobotGameCFG> msSingleton;
public:
	int GetCount();
	const RobCows_RobotGameCFGData* GetData(int RoomID);
	boost::unordered_map<int, RobCows_RobotGameCFGData>& GetMapData();
	void Reload();
	void Load();
	static RobCows_RobotGameCFG* GetSingleton();
private:
	boost::unordered_map<int, RobCows_RobotGameCFGData> mMapData;
};
