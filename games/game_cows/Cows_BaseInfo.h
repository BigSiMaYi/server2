#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct Cows_BaseInfoData
{
	//key
	std::string mKey;
	//ÊýÖµ
	int mValue;
};

class Cows_BaseInfo
{
public:
private:
	static std::auto_ptr<Cows_BaseInfo> msSingleton;
public:
	int GetCount();
	const Cows_BaseInfoData* GetData(std::string Key);
	boost::unordered_map<std::string, Cows_BaseInfoData>& GetMapData();
	void Reload();
	void Load();
	static Cows_BaseInfo* GetSingleton();
private:
	boost::unordered_map<std::string, Cows_BaseInfoData> mMapData;
};
