#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct DragonTiger_CardoddsData
{
	//key
	std::string mKey;
	//ÊýÖµ
	float mValue;
};

class DragonTiger_Cardodds
{
public:
private:
	static std::auto_ptr<DragonTiger_Cardodds> msSingleton;
public:
	int GetCount();
	const DragonTiger_CardoddsData* GetData(std::string Key);
	boost::unordered_map<std::string, DragonTiger_CardoddsData>& GetMapData();
	void Reload();
	void Load();
	static DragonTiger_Cardodds* GetSingleton();
private:
	boost::unordered_map<std::string, DragonTiger_CardoddsData> mMapData;
};
