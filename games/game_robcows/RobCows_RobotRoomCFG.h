#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct RobCows_RobotRoomCFGData
{
	//����id
	int mRoomID;
	//��С������
	int mOpenTableMin;
	//�������
	int mOpenTableMax;
	//��С������
	int mRobotTableMin;
	//���������
	int mRobotTableMax;
	//����������
	int mRobotCountMin;
	//����������
	int mRobotCountMax;
	//���
	int mElapseTime;
	//����
	bool mIsOpen;
};

class RobCows_RobotRoomCFG
{
public:
private:
	static std::auto_ptr<RobCows_RobotRoomCFG> msSingleton;
public:
	int GetCount();
	const RobCows_RobotRoomCFGData* GetData(int RoomID);
	boost::unordered_map<int, RobCows_RobotRoomCFGData>& GetMapData();
	void Reload();
	void Load();
	static RobCows_RobotRoomCFG* GetSingleton();
private:
	boost::unordered_map<int, RobCows_RobotRoomCFGData> mMapData;
};
