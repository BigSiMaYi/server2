#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct DragonTiger_HelpCFGData
{
	//帮助ID
	int mHelpID;
	//牌名称
	std::string mCardsName;
	//牌名信息
	std::string mCardsInfo;
	//牌类型
	std::string mCardsTypeStr;
	//扑克
	std::vector<int> mPokers;
};

class DragonTiger_HelpCFG
{
public:
private:
	static std::auto_ptr<DragonTiger_HelpCFG> msSingleton;
public:
	int GetCount();
	const DragonTiger_HelpCFGData* GetData(int HelpID);
	boost::unordered_map<int, DragonTiger_HelpCFGData>& GetMapData();
	void Reload();
	void Load();
	static DragonTiger_HelpCFG* GetSingleton();
private:
	boost::unordered_map<int, DragonTiger_HelpCFGData> mMapData;
};
