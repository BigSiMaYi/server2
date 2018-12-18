#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct RobCows_RobotCFGData
{
	//����id
	int mRoomID;
	//�����
	int mRobotMaxGold;
	//�볡��С���
	int mRobotMinTake;
	//�볡�����
	int mRobotMaxTake;
	//����������
	int mRobotMinCount;
	//����������
	int mRobotMaxCount;
	//������VIP
	int mRobotMinVip;
	//������VIP
	int mRobotMaxVip;
	//���پ���
	int mRobotMinRound;
	//������
	int mRobotMaxRound;
	//�������
	int mRobotMinEntry;
	//�������
	int mRobotMaxEntry;
	//���Ż�����
	int mIsOpen;
};

class RobCows_RobotCFG
{
public:
private:
	static std::auto_ptr<RobCows_RobotCFG> msSingleton;
public:
	int GetCount();
	const RobCows_RobotCFGData* GetData(int RoomID);
	boost::unordered_map<int, RobCows_RobotCFGData>& GetMapData();
	void Reload();
	void Load();
	static RobCows_RobotCFG* GetSingleton();
private:
	boost::unordered_map<int, RobCows_RobotCFGData> mMapData;
};
