#include "stdafx.h"
#include "tinyxml2/tinyxml2.h"
#include "LuaCfgHelper.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "DragonTiger_BaseInfo.h"
std::auto_ptr<DragonTiger_BaseInfo> DragonTiger_BaseInfo::msSingleton(nullptr);

int DragonTiger_BaseInfo::GetCount()
{
	return (int)mMapData.size();
}

const DragonTiger_BaseInfoData* DragonTiger_BaseInfo::GetData(std::string Key)
{
	auto it = mMapData.find(Key);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

const std::map<std::string, DragonTiger_BaseInfoData>& DragonTiger_BaseInfo::GetMapData()
{
	return mMapData;
}

void DragonTiger_BaseInfo::Load()
{
	tinyxml2::XMLDocument xmlDoc;
	std::string content = FileUtils::getInstance()->getStringFromFile("Config/DragonTiger_BaseInfo.xml");
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
		DragonTiger_BaseInfoData data;
		data.mKey = element->Attribute("Key");
		data.mValue = element->IntAttribute("Value");
		if (mMapData.find(data.mKey) != mMapData.end())std::cout <<"data refind:" << data.mKey << std::endl;
		CCASSERT(mMapData.find(data.mKey) == mMapData.end(), "data.mKey is exists");
		mMapData.insert(std::make_pair(data.mKey, data));
		element = element->NextSiblingElement();
	}
	CCLOG("DragonTiger_BaseInfo Loaded. Load Data:%u", mMapData.size());
}

void DragonTiger_BaseInfo::LoadLua()
{
	LuaEngine::getInstance()->executeScriptFile("Config/DragonTiger_BaseInfo");
	lua_State* L = LuaEngine::getInstance()->getLuaStack()->getLuaState();
	lua_getglobal(L, "DragonTiger_BaseInfo");
	CCASSERT(lua_istable(L, -1) == 1, "is not table");
	lua_pushstring(L, "datas");
	lua_gettable(L, -2);
	CCASSERT(lua_istable(L, -1) == 1, "is not table");
	lua_pushnil(L);
	while(lua_next(L, 2))
	{
		CCASSERT(lua_istable(L, -1) == 1, "is not table");
		DragonTiger_BaseInfoData data;
		LuaCfgHelper::readString(L, "Key", data.mKey);
		LuaCfgHelper::readInt(L, "Value", data.mValue);
		if (mMapData.find(data.mKey) != mMapData.end())std::cout <<"data refind:" << data.mKey << std::endl;
		CCASSERT(mMapData.find(data.mKey) == mMapData.end(), "data.mKey is exists");
		mMapData.insert(std::make_pair(data.mKey, data));
		lua_pop(L, 1);
	}
	lua_settop(L, 0);
	CCLOG("DragonTiger_BaseInfo Loaded. Load Data:%u", mMapData.size());
}

void DragonTiger_BaseInfo::Reload()
{
	mMapData.clear();
	Load();
}

DragonTiger_BaseInfo* DragonTiger_BaseInfo::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new DragonTiger_BaseInfo());
	}
	return msSingleton.get();
}
