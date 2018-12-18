#pragma once
#include <map>
struct DragonTiger_CardsCFGData
{
	//����id
	int mCardsID;
	//������
	std::string mCardsName;
	//�Ʊ���
	int mCardsRate;
	//������
	std::string mCardsTypeStr;
	//����ID
	int mSoundID;
	//����
	std::string mCardsAnimation;
};

class DragonTiger_CardsCFG
{
public:
private:
	static std::auto_ptr<DragonTiger_CardsCFG> msSingleton;
public:
	int GetCount();
	const DragonTiger_CardsCFGData* GetData(int CardsID);
	const std::map<int, DragonTiger_CardsCFGData>& GetMapData();
	void Load();
	void LoadLua();
	void Reload();
	static DragonTiger_CardsCFG* GetSingleton();
private:
	std::map<int, DragonTiger_CardsCFGData> mMapData;
};
