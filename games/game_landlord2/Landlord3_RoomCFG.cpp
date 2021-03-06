#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "Landlord3_RoomCFG.h"
std::auto_ptr<Landlord3_RoomCFG> Landlord3_RoomCFG::msSingleton(nullptr);

int Landlord3_RoomCFG::GetCount()
{
	return (int)mMapData.size();
}

const Landlord3_RoomCFGData* Landlord3_RoomCFG::GetData(int RoomID)
{
	auto it = mMapData.find(RoomID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, Landlord3_RoomCFGData>& Landlord3_RoomCFG::GetMapData()
{
	return mMapData;
}

void Landlord3_RoomCFG::Reload()
{
	mMapData.clear();
	Load();
}

void Landlord3_RoomCFG::Reload(const std::string& path)
{
	std::ifstream readStream(path, std::ios::binary);
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
		Landlord3_RoomCFGData data;
		data.mRoomID = element->IntAttribute("RoomID");
		data.mRoomName = element->Attribute("RoomName");
		data.mRoomImage = element->Attribute("RoomImage");
		data.mKickGoldCondition = element->IntAttribute("KickGoldCondition");
		data.mGoldMinCondition = element->IntAttribute("GoldMinCondition");
		data.mGoldMaxCondition = element->IntAttribute("GoldMaxCondition");
		data.mVipCondition = element->IntAttribute("VipCondition");
		data.mTicketCondition = element->IntAttribute("TicketCondition");
		data.mBaseCondition = element->IntAttribute("BaseCondition");
		data.mTableCount = element->IntAttribute("TableCount");
		data.mPlayerMaxCounter = element->IntAttribute("PlayerMaxCounter");
		data.mIsOpen = element->BoolAttribute("IsOpen");
		mMapData[data.mRoomID] = data;
		element = element->NextSiblingElement();
	}
}

void Landlord3_RoomCFG::Load(const std::string& path)
{
	std::ifstream readStream(path, std::ios::binary);
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
		Landlord3_RoomCFGData data;
		data.mRoomID = element->IntAttribute("RoomID");
		data.mRoomName = element->Attribute("RoomName");
		data.mRoomImage = element->Attribute("RoomImage");
		data.mKickGoldCondition = element->IntAttribute("KickGoldCondition");
		data.mGoldMinCondition = element->IntAttribute("GoldMinCondition");
		data.mGoldMaxCondition = element->IntAttribute("GoldMaxCondition");
		data.mVipCondition = element->IntAttribute("VipCondition");
		data.mTicketCondition = element->IntAttribute("TicketCondition");
		data.mBaseCondition = element->IntAttribute("BaseCondition");
		data.mTableCount = element->IntAttribute("TableCount");
		data.mPlayerMaxCounter = element->IntAttribute("PlayerMaxCounter");
		data.mIsOpen = element->BoolAttribute("IsOpen");
		if (mMapData.find(data.mRoomID) != mMapData.end())std::cout <<"data refind:" << data.mRoomID << std::endl;
		assert(mMapData.find(data.mRoomID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mRoomID, data));
		element = element->NextSiblingElement();
	}
}

void Landlord3_RoomCFG::Load()
{
	Load("../Config/Landlord3_RoomCFG.xml");
}

Landlord3_RoomCFG* Landlord3_RoomCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new Landlord3_RoomCFG());
	}
	return msSingleton.get();
}
