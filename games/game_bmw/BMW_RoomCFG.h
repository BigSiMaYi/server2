#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct BMW_RoomCFGData
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
	//��ȯ����
	int mBetCondition;
	//��ׯ����
	int mBankerGold;
	//��ׯ����
	int mBankerGoldExtra;
	//��ׯ����
	int mBankerRound;
	//��ׯ����
	int mBankerRoundExtra;
	//��ע����
	int mPlayerLimit;
	//��������
	int mAreaLimit;
	//��ע����
	std::vector<int> mPlaceJetton;
	//��ע����ͼƬ
	std::vector<std::string> mChipImages;
	//��ʷ��¼
	int mRecordHistory;
	//ϵͳׯ
	bool mIsSysBanker;
	//����
	int mMaxPlayers;
	//��������
	int mTableCount;
	//�Ƿ񿪷�
	bool mIsOpen;
};

class BMW_RoomCFG
{
public:
private:
	static std::auto_ptr<BMW_RoomCFG> msSingleton;
public:
	int GetCount();
	const BMW_RoomCFGData* GetData(int RoomID);
	boost::unordered_map<int, BMW_RoomCFGData>& GetMapData();
	void Reload();
	void Load();
	static BMW_RoomCFG* GetSingleton();
private:
	boost::unordered_map<int, BMW_RoomCFGData> mMapData;
};
