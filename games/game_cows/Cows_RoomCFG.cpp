#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "Cows_RoomCFG.h"
std::auto_ptr<Cows_RoomCFG> Cows_RoomCFG::msSingleton(nullptr);

int Cows_RoomCFG::GetCount()
{
	return (int)mMapData.size();
}

const Cows_RoomCFGData* Cows_RoomCFG::GetData(int RoomID)
{
	auto it = mMapData.find(RoomID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, Cows_RoomCFGData>& Cows_RoomCFG::GetMapData()
{
	return mMapData;
}

void Cows_RoomCFG::Reload()
{
	mMapData.clear();
	Load();
}

void Cows_RoomCFG::Load()
{
	std::ifstream readStream("../Config/Cows_RoomCFG.xml", std::ios::binary);
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
		Cows_RoomCFGData data;
		data.mRoomID = element->IntAttribute("RoomID");
		data.mRoomName = element->Attribute("RoomName");
		data.mRoomImage = element->Attribute("RoomImage");
		data.mGoldCondition = element->IntAttribute("GoldCondition");
		data.mVipCondition = element->IntAttribute("VipCondition");
		data.mTicketCondition = element->IntAttribute("TicketCondition");
		data.mMaxRate = element->IntAttribute("MaxRate");
		{
			const char* ChipList = element->Attribute("ChipList");
			std::vector<std::string> vecChipList;
			boost::split(vecChipList, ChipList, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecChipList.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecChipList[i].c_str(), &temp))
				{
					data.mChipList.push_back(temp);
				}
			}
		}
		{
			const char* UnlockChipList = element->Attribute("UnlockChipList");
			std::vector<std::string> vecUnlockChipList;
			boost::split(vecUnlockChipList, UnlockChipList, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecUnlockChipList.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecUnlockChipList[i].c_str(), &temp))
				{
					data.mUnlockChipList.push_back(temp);
				}
			}
		}
		{
			const char* ChipImages = element->Attribute("ChipImages");
			std::vector<std::string> vecChipImages;
			boost::split(vecChipImages, ChipImages, boost::is_any_of(","));
			for (unsigned int i = 0; i < vecChipImages.size(); i++)
			{
				data.mChipImages.push_back(vecChipImages[i]);
			}
		}
		data.mIsOpen = element->BoolAttribute("IsOpen");
		data.mBankerGold = element->IntAttribute("BankerGold");
		data.mSnatchGold = element->IntAttribute("SnatchGold");
		data.mForceLeaveGold = element->IntAttribute("ForceLeaveGold");
		data.mMaxPlayerCnt = element->IntAttribute("MaxPlayerCnt");
		data.mExpectEarnRate = element->IntAttribute("ExpectEarnRate");
		data.mMaxEarnRate = element->IntAttribute("MaxEarnRate");
		data.mMinEarnRate = element->IntAttribute("MinEarnRate");
		data.mKillPointsSwtch = element->BoolAttribute("KillPointsSwtch");
		if (mMapData.find(data.mRoomID) != mMapData.end())std::cout <<"data refind:" << data.mRoomID << std::endl;
		assert(mMapData.find(data.mRoomID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mRoomID, data));
		element = element->NextSiblingElement();
	}
}

Cows_RoomCFG* Cows_RoomCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new Cows_RoomCFG());
	}
	return msSingleton.get();
}
