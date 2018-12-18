#include "stdafx.h"
#include "tinyxml2/tinyxml2.h"
#include "LuaCfgHelper.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "DragonTiger_CardsCFG.h"
std::auto_ptr<DragonTiger_CardsCFG> DragonTiger_CardsCFG::msSingleton(nullptr);

int DragonTiger_CardsCFG::GetCount()
{
	return (int)mMapData.size();
}

const DragonTiger_CardsCFGData* DragonTiger_CardsCFG::GetData(int CardsID)
{
	auto it = mMapData.find(CardsID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

const std::map<int, DragonTiger_CardsCFGData>& DragonTiger_CardsCFG::GetMapData()
{
	return mMapData;
}

void DragonTiger_CardsCFG::Load()
{
	tinyxml2::XMLDocument xmlDoc;
	std::string content = FileUtils::getInstance()->getStringFromFile("Config/DragonTiger_CardsCFG.xml");
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
		DragonTiger_CardsCFGData data;
		data.mCardsID = element->IntAttribute("CardsID");
		data.mCardsName = element->Attribute("CardsName");
		data.mCardsRate = element->IntAttribute("CardsRate");
		data.mCardsTypeStr = element->Attribute("CardsTypeStr");
		data.mSoundID = element->IntAttribute("SoundID");
		data.mCardsAnimation = element->Attribute("CardsAnimation");
		if (mMapData.find(data.mCardsID) != mMapData.end())std::cout <<"data refind:" << data.mCardsID << std::endl;
		CCASSERT(mMapData.find(data.mCardsID) == mMapData.end(), "data.mCardsID is exists");
		mMapData.insert(std::make_pair(data.mCardsID, data));
		element = element->NextSiblingElement();
	}
	CCLOG("DragonTiger_CardsCFG Loaded. Load Data:%u", mMapData.size());
}

void DragonTiger_CardsCFG::LoadLua()
{
	LuaEngine::getInstance()->executeScriptFile("Config/DragonTiger_CardsCFG");
	lua_State* L = LuaEngine::getInstance()->getLuaStack()->getLuaState();
	lua_getglobal(L, "DragonTiger_CardsCFG");
	CCASSERT(lua_istable(L, -1) == 1, "is not table");
	lua_pushstring(L, "datas");
	lua_gettable(L, -2);
	CCASSERT(lua_istable(L, -1) == 1, "is not table");
	lua_pushnil(L);
	while(lua_next(L, 2))
	{
		CCASSERT(lua_istable(L, -1) == 1, "is not table");
		DragonTiger_CardsCFGData data;
		LuaCfgHelper::readInt(L, "CardsID", data.mCardsID);
		LuaCfgHelper::readString(L, "CardsName", data.mCardsName);
		LuaCfgHelper::readInt(L, "CardsRate", data.mCardsRate);
		LuaCfgHelper::readString(L, "CardsTypeStr", data.mCardsTypeStr);
		LuaCfgHelper::readInt(L, "SoundID", data.mSoundID);
		LuaCfgHelper::readString(L, "CardsAnimation", data.mCardsAnimation);
		if (mMapData.find(data.mCardsID) != mMapData.end())std::cout <<"data refind:" << data.mCardsID << std::endl;
		CCASSERT(mMapData.find(data.mCardsID) == mMapData.end(), "data.mCardsID is exists");
		mMapData.insert(std::make_pair(data.mCardsID, data));
		lua_pop(L, 1);
	}
	lua_settop(L, 0);
	CCLOG("DragonTiger_CardsCFG Loaded. Load Data:%u", mMapData.size());
}

void DragonTiger_CardsCFG::Reload()
{
	mMapData.clear();
	Load();
}

DragonTiger_CardsCFG* DragonTiger_CardsCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new DragonTiger_CardsCFG());
	}
	return msSingleton.get();
}
