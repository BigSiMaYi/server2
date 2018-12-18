#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "BMW_RoomCFG.h"
std::auto_ptr<BMW_RoomCFG> BMW_RoomCFG::msSingleton(nullptr);

int BMW_RoomCFG::GetCount()
{
	return (int)mMapData.size();
}

const BMW_RoomCFGData* BMW_RoomCFG::GetData(int RoomID)
{
	auto it = mMapData.find(RoomID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, BMW_RoomCFGData>& BMW_RoomCFG::GetMapData()
{
	return mMapData;
}

void BMW_RoomCFG::Reload()
{
	mMapData.clear();
	Load();
}

void BMW_RoomCFG::Load()
{
	std::ifstream readStream("../Config/BMW_RoomCFG.xml", std::ios::binary);
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
		BMW_RoomCFGData data;
		data.mRoomID = element->IntAttribute("RoomID");
		data.mRoomName = element->Attribute("RoomName");
		data.mRoomImage = element->Attribute("RoomImage");
		data.mGoldMinCondition = element->IntAttribute("GoldMinCondition");
		data.mGoldMaxCondition = element->IntAttribute("GoldMaxCondition");
		data.mVipCondition = element->IntAttribute("VipCondition");
		data.mTicketCondition = element->IntAttribute("TicketCondition");
		data.mBetCondition = element->IntAttribute("BetCondition");
		data.mBankerGold = element->IntAttribute("BankerGold");
		data.mBankerGoldExtra = element->IntAttribute("BankerGoldExtra");
		data.mBankerRound = element->IntAttribute("BankerRound");
		data.mBankerRoundExtra = element->IntAttribute("BankerRoundExtra");
		data.mPlayerLimit = element->IntAttribute("PlayerLimit");
		data.mAreaLimit = element->IntAttribute("AreaLimit");
		{
			const char* PlaceJetton = element->Attribute("PlaceJetton");
			std::vector<std::string> vecPlaceJetton;
			boost::split(vecPlaceJetton, PlaceJetton, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecPlaceJetton.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecPlaceJetton[i].c_str(), &temp))
				{
					data.mPlaceJetton.push_back(temp);
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
		data.mRecordHistory = element->IntAttribute("RecordHistory");
		data.mIsSysBanker = element->BoolAttribute("IsSysBanker");
		data.mMaxPlayers = element->IntAttribute("MaxPlayers");
		data.mTableCount = element->IntAttribute("TableCount");
		data.mIsOpen = element->BoolAttribute("IsOpen");
		if (mMapData.find(data.mRoomID) != mMapData.end())std::cout <<"data refind:" << data.mRoomID << std::endl;
		assert(mMapData.find(data.mRoomID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mRoomID, data));
		element = element->NextSiblingElement();
	}
}

BMW_RoomCFG* BMW_RoomCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new BMW_RoomCFG());
	}
	return msSingleton.get();
}
