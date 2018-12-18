#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct DragonTiger_WeightCFGData
{
	//索引
	int mIndex;
	//水位
	float mWater;
	//水位对应的杀分概率
	int mKillScoreRate;
	//是否触发控制
	int mTriggerControl;
};

class DragonTiger_WeightCFG
{
public:
private:
	static std::auto_ptr<DragonTiger_WeightCFG> msSingleton;
public:
	int GetCount();
	const DragonTiger_WeightCFGData* GetData(int Index);
	boost::unordered_map<int, DragonTiger_WeightCFGData>& GetMapData();
	void Reload();
	void Load();
	static DragonTiger_WeightCFG* GetSingleton();
private:
	boost::unordered_map<int, DragonTiger_WeightCFGData> mMapData;
};
