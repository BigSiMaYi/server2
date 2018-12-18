#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "M_DialLotteryTypeCFG.h"
std::auto_ptr<M_DialLotteryTypeCFG> M_DialLotteryTypeCFG::msSingleton(nullptr);

int M_DialLotteryTypeCFG::GetCount()
{
	return (int)mMapData.size();
}

const M_DialLotteryTypeCFGData* M_DialLotteryTypeCFG::GetData(int ID)
{
	auto it = mMapData.find(ID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, M_DialLotteryTypeCFGData>& M_DialLotteryTypeCFG::GetMapData()
{
	return mMapData;
}

void M_DialLotteryTypeCFG::Reload()
{
	mMapData.clear();
	Load();
}

void M_DialLotteryTypeCFG::Reload(const std::string& path)
{
	std::ifstream readStream(path, std::ios::binary);
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
		M_DialLotteryTypeCFGData data;
		data.mID = element->IntAttribute("ID");
		data.mType = element->IntAttribute("Type");
		data.mCount = element->IntAttribute("Count");
		data.mProbCount = element->IntAttribute("ProbCount");
		mMapData[data.mID] = data;
		element = element->NextSiblingElement();
	}
}

void M_DialLotteryTypeCFG::Load(const std::string& path)
{
	std::ifstream readStream(path, std::ios::binary);
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
		M_DialLotteryTypeCFGData data;
		data.mID = element->IntAttribute("ID");
		data.mType = element->IntAttribute("Type");
		data.mCount = element->IntAttribute("Count");
		data.mProbCount = element->IntAttribute("ProbCount");
		if (mMapData.find(data.mID) != mMapData.end())std::cout <<"data refind:" << data.mID << std::endl;
		assert(mMapData.find(data.mID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mID, data));
		element = element->NextSiblingElement();
	}
}

void M_DialLotteryTypeCFG::Load()
{
	Load("../Config/M_DialLotteryTypeCFG.xml");
}

M_DialLotteryTypeCFG* M_DialLotteryTypeCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new M_DialLotteryTypeCFG());
	}
	return msSingleton.get();
}
