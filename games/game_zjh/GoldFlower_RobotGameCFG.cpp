#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "GoldFlower_RobotGameCFG.h"
std::auto_ptr<GoldFlower_RobotGameCFG> GoldFlower_RobotGameCFG::msSingleton(nullptr);

int GoldFlower_RobotGameCFG::GetCount()
{
	return (int)mMapData.size();
}

const GoldFlower_RobotGameCFGData* GoldFlower_RobotGameCFG::GetData(int RoomID)
{
	auto it = mMapData.find(RoomID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, GoldFlower_RobotGameCFGData>& GoldFlower_RobotGameCFG::GetMapData()
{
	return mMapData;
}

void GoldFlower_RobotGameCFG::Reload()
{
	mMapData.clear();
	Load();
}

void GoldFlower_RobotGameCFG::Load()
{
	std::ifstream readStream("../Config/GoldFlower_RobotGameCFG.xml", std::ios::binary);
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
		GoldFlower_RobotGameCFGData data;
		data.mRoomID = element->IntAttribute("RoomID");
		data.mReadyTimeMin = element->FloatAttribute("ReadyTimeMin");
		data.mReadyTimeMax = element->FloatAttribute("ReadyTimeMax");
		data.mOperaTimeMin = element->FloatAttribute("OperaTimeMin");
		data.mOperaTimeMax = element->FloatAttribute("OperaTimeMax");
		data.mRobotResultTime = element->FloatAttribute("RobotResultTime");
		data.mParam1 = element->FloatAttribute("Param1");
		data.mParam2 = element->FloatAttribute("Param2");
		data.mParam3 = element->FloatAttribute("Param3");
		data.mParam4 = element->FloatAttribute("Param4");
		if (mMapData.find(data.mRoomID) != mMapData.end())std::cout <<"data refind:" << data.mRoomID << std::endl;
		assert(mMapData.find(data.mRoomID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mRoomID, data));
		element = element->NextSiblingElement();
	}
}

GoldFlower_RobotGameCFG* GoldFlower_RobotGameCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new GoldFlower_RobotGameCFG());
	}
	return msSingleton.get();
}
