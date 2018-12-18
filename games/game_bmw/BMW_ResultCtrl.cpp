#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "BMW_ResultCtrl.h"
std::auto_ptr<BMW_ResultCtrl> BMW_ResultCtrl::msSingleton(nullptr);

int BMW_ResultCtrl::GetCount()
{
	return (int)mMapData.size();
}

const BMW_ResultCtrlData* BMW_ResultCtrl::GetData(std::string Key)
{
	auto it = mMapData.find(Key);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<std::string, BMW_ResultCtrlData>& BMW_ResultCtrl::GetMapData()
{
	return mMapData;
}

void BMW_ResultCtrl::Reload()
{
	mMapData.clear();
	Load();
}

void BMW_ResultCtrl::Load()
{
	std::ifstream readStream("../Config/BMW_ResultCtrl.xml", std::ios::binary);
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
		BMW_ResultCtrlData data;
		data.mKey = element->Attribute("Key");
		data.mValue = element->IntAttribute("Value");
		if (mMapData.find(data.mKey) != mMapData.end())std::cout <<"data refind:" << data.mKey << std::endl;
		assert(mMapData.find(data.mKey) == mMapData.end());
		mMapData.insert(std::make_pair(data.mKey, data));
		element = element->NextSiblingElement();
	}
}

BMW_ResultCtrl* BMW_ResultCtrl::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new BMW_ResultCtrl());
	}
	return msSingleton.get();
}
