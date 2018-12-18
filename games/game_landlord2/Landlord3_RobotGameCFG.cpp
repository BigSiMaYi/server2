#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "Landlord3_RobotGameCFG.h"
std::auto_ptr<Landlord3_RobotGameCFG> Landlord3_RobotGameCFG::msSingleton(nullptr);

int Landlord3_RobotGameCFG::GetCount()
{
	return (int)mMapData.size();
}

const Landlord3_RobotGameCFGData* Landlord3_RobotGameCFG::GetData(int RoomID)
{
	auto it = mMapData.find(RoomID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, Landlord3_RobotGameCFGData>& Landlord3_RobotGameCFG::GetMapData()
{
	return mMapData;
}

void Landlord3_RobotGameCFG::Reload()
{
	mMapData.clear();
	Load();
}

void Landlord3_RobotGameCFG::Load()
{
	std::ifstream readStream("../Config/Landlord3_RobotGameCFG.xml", std::ios::binary);
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
		Landlord3_RobotGameCFGData data;
		data.mRoomID = element->IntAttribute("RoomID");
		data.mRobotOperaMinTime = element->IntAttribute("RobotOperaMinTime");
		data.mRobotOperaMaxTime = element->IntAttribute("RobotOperaMaxTime");
		data.mRobotReadyMinTime = element->IntAttribute("RobotReadyMinTime");
		data.mRobotReadyMaxTime = element->IntAttribute("RobotReadyMaxTime");
		data.mRobotResultTime = element->IntAttribute("RobotResultTime");
		if (mMapData.find(data.mRoomID) != mMapData.end())std::cout <<"data refind:" << data.mRoomID << std::endl;
		assert(mMapData.find(data.mRoomID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mRoomID, data));
		element = element->NextSiblingElement();
	}
}

Landlord3_RobotGameCFG* Landlord3_RobotGameCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new Landlord3_RobotGameCFG());
	}
	return msSingleton.get();
}
