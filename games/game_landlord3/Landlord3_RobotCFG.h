#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct Landlord3_RobotCFGData
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

class Landlord3_RobotCFG
{
public:
private:
	static std::auto_ptr<Landlord3_RobotCFG> msSingleton;
public:
	int GetCount();
	const Landlord3_RobotCFGData* GetData(int RoomID);
	boost::unordered_map<int, Landlord3_RobotCFGData>& GetMapData();
	void Reload();
	void Load();
	static Landlord3_RobotCFG* GetSingleton();
private:
	boost::unordered_map<int, Landlord3_RobotCFGData> mMapData;
};
