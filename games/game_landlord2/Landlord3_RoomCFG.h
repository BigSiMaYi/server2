#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct Landlord3_RoomCFGData
{
	//����id
	int mRoomID;
	//������
	std::string mRoomName;
	//����ͼƬ
	std::string mRoomImage;
	//�������
	int mKickGoldCondition;
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
	//��������
	int mTableCount;
	//��ʵ����������
	int mPlayerMaxCounter;
	//�Ƿ񿪷�
	bool mIsOpen;
};

class Landlord3_RoomCFG
{
public:
private:
	static std::auto_ptr<Landlord3_RoomCFG> msSingleton;
public:
	int GetCount();
	const Landlord3_RoomCFGData* GetData(int RoomID);
	boost::unordered_map<int, Landlord3_RoomCFGData>& GetMapData();
	void Reload();
	void Reload(const std::string& path);
	void Load(const std::string& path);
	void Load();
	static Landlord3_RoomCFG* GetSingleton();
private:
	boost::unordered_map<int, Landlord3_RoomCFGData> mMapData;
};
