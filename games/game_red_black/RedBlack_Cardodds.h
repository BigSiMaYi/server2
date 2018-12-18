#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct RedBlack_CardoddsData
{
	//key
	std::string mKey;
	//ÊýÖµ
	int mValue;
};

class RedBlack_Cardodds
{
public:
private:
	static std::auto_ptr<RedBlack_Cardodds> msSingleton;
public:
	int GetCount();
	const RedBlack_CardoddsData* GetData(std::string Key);
	boost::unordered_map<std::string, RedBlack_CardoddsData>& GetMapData();
	void Reload();
	void Load();
	static RedBlack_Cardodds* GetSingleton();
private:
	boost::unordered_map<std::string, RedBlack_CardoddsData> mMapData;
};
