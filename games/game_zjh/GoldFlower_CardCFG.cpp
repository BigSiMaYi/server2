#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "GoldFlower_CardCFG.h"
std::auto_ptr<GoldFlower_CardCFG> GoldFlower_CardCFG::msSingleton(nullptr);

int GoldFlower_CardCFG::GetCount()
{
	return (int)mMapData.size();
}

const GoldFlower_CardCFGData* GoldFlower_CardCFG::GetData(int RoomID)
{
	auto it = mMapData.find(RoomID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, GoldFlower_CardCFGData>& GoldFlower_CardCFG::GetMapData()
{
	return mMapData;
}

void GoldFlower_CardCFG::Reload()
{
	mMapData.clear();
	Load();
}

void GoldFlower_CardCFG::Load()
{
	std::ifstream readStream("../Config/GoldFlower_CardCFG.xml", std::ios::binary);
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
		GoldFlower_CardCFGData data;
		data.mRoomID = element->IntAttribute("RoomID");
		{
			const char* CardModeRate = element->Attribute("CardModeRate");
			std::vector<std::string> vecCardModeRate;
			boost::split(vecCardModeRate, CardModeRate, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecCardModeRate.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecCardModeRate[i].c_str(), &temp))
				{
					data.mCardModeRate.push_back(temp);
				}
			}
		}
		if (mMapData.find(data.mRoomID) != mMapData.end())std::cout <<"data refind:" << data.mRoomID << std::endl;
		assert(mMapData.find(data.mRoomID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mRoomID, data));
		element = element->NextSiblingElement();
	}
}

GoldFlower_CardCFG* GoldFlower_CardCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new GoldFlower_CardCFG());
	}
	return msSingleton.get();
}
