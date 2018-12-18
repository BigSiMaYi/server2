#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "GoldFlower_RoomCFG.h"
std::auto_ptr<GoldFlower_RoomCFG> GoldFlower_RoomCFG::msSingleton(nullptr);

int GoldFlower_RoomCFG::GetCount()
{
	return (int)mMapData.size();
}

const GoldFlower_RoomCFGData* GoldFlower_RoomCFG::GetData(int RoomID)
{
	auto it = mMapData.find(RoomID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, GoldFlower_RoomCFGData>& GoldFlower_RoomCFG::GetMapData()
{
	return mMapData;
}

void GoldFlower_RoomCFG::Reload()
{
	mMapData.clear();
	Load();
}

void GoldFlower_RoomCFG::Load()
{
	std::ifstream readStream("../Config/GoldFlower_RoomCFG.xml", std::ios::binary);
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
		GoldFlower_RoomCFGData data;
		data.mRoomID = element->IntAttribute("RoomID");
		data.mRoomName = element->Attribute("RoomName");
		data.mRoomImage = element->Attribute("RoomImage");
		data.mGoldMinCondition = element->IntAttribute("GoldMinCondition");
		data.mGoldMaxCondition = element->IntAttribute("GoldMaxCondition");
		data.mVipCondition = element->IntAttribute("VipCondition");
		data.mTicketCondition = element->IntAttribute("TicketCondition");
		data.mBaseCondition = element->IntAttribute("BaseCondition");
		{
			const char* AddChipList = element->Attribute("AddChipList");
			std::vector<std::string> vecAddChipList;
			boost::split(vecAddChipList, AddChipList, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecAddChipList.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecAddChipList[i].c_str(), &temp))
				{
					data.mAddChipList.push_back(temp);
				}
			}
		}
		{
			const char* AddChipImages = element->Attribute("AddChipImages");
			std::vector<std::string> vecAddChipImages;
			boost::split(vecAddChipImages, AddChipImages, boost::is_any_of(","));
			for (unsigned int i = 0; i < vecAddChipImages.size(); i++)
			{
				data.mAddChipImages.push_back(vecAddChipImages[i]);
			}
		}
		data.mLookCondition = element->IntAttribute("LookCondition");
		data.mPkCondition = element->IntAttribute("PkCondition");
		data.mMaxChip = element->IntAttribute("MaxChip");
		data.mMaxRound = element->IntAttribute("MaxRound");
		data.mMaxPool = element->IntAttribute("MaxPool");
		data.mTableCount = element->IntAttribute("TableCount");
		data.mTableManCount = element->IntAttribute("TableManCount");
		data.mTableRobotCount = element->IntAttribute("TableRobotCount");
		data.mIsOpen = element->BoolAttribute("IsOpen");
		if (mMapData.find(data.mRoomID) != mMapData.end())std::cout <<"data refind:" << data.mRoomID << std::endl;
		assert(mMapData.find(data.mRoomID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mRoomID, data));
		element = element->NextSiblingElement();
	}
}

GoldFlower_RoomCFG* GoldFlower_RoomCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new GoldFlower_RoomCFG());
	}
	return msSingleton.get();
}
