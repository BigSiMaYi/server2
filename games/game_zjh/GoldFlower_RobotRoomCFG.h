#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct GoldFlower_RobotRoomCFGData
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
