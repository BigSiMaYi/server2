#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "RedBlack_RoomCFG.h"
std::auto_ptr<RedBlack_RoomCFG> RedBlack_RoomCFG::msSingleton(nullptr);

int RedBlack_RoomCFG::GetCount()
{
	return (int)mMapData.size();
}

const RedBlack_RoomCFGData* RedBlack_RoomCFG::GetData(int RoomID)
{
	auto it = mMapData.find(RoomID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, RedBlack_RoomCFGData>& RedBlack_RoomCFG::GetMapData()
{
	return mMapData;
}

void RedBlack_RoomCFG::Reload()
{
	mMapData.clear();
	Load();
}

void RedBlack_RoomCFG::Load()
{
	std::ifstream readStream("../Config/RedBlack_RoomCFG.xml", std::ios::binary);
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
		RedBlack_RoomCFGData data;
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
		{
			const char* BetAreaGoldCond = element->Attribute("BetAreaGoldCond");
			std::vector<std::string> vecBetAreaGoldCond;
			boost::split(vecBetAreaGoldCond, BetAreaGoldCond, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecBetAreaGoldCond.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecBetAreaGoldCond[i].c_str(), &temp))
				{
					data.mBetAreaGoldCond.push_back(temp);
				}
			}
		}
		data.mBetGoldCondition = element->IntAttribute("BetGoldCondition");
		data.mRobotLuckAreaBetCond = element->IntAttribute("RobotLuckAreaBetCond");
		if (mMapData.find(data.mRoomID) != mMapData.end())std::cout <<"data refind:" << data.mRoomID << std::endl;
		assert(mMapData.find(data.mRoomID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mRoomID, data));
		element = element->NextSiblingElement();
	}
}

RedBlack_RoomCFG* RedBlack_RoomCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new RedBlack_RoomCFG());
	}
	return msSingleton.get();
}
