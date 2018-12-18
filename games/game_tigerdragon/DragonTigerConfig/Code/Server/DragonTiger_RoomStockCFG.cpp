#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
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

boost::unordered_map<int, DragonTiger_RoomStockCFGData>& DragonTiger_RoomStockCFG::GetMapData()
{
	return mMapData;
}

void DragonTiger_RoomStockCFG::Reload()
{
	mMapData.clear();
	Load();
}

void DragonTiger_RoomStockCFG::Load()
{
	std::ifstream readStream("../Config/DragonTiger_RoomStockCFG.xml", std::ios::binary);
	if (!readStream.is_open())
	{
		assert(false);
		return;
	}
	readStream.seekg(0, std::ios::end);
	int fileSize = readStream.tellg();
	boost::shared_array<char> buffer(new char[fileSize+1]);
	buffer.get()[fileSize] = '\0';
	readStream.seekg(0, std::ios::beg);
	readStream.read(buffer.get(), fileSize);
	readStream.close();
	tinyxml2::XMLDocument xmlDoc;
	auto result = xmlDoc.Parse(buffer.get(), fileSize);
	if (result != tinyxml2::XML_SUCCESS)
	{
		assert(false);
		return;
	}
	auto root = xmlDoc.RootElement();
	if (root == NULL)
	{
		assert(false);
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
		assert(mMapData.find(data.mRoomID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mRoomID, data));
		element = element->NextSiblingElement();
	}
}

DragonTiger_RoomStockCFG* DragonTiger_RoomStockCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new DragonTiger_RoomStockCFG());
	}
	return msSingleton.get();
}
