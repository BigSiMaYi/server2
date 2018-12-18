#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct GoldFlower_RobotGameCFGData
{
	//����id
	int mRoomID;
	//׼��ʱ��
	float mReadyTimeMin;
	//׼��ʱ��
	float mReadyTimeMax;
	//����ʱ��
	float mOperaTimeMin;
	//����ʱ��
	float mOperaTimeMax;
	//����ʱ��
	float mRobotResultTime;
	//���Ʋ���
	float mParam1;
	//���Ʋ���
	float mParam2;
	//���Ʋ���
	float mParam3;
	//���Ʋ���
	float mParam4;
};

class GoldFlower_RobotGameCFG
{
public:
private:
	static std::auto_ptr<GoldFlower_RobotGameCFG> msSingleton;
public:
	int GetCount();
	const GoldFlower_RobotGameCFGData* GetData(int RoomID);
	boost::unordered_map<int, GoldFlower_RobotGameCFGData>& GetMapData();
	void Reload();
	void Load();
	static GoldFlower_RobotGameCFG* GetSingleton();
private:
	boost::unordered_map<int, GoldFlower_RobotGameCFGData> mMapData;
};
