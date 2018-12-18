#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct Cows_RoomCFGData
{
	//����id
	int mRoomID;
	//������
	std::string mRoomName;
	//����ͼƬ
	std::string mRoomImage;
	//�������
	int mGoldCondition;
	//vip����
	int mVipCondition;
	//��ȯ����
	int mTicketCondition;
	//�����
	int mMaxRate;
	//�����б�
	std::vector<int> mChipList;
	//���������б�
	std::vector<int> mUnlockChipList;
	//����ͼƬ
	std::vector<std::string> mChipImages;
	//�Ƿ񿪷�
	bool mIsOpen;
	//��ׯ���
	int mBankerGold;
	//��ׯ���Ľ��
	int mSnatchGold;
	//ǿ����ׯ���
	int mForceLeaveGold;
	//��������
	int mMaxPlayerCnt;
	//����ӯ������
	int mExpectEarnRate;
	//���ӯ������
	int mMaxEarnRate;
	//��Сӯ������
	int mMinEarnRate;
	//ɱ�ֿ���
	bool mKillPointsSwtch;
};

class Cows_RoomCFG
{
public:
private:
	static std::auto_ptr<Cows_RoomCFG> msSingleton;
public:
	int GetCount();
	const Cows_RoomCFGData* GetData(int RoomID);
	boost::unordered_map<int, Cows_RoomCFGData>& GetMapData();
	void Reload();
	void Load();
	static Cows_RoomCFG* GetSingleton();
private:
	boost::unordered_map<int, Cows_RoomCFGData> mMapData;
};
