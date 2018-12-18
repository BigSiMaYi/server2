#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct M_DialLotteryTypeCFGData
{
	//编号
	int mID;
	//类型
	int mType;
	//数量
	int mCount;
	//抽几次后使用Prob2的概率
	int mProbCount;
};

class M_DialLotteryTypeCFG
{
public:
private:
	static std::auto_ptr<M_DialLotteryTypeCFG> msSingleton;
public:
	int GetCount();
	const M_DialLotteryTypeCFGData* GetData(int ID);
	boost::unordered_map<int, M_DialLotteryTypeCFGData>& GetMapData();
	void Reload();
	void Reload(const std::string& path);
	void Load(const std::string& path);
	void Load();
	static M_DialLotteryTypeCFG* GetSingleton();
private:
	boost::unordered_map<int, M_DialLotteryTypeCFGData> mMapData;
};
