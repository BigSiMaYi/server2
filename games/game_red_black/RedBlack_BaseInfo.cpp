#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "RedBlack_BaseInfo.h"
std::auto_ptr<RedBlack_BaseInfo> RedBlack_BaseInfo::msSingleton(nullptr);

int RedBlack_BaseInfo::GetCount()
{
	return (int)mMapData.size();
}

const RedBlack_BaseInfoData* RedBlack_BaseInfo::GetData(std::string Key)
{
	auto it = mMapData.find(Key);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<std::string, RedBlack_BaseInfoData>& RedBlack_BaseInfo::GetMapData()
{
	return mMapData;
}

void RedBlack_BaseInfo::Reload()
{
	mMapData.clear();
	Load();
}

void RedBlack_BaseInfo::Load()
{
	std::ifstream readStream("../Config/RedBlack_BaseInfo.xml", std::ios::binary);
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
		RedBlack_BaseInfoData data;
		data.mKey = element->Attribute("Key");
		data.mValue = element->IntAttribute("Value");
		if (mMapData.find(data.mKey) != mMapData.end())std::cout <<"data refind:" << data.mKey << std::endl;
		assert(mMapData.find(data.mKey) == mMapData.end());
		mMapData.insert(std::make_pair(data.mKey, data));
		element = element->NextSiblingElement();
	}
}

RedBlack_BaseInfo* RedBlack_BaseInfo::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new RedBlack_BaseInfo());
	}
	return msSingleton.get();
}
