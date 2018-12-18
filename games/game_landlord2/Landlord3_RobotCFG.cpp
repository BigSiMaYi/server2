#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "Landlord3_RobotCFG.h"
std::auto_ptr<Landlord3_RobotCFG> Landlord3_RobotCFG::msSingleton(nullptr);

int Landlord3_RobotCFG::GetCount()
{
	return (int)mMapData.size();
}

const Landlord3_RobotCFGData* Landlord3_RobotCFG::GetData(int RoomID)
{
	auto it = mMapData.find(RoomID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, Landlord3_RobotCFGData>& Landlord3_RobotCFG::GetMapData()
{
	return mMapData;
}

void Landlord3_RobotCFG::Reload()
{
	mMapData.clear();
	Load();
}

void Landlord3_RobotCFG::Load()
{
	std::ifstream readStream("../Config/Landlord3_RobotCFG.xml", std::ios::binary);
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
		Landlord3_RobotCFGData data;
		data.mRoomID = element->IntAttribute("RoomID");
		data.mRobotMaxGold = element->IntAttribute("RobotMaxGold");
		data.mRobotMinTake = element->IntAttribute("RobotMinTake");
		data.mRobotMaxTake = element->IntAttribute("RobotMaxTake");
		data.mRoomRobotMinCount = element->IntAttribute("RoomRobotMinCount");
		data.mRoomRobotMaxCount = element->IntAttribute("RoomRobotMaxCount");
		data.mRobotMinCount = element->IntAttribute("RobotMinCount");
		data.mRobotMaxCount = element->IntAttribute("RobotMaxCount");
		data.mRobotMinVip = element->IntAttribute("RobotMinVip");
		data.mRobotMaxVip = element->IntAttribute("RobotMaxVip");
		data.mRobotMinRound = element->IntAttribute("RobotMinRound");
		data.mRobotMaxRound = element->IntAttribute("RobotMaxRound");
		data.mRobotMinEntry = element->IntAttribute("RobotMinEntry");
		data.mRobotMaxEntry = element->IntAttribute("RobotMaxEntry");
		data.mIsOpen = element->IntAttribute("IsOpen");
		if (mMapData.find(data.mRoomID) != mMapData.end())std::cout <<"data refind:" << data.mRoomID << std::endl;
		assert(mMapData.find(data.mRoomID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mRoomID, data));
		element = element->NextSiblingElement();
	}
}

Landlord3_RobotCFG* Landlord3_RobotCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new Landlord3_RobotCFG());
	}
	return msSingleton.get();
}
