#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "DragonTiger_WeightCFG.h"
std::auto_ptr<DragonTiger_WeightCFG> DragonTiger_WeightCFG::msSingleton(nullptr);

int DragonTiger_WeightCFG::GetCount()
{
	return (int)mMapData.size();
}

const DragonTiger_WeightCFGData* DragonTiger_WeightCFG::GetData(int Index)
{
	auto it = mMapData.find(Index);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, DragonTiger_WeightCFGData>& DragonTiger_WeightCFG::GetMapData()
{
	return mMapData;
}

void DragonTiger_WeightCFG::Reload()
{
	mMapData.clear();
	Load();
}

void DragonTiger_WeightCFG::Load()
{
	std::ifstream readStream("../Config/DragonTiger_WeightCFG.xml", std::ios::binary);
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
		DragonTiger_WeightCFGData data;
		data.mIndex = element->IntAttribute("Index");
		data.mWater = element->FloatAttribute("Water");
		data.mKillScoreRate = element->IntAttribute("KillScoreRate");
		data.mTriggerControl = element->IntAttribute("TriggerControl");
		if (mMapData.find(data.mIndex) != mMapData.end())std::cout <<"data refind:" << data.mIndex << std::endl;
		assert(mMapData.find(data.mIndex) == mMapData.end());
		mMapData.insert(std::make_pair(data.mIndex, data));
		element = element->NextSiblingElement();
	}
}

DragonTiger_WeightCFG* DragonTiger_WeightCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new DragonTiger_WeightCFG());
	}
	return msSingleton.get();
}
