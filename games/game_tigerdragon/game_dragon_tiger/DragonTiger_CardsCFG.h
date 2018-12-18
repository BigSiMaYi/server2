#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
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
	boost::unordered_map<int, DragonTiger_CardsCFGData>& GetMapData();
	void Reload();
	void Load();
	static DragonTiger_CardsCFG* GetSingleton();
private:
	boost::unordered_map<int, DragonTiger_CardsCFGData> mMapData;
};
