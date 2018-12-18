#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "DragonTiger_Cardodds.h"
std::auto_ptr<DragonTiger_Cardodds> DragonTiger_Cardodds::msSingleton(nullptr);

int DragonTiger_Cardodds::GetCount()
{
	return (int)mMapData.size();
}

const DragonTiger_CardoddsData* DragonTiger_Cardodds::GetData(std::string Key)
{
	auto it = mMapData.find(Key);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<std::string, DragonTiger_CardoddsData>& DragonTiger_Cardodds::GetMapData()
{
	return mMapData;
}

void DragonTiger_Cardodds::Reload()
{
	mMapData.clear();
	Load();
}

void DragonTiger_Cardodds::Load()
{
	std::ifstream readStream("../Config/DragonTiger_Cardodds.xml", std::ios::binary);
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
		DragonTiger_CardoddsData data;
		data.mKey = element->Attribute("Key");
		data.mValue = element->FloatAttribute("Value");
		if (mMapData.find(data.mKey) != mMapData.end())std::cout <<"data refind:" << data.mKey << std::endl;
		assert(mMapData.find(data.mKey) == mMapData.end());
		mMapData.insert(std::make_pair(data.mKey, data));
		element = element->NextSiblingElement();
	}
}

DragonTiger_Cardodds* DragonTiger_Cardodds::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new DragonTiger_Cardodds());
	}
	return msSingleton.get();
}
