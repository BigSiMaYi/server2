#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct Cows_MultiLanguageCFGData
{
	//Key
	std::string mID;
	//ÖÐÎÄÃû×Ö
	std::string mName;
};

class Cows_MultiLanguageCFG
{
public:
private:
	static std::auto_ptr<Cows_MultiLanguageCFG> msSingleton;
public:
	int GetCount();
	const Cows_MultiLanguageCFGData* GetData(std::string ID);
	boost::unordered_map<std::string, Cows_MultiLanguageCFGData>& GetMapData();
	void Reload();
	void Load();
	static Cows_MultiLanguageCFG* GetSingleton();
private:
	boost::unordered_map<std::string, Cows_MultiLanguageCFGData> mMapData;
};
