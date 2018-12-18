#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct Landlord3_PlayerStockData
{
	//充值范围
	int mID;
	//杀分标记
	bool mCutRoundFlag;
	//充值范围
	std::vector<int> mTopUpRang;
	//参数(区间最小值，最大值，系数，机器人开关…)
	std::vector<int> mParam;
};

class Landlord3_PlayerStock
{
public:
private:
	static std::auto_ptr<Landlord3_PlayerStock> msSingleton;
public:
	int GetCount();
	const Landlord3_PlayerStockData* GetData(int ID);
	boost::unordered_map<int, Landlord3_PlayerStockData>& GetMapData();
	void Reload();
	void Reload(const std::string& path);
	void Load(const std::string& path);
	void Load();
	static Landlord3_PlayerStock* GetSingleton();
private:
	boost::unordered_map<int, Landlord3_PlayerStockData> mMapData;
};
