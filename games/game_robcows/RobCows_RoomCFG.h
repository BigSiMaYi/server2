#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct RobCows_RoomCFGData
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
	//��ׯ�б�
	std::vector<int> mRobBankerList;
	//��ע�б�
	std::vector<int> mBetList;
	//��ʵ�����
	int mRealCount;
	//�ȴ������
	int mWaitCount;
	//��������
	int mTableCount;
	//�Ƿ񿪷�
	bool mIsOpen;
};

class RobCows_RoomCFG
{
public:
private:
	static std::auto_ptr<RobCows_RoomCFG> msSingleton;
public:
	int GetCount();
	const RobCows_RoomCFGData* GetData(int RoomID);
	boost::unordered_map<int, RobCows_RoomCFGData>& GetMapData();
	void Reload();
	void Load();
	static RobCows_RoomCFG* GetSingleton();
private:
	boost::unordered_map<int, RobCows_RoomCFGData> mMapData;
};
