#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct RedBlack_BaseInfoData
{
	//key
	std::string mKey;
	//ÊýÖµ
	int mValue;
};

class RedBlack_BaseInfo
{
public:
private:
	static std::auto_ptr<RedBlack_BaseInfo> msSingleton;
public:
	int GetCount();
	const RedBlack_BaseInfoData* GetData(std::string Key);
	boost::unordered_map<std::string, RedBlack_BaseInfoData>& GetMapData();
	void Reload();
	void Load();
	static RedBlack_BaseInfo* GetSingleton();
private:
	boost::unordered_map<std::string, RedBlack_BaseInfoData> mMapData;
};
