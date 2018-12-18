#include "stdafx.h"
#include "tinyxml2/tinyxml2.h"
#include "LuaCfgHelper.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "DragonTiger_WeightCFG.h"
std::auto_ptr<DragonTiger_WeightCFG> DragonTiger_WeightCFG::msSingleton(nullptr);

int DragonTiger_WeightCFG::GetCount()
{
	return (int)mMapData.size();
}

const DragonTiger_WeightCFGData* DragonTiger_WeightCFG::GetData(int Index)
{
	auto it = mMapData.find(Index);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

const std::map<int, DragonTiger_WeightCFGData>& DragonTiger_WeightCFG::GetMapData()
{
	return mMapData;
}

void DragonTiger_WeightCFG::Load()
{
	tinyxml2::XMLDocument xmlDoc;
	std::string content = FileUtils::getInstance()->getStringFromFile("Config/DragonTiger_WeightCFG.xml");
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
		DragonTiger_WeightCFGData data;
		data.mIndex = element->IntAttribute("Index");
		data.mWater = element->FloatAttribute("Water");
		data.mKillScoreRate = element->IntAttribute("KillScoreRate");
		data.mTriggerControl = element->IntAttribute("TriggerControl");
		if (mMapData.find(data.mIndex) != mMapData.end())std::cout <<"data refind:" << data.mIndex << std::endl;
		CCASSERT(mMapData.find(data.mIndex) == mMapData.end(), "data.mIndex is exists");
		mMapData.insert(std::make_pair(data.mIndex, data));
		element = element->NextSiblingElement();
	}
	CCLOG("DragonTiger_WeightCFG Loaded. Load Data:%u", mMapData.size());
}

void DragonTiger_WeightCFG::LoadLua()
{
	LuaEngine::getInstance()->executeScriptFile("Config/DragonTiger_WeightCFG");
	lua_State* L = LuaEngine::getInstance()->getLuaStack()->getLuaState();
	lua_getglobal(L, "DragonTiger_WeightCFG");
	CCASSERT(lua_istable(L, -1) == 1, "is not table");
	lua_pushstring(L, "datas");
	lua_gettable(L, -2);
	CCASSERT(lua_istable(L, -1) == 1, "is not table");
	lua_pushnil(L);
	while(lua_next(L, 2))
	{
		CCASSERT(lua_istable(L, -1) == 1, "is not table");
		DragonTiger_WeightCFGData data;
		LuaCfgHelper::readInt(L, "Index", data.mIndex);
		LuaCfgHelper::readFloat(L, "Water", data.mWater);
		LuaCfgHelper::readInt(L, "KillScoreRate", data.mKillScoreRate);
		LuaCfgHelper::readInt(L, "TriggerControl", data.mTriggerControl);
		if (mMapData.find(data.mIndex) != mMapData.end())std::cout <<"data refind:" << data.mIndex << std::endl;
		CCASSERT(mMapData.find(data.mIndex) == mMapData.end(), "data.mIndex is exists");
		mMapData.insert(std::make_pair(data.mIndex, data));
		lua_pop(L, 1);
	}
	lua_settop(L, 0);
	CCLOG("DragonTiger_WeightCFG Loaded. Load Data:%u", mMapData.size());
}

void DragonTiger_WeightCFG::Reload()
{
	mMapData.clear();
	Load();
}

DragonTiger_WeightCFG* DragonTiger_WeightCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new DragonTiger_WeightCFG());
	}
	return msSingleton.get();
}
