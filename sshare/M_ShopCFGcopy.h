#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct M_ShopCFGcopyData
{
	//充值ID
	int mID;
	//名字
	std::string mName;
	//描述
	std::string mDesc;
	//名字ICON
	std::string mNameIcon;
	//ICON
	std::string mIcon;
	//类型 1金币 2月卡 3礼包
	int mType;
	//价格(钻石)
	int mPrice;
	//金币
	int mGold;
	//显示序号
	int mIndex;
	//显示商店
	int mShopType;
};

class M_ShopCFGcopy
{
public:
private:
	static std::auto_ptr<M_ShopCFGcopy> msSingleton;
public:
	int GetCount();
	const M_ShopCFGcopyData* GetData(int ID);
	boost::unordered_map<int, M_ShopCFGcopyData>& GetMapData();
	void Reload();
	void Reload(const std::string& path);
	void Load(const std::string& path);
	void Load();
	static M_ShopCFGcopy* GetSingleton();
private:
	boost::unordered_map<int, M_ShopCFGcopyData> mMapData;
};
