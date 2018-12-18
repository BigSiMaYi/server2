#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
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
	boost::unordered_map<std::string, DragonTiger_BaseInfoData>& GetMapData();
	void Reload();
	void Load();
	static DragonTiger_BaseInfo* GetSingleton();
private:
	boost::unordered_map<std::string, DragonTiger_BaseInfoData> mMapData;
};
