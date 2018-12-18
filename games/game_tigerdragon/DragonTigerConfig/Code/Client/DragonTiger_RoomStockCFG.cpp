#include "stdafx.h"
#include "tinyxml2/tinyxml2.h"
#include "LuaCfgHelper.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "DragonTiger_RoomStockCFG.h"
std::auto_ptr<DragonTiger_RoomStockCFG> DragonTiger_RoomStockCFG::msSingleton(nullptr);

int DragonTiger_RoomStockCFG::GetCount()
{
	return (int)mMapData.size();
}

const DragonTiger_RoomStockCFGData* DragonTiger_RoomStockCFG::GetData(int RoomID)
{
	auto it = mMapData.find(RoomID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

const std::map<int, DragonTiger_RoomStockCFGData>& DragonTiger_RoomStockCFG::GetMapData()
{
	return mMapData;
}

void DragonTiger_RoomStockCFG::Load()
{
	tinyxml2::XMLDocument xmlDoc;
	std::string content = FileUtils::getInstance()->getStringFromFile("Config/DragonTiger_RoomStockCFG.xml");
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
		DragonTiger_RoomStockCFGData data;
		data.mRoomID = element->IntAttribute("RoomID");
		data.mDeduct_1 = element->FloatAttribute("Deduct_1");
		data.mDeduct_2 = element->FloatAttribute("Deduct_2");
		data.mDefaultStock = element->IntAttribute("DefaultStock");
		{
			const char* Stock = element->Attribute("Stock");
			std::vector<std::string> vecStock;
			boost::split(vecStock, Stock, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecStock.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecStock[i].c_str(), &temp))
				{
					data.mStock.push_back(temp);
				}
			}
		}
		{
			const char* ChangeBuff = element->Attribute("ChangeBuff");
			std::vector<std::string> vecChangeBuff;
			boost::split(vecChangeBuff, ChangeBuff, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecChangeBuff.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecChangeBuff[i].c_str(), &temp))
				{
					data.mChangeBuff.push_back(temp);
				}
			}
		}
		data.mIsOpen = element->IntAttribute("IsOpen");
		data.mRobotMinCount = element->IntAttribute("RobotMinCount");
		data.mRobotMaxCount = element->IntAttribute("RobotMaxCount");
		data.mRobotMinVip = element->IntAttribute("RobotMinVip");
		data.mRobotMaxVip = element->IntAttribute("RobotMaxVip");
		data.mRobotMinLifeTime = element->IntAttribute("RobotMinLifeTime");
		data.mRobotMaxLifeTime = element->IntAttribute("RobotMaxLifeTime");
		data.mRobotBankerGold = element->IntAttribute("RobotBankerGold");
		data.mRobotSystemMaxBet = element->IntAttribute("RobotSystemMaxBet");
		data.mRobotSystemMinBet = element->IntAttribute("RobotSystemMinBet");
		data.mRobotPlayerMinBet = element->IntAttribute("RobotPlayerMinBet");
		data.mRobotPlayerMaxBet = element->IntAttribute("RobotPlayerMaxBet");
		data.mRobotRobotMinBet = element->IntAttribute("RobotRobotMinBet");
		data.mRobotRobotMaxBet = element->IntAttribute("RobotRobotMaxBet");
		if (mMapData.find(data.mRoomID) != mMapData.end())std::cout <<"data refind:" << data.mRoomID << std::endl;
		CCASSERT(mMapData.find(data.mRoomID) == mMapData.end(), "data.mRoomID is exists");
		mMapData.insert(std::make_pair(data.mRoomID, data));
		element = element->NextSiblingElement();
	}
	CCLOG("DragonTiger_RoomStockCFG Loaded. Load Data:%u", mMapData.size());
}

void DragonTiger_RoomStockCFG::LoadLua()
{
	LuaEngine::getInstance()->executeScriptFile("Config/DragonTiger_RoomStockCFG");
	lua_State* L = LuaEngine::getInstance()->getLuaStack()->getLuaState();
	lua_getglobal(L, "DragonTiger_RoomStockCFG");
	CCASSERT(lua_istable(L, -1) == 1, "is not table");
	lua_pushstring(L, "datas");
	lua_gettable(L, -2);
	CCASSERT(lua_istable(L, -1) == 1, "is not table");
	lua_pushnil(L);
	while(lua_next(L, 2))
	{
		CCASSERT(lua_istable(L, -1) == 1, "is not table");
		DragonTiger_RoomStockCFGData data;
		LuaCfgHelper::readInt(L, "RoomID", data.mRoomID);
		LuaCfgHelper::readFloat(L, "Deduct_1", data.mDeduct_1);
		LuaCfgHelper::readFloat(L, "Deduct_2", data.mDeduct_2);
		LuaCfgHelper::readInt(L, "DefaultStock", data.mDefaultStock);
		LuaCfgHelper::readVectorInt(L, "Stock", data.mStock);
		LuaCfgHelper::readVectorInt(L, "ChangeBuff", data.mChangeBuff);
		LuaCfgHelper::readInt(L, "IsOpen", data.mIsOpen);
		LuaCfgHelper::readInt(L, "RobotMinCount", data.mRobotMinCount);
		LuaCfgHelper::readInt(L, "RobotMaxCount", data.mRobotMaxCount);
		LuaCfgHelper::readInt(L, "RobotMinVip", data.mRobotMinVip);
		LuaCfgHelper::readInt(L, "RobotMaxVip", data.mRobotMaxVip);
		LuaCfgHelper::readInt(L, "RobotMinLifeTime", data.mRobotMinLifeTime);
		LuaCfgHelper::readInt(L, "RobotMaxLifeTime", data.mRobotMaxLifeTime);
		LuaCfgHelper::readInt(L, "RobotBankerGold", data.mRobotBankerGold);
		LuaCfgHelper::readInt(L, "RobotSystemMaxBet", data.mRobotSystemMaxBet);
		LuaCfgHelper::readInt(L, "RobotSystemMinBet", data.mRobotSystemMinBet);
		LuaCfgHelper::readInt(L, "RobotPlayerMinBet", data.mRobotPlayerMinBet);
		LuaCfgHelper::readInt(L, "RobotPlayerMaxBet", data.mRobotPlayerMaxBet);
		LuaCfgHelper::readInt(L, "RobotRobotMinBet", data.mRobotRobotMinBet);
		LuaCfgHelper::readInt(L, "RobotRobotMaxBet", data.mRobotRobotMaxBet);
		if (mMapData.find(data.mRoomID) != mMapData.end())std::cout <<"data refind:" << data.mRoomID << std::endl;
		CCASSERT(mMapData.find(data.mRoomID) == mMapData.end(), "data.mRoomID is exists");
		mMapData.insert(std::make_pair(data.mRoomID, data));
		lua_pop(L, 1);
	}
	lua_settop(L, 0);
	CCLOG("DragonTiger_RoomStockCFG Loaded. Load Data:%u", mMapData.size());
}

void DragonTiger_RoomStockCFG::Reload()
{
	mMapData.clear();
	Load();
}

DragonTiger_RoomStockCFG* DragonTiger_RoomStockCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new DragonTiger_RoomStockCFG());
	}
	return msSingleton.get();
}
