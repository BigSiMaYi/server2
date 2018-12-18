#pragma once
#include <map>
struct DragonTiger_RoomStockCFGData
{
	//����id
	int mRoomID;
	//ӮǮ��ˮ
	float mDeduct_1;
	//��Ǯ��ˮ
	float mDeduct_2;
	//Ĭ�Ͽ��
	int mDefaultStock;
	//���ˮλ
	std::vector<int> mStock;
	//�ı�Ч��
	std::vector<int> mChangeBuff;
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
	//��ׯ��Ǯ
	int mRobotBankerGold;
	//ϵͳ��ׯ��ע
	int mRobotSystemMaxBet;
	//ϵͳ��ׯ��ע
	int mRobotSystemMinBet;
	//�����ׯ��ע(�ٷֱ�)
	int mRobotPlayerMinBet;
	//�����ׯ��ע(�ٷֱ�)
	int mRobotPlayerMaxBet;
	//��������ׯ��ע(�ٷֱ�)
	int mRobotRobotMinBet;
	//��������ׯ��ע(�ٷֱ�)
	int mRobotRobotMaxBet;
};

class DragonTiger_RoomStockCFG
{
public:
private:
	static std::auto_ptr<DragonTiger_RoomStockCFG> msSingleton;
public:
	int GetCount();
	const DragonTiger_RoomStockCFGData* GetData(int RoomID);
	const std::map<int, DragonTiger_RoomStockCFGData>& GetMapData();
	void Load();
	void LoadLua();
	void Reload();
	static DragonTiger_RoomStockCFG* GetSingleton();
private:
	std::map<int, DragonTiger_RoomStockCFGData> mMapData;
};
