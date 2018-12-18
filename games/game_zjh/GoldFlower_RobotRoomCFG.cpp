#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "GoldFlower_RobotRoomCFG.h"
std::auto_ptr<GoldFlower_RobotRoomCFG> GoldFlower_RobotRoomCFG::msSingleton(nullptr);

int GoldFlower_RobotRoomCFG::GetCount()
{
	return (int)mMapData.size();
}

const GoldFlower_RobotRoomCFGData* GoldFlower_RobotRoomCFG::GetData(int RoomID)
{
	auto it = mMapData.find(RoomID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, GoldFlower_RobotRoomCFGData>& GoldFlower_RobotRoomCFG::GetMapData()
{
	return mMapData;
}

void GoldFlower_RobotRoomCFG::Reload()
{
	mMapData.clear();
	Load();
}

void GoldFlower_RobotRoomCFG::Load()
{
	std::ifstream readStream("../Config/GoldFlower_RobotRoomCFG.xml", std::ios::binary);
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
		GoldFlower_RobotRoomCFGData data;
		data.mRoomID = element->IntAttribute("RoomID");
		data.mOpenTableMin = element->IntAttribute("OpenTableMin");
		data.mOpenTableMax = element->IntAttribute("OpenTableMax");
		data.mRobotTableMin = element->IntAttribute("RobotTableMin");
		data.mRobotTableMax = element->IntAttribute("RobotTableMax");
		data.mRobotCountMin = element->IntAttribute("RobotCountMin");
		data.mRobotCountMax = element->IntAttribute("RobotCountMax");
		data.mElapseTime = element->IntAttribute("ElapseTime");
		data.mIsOpen = element->BoolAttribute("IsOpen");
		if (mMapData.find(data.mRoomID) != mMapData.end())std::cout <<"data refind:" << data.mRoomID << std::endl;
		assert(mMapData.find(data.mRoomID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mRoomID, data));
		element = element->NextSiblingElement();
	}
}

GoldFlower_RobotRoomCFG* GoldFlower_RobotRoomCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new GoldFlower_RobotRoomCFG());
	}
	return msSingleton.get();
}
