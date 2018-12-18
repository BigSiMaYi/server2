#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct BMW_ResultCtrlData
{
	//key
	std::string mKey;
	//ÊýÖµ
	int mValue;
};

class BMW_ResultCtrl
{
public:
private:
	static std::auto_ptr<BMW_ResultCtrl> msSingleton;
public:
	int GetCount();
	const BMW_ResultCtrlData* GetData(std::string Key);
	boost::unordered_map<std::string, BMW_ResultCtrlData>& GetMapData();
	void Reload();
	void Load();
	static BMW_ResultCtrl* GetSingleton();
private:
	boost::unordered_map<std::string, BMW_ResultCtrlData> mMapData;
};
