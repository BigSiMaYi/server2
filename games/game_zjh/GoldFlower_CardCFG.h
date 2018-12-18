#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct GoldFlower_CardCFGData
{
	//·¿¼äid
	int mRoomID;
	//Ï´ÅÆ¸ÅÂÊ-ABCDE
	std::vector<int> mCardModeRate;
};

class GoldFlower_CardCFG
{
public:
private:
	static std::auto_ptr<GoldFlower_CardCFG> msSingleton;
public:
	int GetCount();
	const GoldFlower_CardCFGData* GetData(int RoomID);
	boost::unordered_map<int, GoldFlower_CardCFGData>& GetMapData();
	void Reload();
	void Load();
	static GoldFlower_CardCFG* GetSingleton();
private:
	boost::unordered_map<int, GoldFlower_CardCFGData> mMapData;
};
