#pragma once
#include <map>
struct DragonTiger_BaseInfoData
{
	//key
	std::string mKey;
	//ÊýÖµ
	int mValue;
};

class DragonTiger_BaseInfo
{
public:
private:
	static std::auto_ptr<DragonTiger_BaseInfo> msSingleton;
public:
	int GetCount();
	const DragonTiger_BaseInfoData* GetData(std::string Key);
	const std::map<std::string, DragonTiger_BaseInfoData>& GetMapData();
	void Load();
	void LoadLua();
	void Reload();
	static DragonTiger_BaseInfo* GetSingleton();
private:
	std::map<std::string, DragonTiger_BaseInfoData> mMapData;
};
