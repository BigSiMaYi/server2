#include "stdafx.h"
#include "tinyxml2/tinyxml2.h"
#include "LuaCfgHelper.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "DragonTiger_MultiLanguageCFG.h"
std::auto_ptr<DragonTiger_MultiLanguageCFG> DragonTiger_MultiLanguageCFG::msSingleton(nullptr);

int DragonTiger_MultiLanguageCFG::GetCount()
{
	return (int)mMapData.size();
}

const DragonTiger_MultiLanguageCFGData* DragonTiger_MultiLanguageCFG::GetData(std::string ID)
{
	auto it = mMapData.find(ID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

const std::map<std::string, DragonTiger_MultiLanguageCFGData>& DragonTiger_MultiLanguageCFG::GetMapData()
{
	return mMapData;
}

void DragonTiger_MultiLanguageCFG::Load()
{
	tinyxml2::XMLDocument xmlDoc;
	std::string content = FileUtils::getInstance()->getStringFromFile("Config/DragonTiger_MultiLanguageCFG.xml");
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
		DragonTiger_MultiLanguageCFGData data;
		data.mID = element->Attribute("ID");
		data.mName = element->Attribute("Name");
		if (mMapData.find(data.mID) != mMapData.end())std::cout <<"data refind:" << data.mID << std::endl;
		CCASSERT(mMapData.find(data.mID) == mMapData.end(), "data.mID is exists");
		mMapData.insert(std::make_pair(data.mID, data));
		element = element->NextSiblingElement();
	}
	CCLOG("DragonTiger_MultiLanguageCFG Loaded. Load Data:%u", mMapData.size());
}

void DragonTiger_MultiLanguageCFG::LoadLua()
{
	LuaEngine::getInstance()->executeScriptFile("Config/DragonTiger_MultiLanguageCFG");
	lua_State* L = LuaEngine::getInstance()->getLuaStack()->getLuaState();
	lua_getglobal(L, "DragonTiger_MultiLanguageCFG");
	CCASSERT(lua_istable(L, -1) == 1, "is not table");
	lua_pushstring(L, "datas");
	lua_gettable(L, -2);
	CCASSERT(lua_istable(L, -1) == 1, "is not table");
	lua_pushnil(L);
	while(lua_next(L, 2))
	{
		CCASSERT(lua_istable(L, -1) == 1, "is not table");
		DragonTiger_MultiLanguageCFGData data;
		LuaCfgHelper::readString(L, "ID", data.mID);
		LuaCfgHelper::readString(L, "Name", data.mName);
		if (mMapData.find(data.mID) != mMapData.end())std::cout <<"data refind:" << data.mID << std::endl;
		CCASSERT(mMapData.find(data.mID) == mMapData.end(), "data.mID is exists");
		mMapData.insert(std::make_pair(data.mID, data));
		lua_pop(L, 1);
	}
	lua_settop(L, 0);
	CCLOG("DragonTiger_MultiLanguageCFG Loaded. Load Data:%u", mMapData.size());
}

void DragonTiger_MultiLanguageCFG::Reload()
{
	mMapData.clear();
	Load();
}

DragonTiger_MultiLanguageCFG* DragonTiger_MultiLanguageCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new DragonTiger_MultiLanguageCFG());
	}
	return msSingleton.get();
}
