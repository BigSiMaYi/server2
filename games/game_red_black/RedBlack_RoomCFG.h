#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct RedBlack_RoomCFGData
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
	//�죬���ˣ�����(��ע����������)
	std::vector<int> mBetAreaGoldCond;
	//������ע�������
	int mBetGoldCondition;
	//��������������ע
	int mRobotLuckAreaBetCond;
};

class RedBlack_RoomCFG
{
public:
private:
	static std::auto_ptr<RedBlack_RoomCFG> msSingleton;
public:
	int GetCount();
	const RedBlack_RoomCFGData* GetData(int RoomID);
	boost::unordered_map<int, RedBlack_RoomCFGData>& GetMapData();
	void Reload();
	void Load();
	static RedBlack_RoomCFG* GetSingleton();
private:
	boost::unordered_map<int, RedBlack_RoomCFGData> mMapData;
};
