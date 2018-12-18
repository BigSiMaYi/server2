#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct Landlord3_RobotGameCFGData
{
	//����id
	int mRoomID;
	//����ʱ��
	int mRobotOperaTime;
	//׼��ʱ��
	int mRobotReadyTime;
	//����ʱ��
	int mRobotResultTime;
};

class Landlord3_RobotGameCFG
{
public:
private:
	static std::auto_ptr<Landlord3_RobotGameCFG> msSingleton;
public:
	int GetCount();
	const Landlord3_RobotGameCFGData* GetData(int RoomID);
	boost::unordered_map<int, Landlord3_RobotGameCFGData>& GetMapData();
	void Reload();
	void Load();
	static Landlord3_RobotGameCFG* GetSingleton();
private:
	boost::unordered_map<int, Landlord3_RobotGameCFGData> mMapData;
};
