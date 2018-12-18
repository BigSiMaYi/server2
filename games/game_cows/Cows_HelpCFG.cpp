#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "Cows_HelpCFG.h"
std::auto_ptr<Cows_HelpCFG> Cows_HelpCFG::msSingleton(nullptr);

int Cows_HelpCFG::GetCount()
{
	return (int)mMapData.size();
}

const Cows_HelpCFGData* Cows_HelpCFG::GetData(int HelpID)
{
	auto it = mMapData.find(HelpID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, Cows_HelpCFGData>& Cows_HelpCFG::GetMapData()
{
	return mMapData;
}

void Cows_HelpCFG::Reload()
{
	mMapData.clear();
	Load();
}

void Cows_HelpCFG::Load()
{
	std::ifstream readStream("../Config/Cows_HelpCFG.xml", std::ios::binary);
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
		Cows_HelpCFGData data;
		data.mHelpID = element->IntAttribute("HelpID");
		data.mCardsName = element->Attribute("CardsName");
		data.mCardsInfo = element->Attribute("CardsInfo");
		data.mCardsTypeStr = element->Attribute("CardsTypeStr");
		{
			const char* Pokers = element->Attribute("Pokers");
			std::vector<std::string> vecPokers;
			boost::split(vecPokers, Pokers, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecPokers.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecPokers[i].c_str(), &temp))
				{
					data.mPokers.push_back(temp);
				}
			}
		}
		if (mMapData.find(data.mHelpID) != mMapData.end())std::cout <<"data refind:" << data.mHelpID << std::endl;
		assert(mMapData.find(data.mHelpID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mHelpID, data));
		element = element->NextSiblingElement();
	}
}

Cows_HelpCFG* Cows_HelpCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new Cows_HelpCFG());
	}
	return msSingleton.get();
}
