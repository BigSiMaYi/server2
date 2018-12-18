#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct DragonTiger_MultiLanguageCFGData
{
	//Key
	std::string mID;
	//ÖÐÎÄÃû×Ö
	std::string mName;
};

class DragonTiger_MultiLanguageCFG
{
public:
private:
	static std::auto_ptr<DragonTiger_MultiLanguageCFG> msSingleton;
public:
	int GetCount();
	const DragonTiger_MultiLanguageCFGData* GetData(std::string ID);
	boost::unordered_map<std::string, DragonTiger_MultiLanguageCFGData>& GetMapData();
	void Reload();
	void Load();
	static DragonTiger_MultiLanguageCFG* GetSingleton();
private:
	boost::unordered_map<std::string, DragonTiger_MultiLanguageCFGData> mMapData;
};
