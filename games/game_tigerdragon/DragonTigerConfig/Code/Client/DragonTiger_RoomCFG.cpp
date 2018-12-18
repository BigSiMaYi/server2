#include "stdafx.h"
#include "tinyxml2/tinyxml2.h"
#include "LuaCfgHelper.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "DragonTiger_RoomCFG.h"
std::auto_ptr<DragonTiger_RoomCFG> DragonTiger_RoomCFG::msSingleton(nullptr);

int DragonTiger_RoomCFG::GetCount()
{
	return (int)mMapData.size();
}

const DragonTiger_RoomCFGData* DragonTiger_RoomCFG::GetData(int RoomID)
{
	auto it = mMapData.find(RoomID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

const std::map<int, DragonTiger_RoomCFGData>& DragonTiger_RoomCFG::GetMapData()
{
	return mMapData;
}

void DragonTiger_RoomCFG::Load()
{
	tinyxml2::XMLDocument xmlDoc;
	std::string content = FileUtils::getInstance()->getStringFromFile("Config/DragonTiger_RoomCFG.xml");
	auto result = xmlDoc.Parse(content.c_str(), content.length());
	if (result != tinyxml2::XML_SUCCESS)
	{
		CCLOGERROR("Result:%d", result);
		CCASSERT(false, "result != tinyxml2::XML_SUCCESS");
		return;
	}
	auto root = xmlDoc.RootElement();
	if (root == NULL)
	{
		CCASSERT(false, "root == NULL");
		return;
	}
	auto element = root->FirstChildElement("Data");
	while (element != NULL)
	{
		DragonTiger_RoomCFGData data;
		data.mRoomID = element->IntAttribute("RoomID");
		data.mRoomName = element->Attribute("RoomName");
		data.mRoomImage = element->Attribute("RoomImage");
		data.mGoldCondition = element->IntAttribute("GoldCondition");
		data.mVipCondition = element->IntAttribute("VipCondition");
		data.mTicketCondition = element->IntAttribute("TicketCondition");
		data.mMaxRate = element->IntAttribute("MaxRate");
		{
			const char* ChipList = element->Attribute("ChipList");
			std::vector<std::string> vecChipList;
			boost::split(vecChipList, ChipList, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecChipList.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecChipList[i].c_str(), &temp))
				{
					data.mChipList.push_back(temp);
				}
			}
		}
		{
			const char* UnlockChipList = element->Attribute("UnlockChipList");
			std::vector<std::string> vecUnlockChipList;
			boost::split(vecUnlockChipList, UnlockChipList, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecUnlockChipList.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecUnlockChipList[i].c_str(), &temp))
				{
					data.mUnlockChipList.push_back(temp);
				}
			}
		}
		{
			const char* ChipImages = element->Attribute("ChipImages");
			std::vector<std::string> vecChipImages;
			boost::split(vecChipImages, ChipImages, boost::is_any_of(","));
			for (unsigned int i = 0; i < vecChipImages.size(); i++)
			{
				data.mChipImages.push_back(vecChipImages[i]);
			}
		}
		data.mIsOpen = element->BoolAttribute("IsOpen");
		data.mBankerGold = element->IntAttribute("BankerGold");
		data.mSnatchGold = element->IntAttribute("SnatchGold");
		data.mForceLeaveGold = element->IntAttribute("ForceLeaveGold");
		if (mMapData.find(data.mRoomID) != mMapData.end())std::cout <<"data refind:" << data.mRoomID << std::endl;
		CCASSERT(mMapData.find(data.mRoomID) == mMapData.end(), "data.mRoomID is exists");
		mMapData.insert(std::make_pair(data.mRoomID, data));
		element = element->NextSiblingElement();
	}
	CCLOG("DragonTiger_RoomCFG Loaded. Load Data:%u", mMapData.size());
}

void DragonTiger_RoomCFG::LoadLua()
{
	LuaEngine::getInstance()->executeScriptFile("Config/DragonTiger_RoomCFG");
	lua_State* L = LuaEngine::getInstance()->getLuaStack()->getLuaState();
	lua_getglobal(L, "DragonTiger_RoomCFG");
	CCASSERT(lua_istable(L, -1) == 1, "is not table");
	lua_pushstring(L, "datas");
	lua_gettable(L, -2);
	CCASSERT(lua_istable(L, -1) == 1, "is not table");
	lua_pushnil(L);
	while(lua_next(L, 2))
	{
		CCASSERT(lua_istable(L, -1) == 1, "is not table");
		DragonTiger_RoomCFGData data;
		LuaCfgHelper::readInt(L, "RoomID", data.mRoomID);
		LuaCfgHelper::readString(L, "RoomName", data.mRoomName);
		LuaCfgHelper::readString(L, "RoomImage", data.mRoomImage);
		LuaCfgHelper::readInt(L, "GoldCondition", data.mGoldCondition);
		LuaCfgHelper::readInt(L, "VipCondition", data.mVipCondition);
		LuaCfgHelper::readInt(L, "TicketCondition", data.mTicketCondition);
		LuaCfgHelper::readInt(L, "MaxRate", data.mMaxRate);
		LuaCfgHelper::readVectorInt(L, "ChipList", data.mChipList);
		LuaCfgHelper::readVectorInt(L, "UnlockChipList", data.mUnlockChipList);
		LuaCfgHelper::readVectorString(L, "ChipImages", data.mChipImages);
		LuaCfgHelper::readBool(L, "IsOpen", data.mIsOpen);
		LuaCfgHelper::readInt(L, "BankerGold", data.mBankerGold);
		LuaCfgHelper::readInt(L, "SnatchGold", data.mSnatchGold);
		LuaCfgHelper::readInt(L, "ForceLeaveGold", data.mForceLeaveGold);
		if (mMapData.find(data.mRoomID) != mMapData.end())std::cout <<"data refind:" << data.mRoomID << std::endl;
		CCASSERT(mMapData.find(data.mRoomID) == mMapData.end(), "data.mRoomID is exists");
		mMapData.insert(std::make_pair(data.mRoomID, data));
		lua_pop(L, 1);
	}
	lua_settop(L, 0);
	CCLOG("DragonTiger_RoomCFG Loaded. Load Data:%u", mMapData.size());
}

void DragonTiger_RoomCFG::Reload()
{
	mMapData.clear();
	Load();
}

DragonTiger_RoomCFG* DragonTiger_RoomCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new DragonTiger_RoomCFG());
	}
	return msSingleton.get();
}
