#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct GoldFlower_RobotActionCFGData
{
	//牌型
	int mModelID;
	//牌型
	std::string mModelName;
	//牌型
	std::vector<int> mModelData;
	//加注系数
	float mAddRateParam;
	//加注百分比
	std::vector<int> mAddRateList;
	//比牌系数
	float mCmpParam1;
	//比牌系数
	float mCmpParam2;
	//比牌系数
	float mCmpParam3;
	//全压系数
	float mAllInParam1;
	//全压系数
	float mAllInParam2;
	//弃牌系数
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
