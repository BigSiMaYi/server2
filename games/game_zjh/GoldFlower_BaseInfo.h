#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct GoldFlower_BaseInfoData
{
	//key
	std::string mKey;
	//ÊýÖµ
	int mValue;
};

class GoldFlower_BaseInfo
{
public:
private:
	static std::auto_ptr<GoldFlower_BaseInfo> msSingleton;
public:
	int GetCount();
	const GoldFlower_BaseInfoData* GetData(std::string Key);
	boost::unordered_map<std::string, GoldFlower_BaseInfoData>& GetMapData();
	void Reload();
	void Load();
	static GoldFlower_BaseInfo* GetSingleton();
private:
	boost::unordered_map<std::string, GoldFlower_BaseInfoData> mMapData;
};
