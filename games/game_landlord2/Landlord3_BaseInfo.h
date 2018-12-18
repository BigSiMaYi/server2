#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct Landlord3_BaseInfoData
{
	//key
	std::string mKey;
	//ÊýÖµ
	int mValue;
};

class Landlord3_BaseInfo
{
public:
private:
	static std::auto_ptr<Landlord3_BaseInfo> msSingleton;
public:
	int GetCount();
	const Landlord3_BaseInfoData* GetData(std::string Key);
	boost::unordered_map<std::string, Landlord3_BaseInfoData>& GetMapData();
	void Reload();
	void Load();
	static Landlord3_BaseInfo* GetSingleton();
private:
	boost::unordered_map<std::string, Landlord3_BaseInfoData> mMapData;
};
