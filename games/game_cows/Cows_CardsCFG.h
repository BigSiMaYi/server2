#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct Cows_CardsCFGData
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

class Cows_CardsCFG
{
public:
private:
	static std::auto_ptr<Cows_CardsCFG> msSingleton;
public:
	int GetCount();
	const Cows_CardsCFGData* GetData(int CardsID);
	boost::unordered_map<int, Cows_CardsCFGData>& GetMapData();
	void Reload();
	void Load();
	static Cows_CardsCFG* GetSingleton();
private:
	boost::unordered_map<int, Cows_CardsCFGData> mMapData;
};
