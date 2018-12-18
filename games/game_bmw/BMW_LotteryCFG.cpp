#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "BMW_LotteryCFG.h"
std::auto_ptr<BMW_LotteryCFG> BMW_LotteryCFG::msSingleton(nullptr);

int BMW_LotteryCFG::GetCount()
{
	return (int)mMapData.size();
}

const BMW_LotteryCFGData* BMW_LotteryCFG::GetData(int RoomID)
{
	auto it = mMapData.find(RoomID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, BMW_LotteryCFGData>& BMW_LotteryCFG::GetMapData()
{
	return mMapData;
}

void BMW_LotteryCFG::Reload()
{
	mMapData.clear();
	Load();
}

void BMW_LotteryCFG::Load()
{
	std::ifstream readStream("../Config/BMW_LotteryCFG.xml", std::ios::binary);
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
		BMW_LotteryCFGData data;
		data.mRoomID = element->IntAttribute("RoomID");
		data.mRandType = element->IntAttribute("RandType");
		data.mBigRate = element->IntAttribute("BigRate");
		data.mRefreshMinute = element->IntAttribute("RefreshMinute");
		if (mMapData.find(data.mRoomID) != mMapData.end())std::cout <<"data refind:" << data.mRoomID << std::endl;
		assert(mMapData.find(data.mRoomID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mRoomID, data));
		element = element->NextSiblingElement();
	}
}

BMW_LotteryCFG* BMW_LotteryCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new BMW_LotteryCFG());
	}
	return msSingleton.get();
}
