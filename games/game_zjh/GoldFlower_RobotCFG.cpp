#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "GoldFlower_RobotCFG.h"
std::auto_ptr<GoldFlower_RobotCFG> GoldFlower_RobotCFG::msSingleton(nullptr);

int GoldFlower_RobotCFG::GetCount()
{
	return (int)mMapData.size();
}

const GoldFlower_RobotCFGData* GoldFlower_RobotCFG::GetData(int RoomID)
{
	auto it = mMapData.find(RoomID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, GoldFlower_RobotCFGData>& GoldFlower_RobotCFG::GetMapData()
{
	return mMapData;
}

void GoldFlower_RobotCFG::Reload()
{
	mMapData.clear();
	Load();
}

void GoldFlower_RobotCFG::Load()
{
	std::ifstream readStream("../Config/GoldFlower_RobotCFG.xml", std::ios::binary);
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
		GoldFlower_RobotCFGData data;
		data.mRoomID = element->IntAttribute("RoomID");
		data.mRobotMaxGold = element->IntAttribute("RobotMaxGold");
		data.mRobotMinTake = element->IntAttribute("RobotMinTake");
		data.mRobotMaxTake = element->IntAttribute("RobotMaxTake");
		data.mRobotMinCount = element->IntAttribute("RobotMinCount");
		data.mRobotMaxCount = element->IntAttribute("RobotMaxCount");
		data.mRobotMinVip = element->IntAttribute("RobotMinVip");
		data.mRobotMaxVip = element->IntAttribute("RobotMaxVip");
		data.mRobotMinRound = element->IntAttribute("RobotMinRound");
		data.mRobotMaxRound = element->IntAttribute("RobotMaxRound");
		data.mRobotMinEntry = element->IntAttribute("RobotMinEntry");
		data.mRobotMaxEntry = element->IntAttribute("RobotMaxEntry");
		data.mRobotCoeffi = element->FloatAttribute("RobotCoeffi");
		data.mIsOpen = element->IntAttribute("IsOpen");
		if (mMapData.find(data.mRoomID) != mMapData.end())std::cout <<"data refind:" << data.mRoomID << std::endl;
		assert(mMapData.find(data.mRoomID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mRoomID, data));
		element = element->NextSiblingElement();
	}
}

GoldFlower_RobotCFG* GoldFlower_RobotCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new GoldFlower_RobotCFG());
	}
	return msSingleton.get();
}
