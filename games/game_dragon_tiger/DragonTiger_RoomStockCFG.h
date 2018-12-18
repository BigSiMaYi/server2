#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct DragonTiger_RoomStockCFGData
{
	//����id
	int mRoomID;
	//ӮǮ��ˮ
	float mDeduct_1;
	//��Ǯ��ˮ
	float mDeduct_2;
	//���Ż�����
	int mIsOpen;
	//����������
	int mRobotMinCount;
	//����������
	int mRobotMaxCount;
	//������VIP
	int mRobotMinVip;
	//������VIP
	int mRobotMaxVip;
	//������ʱ��
	int mRobotMinLifeTime;
	//������ʱ��
	int mRobotMaxLifeTime;
	//��������С��ע
	int mRobotSystemMinBet;
	//�����������ע
	int mRobotSystemMaxBet;
	//������ÿ����עʱ����
	int mRobotMinBetTime;
	//������ÿ����עʱ����
	int mRobotMaxBetTime;
	//�����˲���ע�ĸ��ʣ��ٷֱȣ�
	int mRobotCannotBet;
};

class DragonTiger_RoomStockCFG
{
public:
private:
	static std::auto_ptr<DragonTiger_RoomStockCFG> msSingleton;
public:
	int GetCount();
	const DragonTiger_RoomStockCFGData* GetData(int RoomID);
	boost::unordered_map<int, DragonTiger_RoomStockCFGData>& GetMapData();
	void Reload();
	void Load();
	static DragonTiger_RoomStockCFG* GetSingleton();
private:
	boost::unordered_map<int, DragonTiger_RoomStockCFGData> mMapData;
};
