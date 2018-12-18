#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct GoldFlower_RobotCFGData
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
	//������ϵ��
	float mRobotCoeffi;
	//���Ż�����
	int mIsOpen;
};

class GoldFlower_RobotCFG
{
public:
private:
	static std::auto_ptr<GoldFlower_RobotCFG> msSingleton;
public:
	int GetCount();
	const GoldFlower_RobotCFGData* GetData(int RoomID);
	boost::unordered_map<int, GoldFlower_RobotCFGData>& GetMapData();
	void Reload();
	void Load();
	static GoldFlower_RobotCFG* GetSingleton();
private:
	boost::unordered_map<int, GoldFlower_RobotCFGData> mMapData;
};
