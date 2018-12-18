#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "Cows_CardsCFG.h"
std::auto_ptr<Cows_CardsCFG> Cows_CardsCFG::msSingleton(nullptr);

int Cows_CardsCFG::GetCount()
{
	return (int)mMapData.size();
}

const Cows_CardsCFGData* Cows_CardsCFG::GetData(int CardsID)
{
	auto it = mMapData.find(CardsID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, Cows_CardsCFGData>& Cows_CardsCFG::GetMapData()
{
	return mMapData;
}

void Cows_CardsCFG::Reload()
{
	mMapData.clear();
	Load();
}

void Cows_CardsCFG::Load()
{
	std::ifstream readStream("../Config/Cows_CardsCFG.xml", std::ios::binary);
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
		Cows_CardsCFGData data;
		data.mCardsID = element->IntAttribute("CardsID");
		data.mCardsName = element->Attribute("CardsName");
		data.mCardsRate = element->IntAttribute("CardsRate");
		data.mCardsTypeStr = element->Attribute("CardsTypeStr");
		data.mSoundID = element->IntAttribute("SoundID");
		data.mCardsAnimation = element->Attribute("CardsAnimation");
		if (mMapData.find(data.mCardsID) != mMapData.end())std::cout <<"data refind:" << data.mCardsID << std::endl;
		assert(mMapData.find(data.mCardsID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mCardsID, data));
		element = element->NextSiblingElement();
	}
}

Cows_CardsCFG* Cows_CardsCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new Cows_CardsCFG());
	}
	return msSingleton.get();
}
