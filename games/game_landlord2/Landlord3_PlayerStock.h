#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct Landlord3_PlayerStockData
{
	//��ֵ��Χ
	int mID;
	//ɱ�ֱ��
	bool mCutRoundFlag;
	//��ֵ��Χ
	std::vector<int> mTopUpRang;
	//����(������Сֵ�����ֵ��ϵ���������˿��ء�)
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
