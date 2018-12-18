#pragma once
#include <map>
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
	const std::map<int, DragonTiger_WeightCFGData>& GetMapData();
	void Load();
	void LoadLua();
	void Reload();
	static DragonTiger_WeightCFG* GetSingleton();
private:
	std::map<int, DragonTiger_WeightCFGData> mMapData;
};
