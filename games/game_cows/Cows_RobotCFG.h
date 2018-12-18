#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct Cows_RobotCFGData
{
	//key
	std::string mKey;
	//ÊýÖµ
	int mValue;
};

class Cows_RobotCFG
{
public:
private:
	static std::auto_ptr<Cows_RobotCFG> msSingleton;
public:
	int GetCount();
	const Cows_RobotCFGData* GetData(std::string Key);
	boost::unordered_map<std::string, Cows_RobotCFGData>& GetMapData();
	void Reload();
	void Load();
	static Cows_RobotCFG* GetSingleton();
private:
	boost::unordered_map<std::string, Cows_RobotCFGData> mMapData;
};
