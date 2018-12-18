#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct Cows_HelpCFGData
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

class Cows_HelpCFG
{
public:
private:
	static std::auto_ptr<Cows_HelpCFG> msSingleton;
public:
	int GetCount();
	const Cows_HelpCFGData* GetData(int HelpID);
	boost::unordered_map<int, Cows_HelpCFGData>& GetMapData();
	void Reload();
	void Load();
	static Cows_HelpCFG* GetSingleton();
private:
	boost::unordered_map<int, Cows_HelpCFGData> mMapData;
};
