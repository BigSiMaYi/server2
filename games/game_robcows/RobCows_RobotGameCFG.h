#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct RobCows_RobotGameCFGData
{
	//����id
	int mRoomID;
	//׼��ʱ��
	int mReadyTime;
	//��ׯʱ��
	int mBankerTime;
	//��ׯ�б�
	std::vector<int> mRobList;
	//��עʱ��
	int mBetTime;
	//��ׯ�б�
	std::vector<int> mBetList;
	//ƴţʱ��
	int mOperaTime;
	//����ʱ��
	int mResultTime;
};

class RobCows_RobotGameCFG
{
public:
private:
	static std::auto_ptr<RobCows_RobotGameCFG> msSingleton;
public:
	int GetCount();
	const RobCows_RobotGameCFGData* GetData(int RoomID);
	boost::unordered_map<int, RobCows_RobotGameCFGData>& GetMapData();
	void Reload();
	void Load();
	static RobCows_RobotGameCFG* GetSingleton();
private:
	boost::unordered_map<int, RobCows_RobotGameCFGData> mMapData;
};
