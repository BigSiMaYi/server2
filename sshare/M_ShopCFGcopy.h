#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct M_ShopCFGcopyData
{
	//��ֵID
	int mID;
	//����
	std::string mName;
	//����
	std::string mDesc;
	//����ICON
	std::string mNameIcon;
	//ICON
	std::string mIcon;
	//���� 1��� 2�¿� 3���
	int mType;
	//�۸�(��ʯ)
	int mPrice;
	//���
	int mGold;
	//��ʾ���
	int mIndex;
	//��ʾ�̵�
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
