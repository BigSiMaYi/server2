#pragma once
#include <map>
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
	const std::map<std::string, DragonTiger_MultiLanguageCFGData>& GetMapData();
	void Load();
	void LoadLua();
	void Reload();
	static DragonTiger_MultiLanguageCFG* GetSingleton();
private:
	std::map<std::string, DragonTiger_MultiLanguageCFGData> mMapData;
};
