#pragma once
#include <map>
struct DragonTiger_CardoddsData
{
	//key
	std::string mKey;
	//ÊýÖµ
	float mValue;
};

class DragonTiger_Cardodds
{
public:
private:
	static std::auto_ptr<DragonTiger_Cardodds> msSingleton;
public:
	int GetCount();
	const DragonTiger_CardoddsData* GetData(std::string Key);
	const std::map<std::string, DragonTiger_CardoddsData>& GetMapData();
	void Load();
	void LoadLua();
	void Reload();
	static DragonTiger_Cardodds* GetSingleton();
private:
	std::map<std::string, DragonTiger_CardoddsData> mMapData;
};
