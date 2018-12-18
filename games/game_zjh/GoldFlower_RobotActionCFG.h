#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct GoldFlower_RobotActionCFGData
{
	//����
	int mModelID;
	//����
	std::string mModelName;
	//����
	std::vector<int> mModelData;
	//��עϵ��
	float mAddRateParam;
	//��ע�ٷֱ�
	std::vector<int> mAddRateList;
	//����ϵ��
	float mCmpParam1;
	//����ϵ��
	float mCmpParam2;
	//����ϵ��
	float mCmpParam3;
	//ȫѹϵ��
	float mAllInParam1;
	//ȫѹϵ��
	float mAllInParam2;
	//����ϵ��
	float mGiveUpParam;
};

class GoldFlower_RobotActionCFG
{
public:
private:
	static std::auto_ptr<GoldFlower_RobotActionCFG> msSingleton;
public:
	int GetCount();
	const GoldFlower_RobotActionCFGData* GetData(int ModelID);
	boost::unordered_map<int, GoldFlower_RobotActionCFGData>& GetMapData();
	void Reload();
	void Load();
	static GoldFlower_RobotActionCFG* GetSingleton();
private:
	boost::unordered_map<int, GoldFlower_RobotActionCFGData> mMapData;
};
