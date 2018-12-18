#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct RedBlack_RobotCFGData
{
	//key
	std::string mKey;
	//ÊýÖµ
	int mValue;
};

class RedBlack_RobotCFG
{
public:
private:
	static std::auto_ptr<RedBlack_RobotCFG> msSingleton;
public:
	int GetCount();
	const RedBlack_RobotCFGData* GetData(std::string Key);
	boost::unordered_map<std::string, RedBlack_RobotCFGData>& GetMapData();
	void Reload();
	void Load();
	static RedBlack_RobotCFG* GetSingleton();
private:
	boost::unordered_map<std::string, RedBlack_RobotCFGData> mMapData;
};
