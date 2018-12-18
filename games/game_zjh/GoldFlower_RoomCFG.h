#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct GoldFlower_RoomCFGData
{
	//����id
	int mRoomID;
	//������
	std::string mRoomName;
	//����ͼƬ
	std::string mRoomImage;
	//�������
	int mGoldMinCondition;
	//�������
	int mGoldMaxCondition;
	//vip����
	int mVipCondition;
	//��ȯ����
	int mTicketCondition;
	//�׷�
	int mBaseCondition;
	//��ע�����б�
	std::vector<int> mAddChipList;
	//��ע����ͼƬ
	std::vector<std::string> mAddChipImages;
	//��������
	int mLookCondition;
	//��������
	int mPkCondition;
	//������
	int mMaxChip;
	//�������
	int mMaxRound;
	//����ܳ���
	int mMaxPool;
	//��������
	int mTableCount;
	//��������
	int mTableManCount;
	//����������
	int mTableRobotCount;
	//�Ƿ񿪷�
	bool mIsOpen;
};

class GoldFlower_RoomCFG
{
public:
private:
	static std::auto_ptr<GoldFlower_RoomCFG> msSingleton;
public:
	int GetCount();
	const GoldFlower_RoomCFGData* GetData(int RoomID);
	boost::unordered_map<int, GoldFlower_RoomCFGData>& GetMapData();
	void Reload();
	void Load();
	static GoldFlower_RoomCFG* GetSingleton();
private:
	boost::unordered_map<int, GoldFlower_RoomCFGData> mMapData;
};
