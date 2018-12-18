#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct RobCows_BaseInfoData
{
	//key
	std::string mKey;
	//ÊýÖµ
	int mValue;
};

class RobCows_BaseInfo
{
public:
private:
	static std::auto_ptr<RobCows_BaseInfo> msSingleton;
public:
	int GetCount();
	const RobCows_BaseInfoData* GetData(std::string Key);
	boost::unordered_map<std::string, RobCows_BaseInfoData>& GetMapData();
	void Reload();
	void Load();
	static RobCows_BaseInfo* GetSingleton();
private:
	boost::unordered_map<std::string, RobCows_BaseInfoData> mMapData;
};
