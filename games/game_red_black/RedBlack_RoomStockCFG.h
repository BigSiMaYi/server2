#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct RedBlack_RoomStockCFGData
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
	//��������עʱ��1
	int mRobotMinBetTime;
	//��������עʱ��1
	int mRobotMaxBetTime;
};

class RedBlack_RoomStockCFG
{
public:
private:
	static std::auto_ptr<RedBlack_RoomStockCFG> msSingleton;
public:
	int GetCount();
	const RedBlack_RoomStockCFGData* GetData(int RoomID);
	boost::unordered_map<int, RedBlack_RoomStockCFGData>& GetMapData();
	void Reload();
	void Load();
	static RedBlack_RoomStockCFG* GetSingleton();
private:
	boost::unordered_map<int, RedBlack_RoomStockCFGData> mMapData;
};
