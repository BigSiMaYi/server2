#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "RobCows_RoomCFG.h"
std::auto_ptr<RobCows_RoomCFG> RobCows_RoomCFG::msSingleton(nullptr);

int RobCows_RoomCFG::GetCount()
{
	return (int)mMapData.size();
}

const RobCows_RoomCFGData* RobCows_RoomCFG::GetData(int RoomID)
{
	auto it = mMapData.find(RoomID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, RobCows_RoomCFGData>& RobCows_RoomCFG::GetMapData()
{
	return mMapData;
}

void RobCows_RoomCFG::Reload()
{
	mMapData.clear();
	Load();
}

void RobCows_RoomCFG::Load()
{
	std::ifstream readStream("../Config/RobCows_RoomCFG.xml", std::ios::binary);
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
		RobCows_RoomCFGData data;
		data.mRoomID = element->IntAttribute("RoomID");
		data.mRoomName = element->Attribute("RoomName");
		data.mRoomImage = element->Attribute("RoomImage");
		data.mGoldMinCondition = element->IntAttribute("GoldMinCondition");
		data.mGoldMaxCondition = element->IntAttribute("GoldMaxCondition");
		data.mVipCondition = element->IntAttribute("VipCondition");
		data.mTicketCondition = element->IntAttribute("TicketCondition");
		data.mBaseCondition = element->IntAttribute("BaseCondition");
		{
			const char* RobBankerList = element->Attribute("RobBankerList");
			std::vector<std::string> vecRobBankerList;
			boost::split(vecRobBankerList, RobBankerList, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecRobBankerList.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecRobBankerList[i].c_str(), &temp))
				{
					data.mRobBankerList.push_back(temp);
				}
			}
		}
		{
			const char* BetList = element->Attribute("BetList");
			std::vector<std::string> vecBetList;
			boost::split(vecBetList, BetList, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecBetList.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecBetList[i].c_str(), &temp))
				{
					data.mBetList.push_back(temp);
				}
			}
		}
		data.mRealCount = element->IntAttribute("RealCount");
		data.mWaitCount = element->IntAttribute("WaitCount");
		data.mTableCount = element->IntAttribute("TableCount");
		data.mIsOpen = element->BoolAttribute("IsOpen");
		if (mMapData.find(data.mRoomID) != mMapData.end())std::cout <<"data refind:" << data.mRoomID << std::endl;
		assert(mMapData.find(data.mRoomID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mRoomID, data));
		element = element->NextSiblingElement();
	}
}

RobCows_RoomCFG* RobCows_RoomCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new RobCows_RoomCFG());
	}
	return msSingleton.get();
}
