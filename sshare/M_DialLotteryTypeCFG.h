#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct M_DialLotteryTypeCFGData
{
	//���
	int mID;
	//����
	int mType;
	//����
	int mCount;
	//�鼸�κ�ʹ��Prob2�ĸ���
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
