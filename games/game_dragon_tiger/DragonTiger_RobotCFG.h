#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct DragonTiger_RobotCFGData
{
	//key
	std::string mKey;
	//ÊýÖµ
	int mValue;
};

class DragonTiger_RobotCFG
{
public:
private:
	static std::auto_ptr<DragonTiger_RobotCFG> msSingleton;
public:
	int GetCount();
	const DragonTiger_RobotCFGData* GetData(std::string Key);
	boost::unordered_map<std::string, DragonTiger_RobotCFGData>& GetMapData();
	void Reload();
	void Load();
	static DragonTiger_RobotCFG* GetSingleton();
private:
	boost::unordered_map<std::string, DragonTiger_RobotCFGData> mMapData;
};
