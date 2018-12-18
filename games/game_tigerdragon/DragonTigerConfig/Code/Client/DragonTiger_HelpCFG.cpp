#include "stdafx.h"
#include "tinyxml2/tinyxml2.h"
#include "LuaCfgHelper.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "DragonTiger_HelpCFG.h"
std::auto_ptr<DragonTiger_HelpCFG> DragonTiger_HelpCFG::msSingleton(nullptr);

int DragonTiger_HelpCFG::GetCount()
{
	return (int)mMapData.size();
}

const DragonTiger_HelpCFGData* DragonTiger_HelpCFG::GetData(int HelpID)
{
	auto it = mMapData.find(HelpID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

const std::map<int, DragonTiger_HelpCFGData>& DragonTiger_HelpCFG::GetMapData()
{
	return mMapData;
}

void DragonTiger_HelpCFG::Load()
{
	tinyxml2::XMLDocument xmlDoc;
	std::string content = FileUtils::getInstance()->getStringFromFile("Config/DragonTiger_HelpCFG.xml");
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
		DragonTiger_HelpCFGData data;
		data.mHelpID = element->IntAttribute("HelpID");
		data.mCardsName = element->Attribute("CardsName");
		data.mCardsInfo = element->Attribute("CardsInfo");
		data.mCardsTypeStr = element->Attribute("CardsTypeStr");
		{
			const char* Pokers = element->Attribute("Pokers");
			std::vector<std::string> vecPokers;
			boost::split(vecPokers, Pokers, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecPokers.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecPokers[i].c_str(), &temp))
				{
					data.mPokers.push_back(temp);
				}
			}
		}
		if (mMapData.find(data.mHelpID) != mMapData.end())std::cout <<"data refind:" << data.mHelpID << std::endl;
		CCASSERT(mMapData.find(data.mHelpID) == mMapData.end(), "data.mHelpID is exists");
		mMapData.insert(std::make_pair(data.mHelpID, data));
		element = element->NextSiblingElement();
	}
	CCLOG("DragonTiger_HelpCFG Loaded. Load Data:%u", mMapData.size());
}

void DragonTiger_HelpCFG::LoadLua()
{
	LuaEngine::getInstance()->executeScriptFile("Config/DragonTiger_HelpCFG");
	lua_State* L = LuaEngine::getInstance()->getLuaStack()->getLuaState();
	lua_getglobal(L, "DragonTiger_HelpCFG");
	CCASSERT(lua_istable(L, -1) == 1, "is not table");
	lua_pushstring(L, "datas");
	lua_gettable(L, -2);
	CCASSERT(lua_istable(L, -1) == 1, "is not table");
	lua_pushnil(L);
	while(lua_next(L, 2))
	{
		CCASSERT(lua_istable(L, -1) == 1, "is not table");
		DragonTiger_HelpCFGData data;
		LuaCfgHelper::readInt(L, "HelpID", data.mHelpID);
		LuaCfgHelper::readString(L, "CardsName", data.mCardsName);
		LuaCfgHelper::readString(L, "CardsInfo", data.mCardsInfo);
		LuaCfgHelper::readString(L, "CardsTypeStr", data.mCardsTypeStr);
		LuaCfgHelper::readVectorInt(L, "Pokers", data.mPokers);
		if (mMapData.find(data.mHelpID) != mMapData.end())std::cout <<"data refind:" << data.mHelpID << std::endl;
		CCASSERT(mMapData.find(data.mHelpID) == mMapData.end(), "data.mHelpID is exists");
		mMapData.insert(std::make_pair(data.mHelpID, data));
		lua_pop(L, 1);
	}
	lua_settop(L, 0);
	CCLOG("DragonTiger_HelpCFG Loaded. Load Data:%u", mMapData.size());
}

void DragonTiger_HelpCFG::Reload()
{
	mMapData.clear();
	Load();
}

DragonTiger_HelpCFG* DragonTiger_HelpCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new DragonTiger_HelpCFG());
	}
	return msSingleton.get();
}
