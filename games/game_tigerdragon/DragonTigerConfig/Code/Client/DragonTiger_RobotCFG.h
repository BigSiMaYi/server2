#pragma once
#include <map>
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
	const std::map<std::string, DragonTiger_RobotCFGData>& GetMapData();
	void Load();
	void LoadLua();
	void Reload();
	static DragonTiger_RobotCFG* GetSingleton();
private:
	std::map<std::string, DragonTiger_RobotCFGData> mMapData;
};
