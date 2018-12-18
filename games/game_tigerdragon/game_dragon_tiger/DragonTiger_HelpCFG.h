#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct DragonTiger_HelpCFGData
{
	//����ID
	int mHelpID;
	//������
	std::string mCardsName;
	//������Ϣ
	std::string mCardsInfo;
	//������
	std::string mCardsTypeStr;
	//�˿�
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
