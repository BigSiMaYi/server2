#pragma once
#include <map>
struct DragonTiger_WeightCFGData
{
	//����
	int mIndex;
	//ˮλ
	float mWater;
	//ˮλ��Ӧ��ɱ�ָ���
	int mKillScoreRate;
	//�Ƿ񴥷�����
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
