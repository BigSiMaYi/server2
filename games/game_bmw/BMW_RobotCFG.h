#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct BMW_RobotCFGData
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
	//��ׯ����
	int mBankerCount;
	//Я�����ϵ��
	float mBankerMinRate;
	//Я�����ϵ��
	float mBankerMaxRate;
	//�������
	int mBankerMinEntry;
	//�������
	int mBankerMaxEntry;
	//���Ż�����
	int mIsOpen;
};

class BMW_RobotCFG
{
public:
private:
	static std::auto_ptr<BMW_RobotCFG> msSingleton;
public:
	int GetCount();
	const BMW_RobotCFGData* GetData(int RoomID);
	boost::unordered_map<int, BMW_RobotCFGData>& GetMapData();
	void Reload();
	void Load();
	static BMW_RobotCFG* GetSingleton();
private:
	boost::unordered_map<int, BMW_RobotCFGData> mMapData;
};
