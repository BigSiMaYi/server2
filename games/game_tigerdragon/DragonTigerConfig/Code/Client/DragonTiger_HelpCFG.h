#pragma once
#include <map>
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
	const std::map<int, DragonTiger_HelpCFGData>& GetMapData();
	void Load();
	void LoadLua();
	void Reload();
	static DragonTiger_HelpCFG* GetSingleton();
private:
	std::map<int, DragonTiger_HelpCFGData> mMapData;
};
