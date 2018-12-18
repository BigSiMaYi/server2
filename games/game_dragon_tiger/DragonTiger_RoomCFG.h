#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct DragonTiger_RoomCFGData
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
	//�����б�
	std::vector<int> mChipList;
	//���������б�
	std::vector<int> mUnlockChipList;
	//����ͼƬ
	std::vector<std::string> mChipImages;
	//���ͺ��������ѹע����
	std::vector<int> mBetAreaGoldCond;
	//��ע����
	int mBetGoldCondition;
	//�Ƿ񿪷�
	bool mIsOpen;
};

class DragonTiger_RoomCFG
{
public:
private:
	static std::auto_ptr<DragonTiger_RoomCFG> msSingleton;
public:
	int GetCount();
	const DragonTiger_RoomCFGData* GetData(int RoomID);
	boost::unordered_map<int, DragonTiger_RoomCFGData>& GetMapData();
	void Reload();
	void Load();
	static DragonTiger_RoomCFG* GetSingleton();
private:
	boost::unordered_map<int, DragonTiger_RoomCFGData> mMapData;
};
