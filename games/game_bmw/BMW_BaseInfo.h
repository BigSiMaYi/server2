#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct BMW_BaseInfoData
{
	//key
	std::string mKey;
	//ÊýÖµ
	int mValue;
};

class BMW_BaseInfo
{
public:
private:
	static std::auto_ptr<BMW_BaseInfo> msSingleton;
public:
	int GetCount();
	const BMW_BaseInfoData* GetData(std::string Key);
	boost::unordered_map<std::string, BMW_BaseInfoData>& GetMapData();
	void Reload();
	void Load();
	static BMW_BaseInfo* GetSingleton();
private:
	boost::unordered_map<std::string, BMW_BaseInfoData> mMapData;
};
