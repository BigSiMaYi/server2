#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "Landlord3_PlayerStock.h"
std::auto_ptr<Landlord3_PlayerStock> Landlord3_PlayerStock::msSingleton(nullptr);

int Landlord3_PlayerStock::GetCount()
{
	return (int)mMapData.size();
}

const Landlord3_PlayerStockData* Landlord3_PlayerStock::GetData(int ID)
{
	auto it = mMapData.find(ID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, Landlord3_PlayerStockData>& Landlord3_PlayerStock::GetMapData()
{
	return mMapData;
}

void Landlord3_PlayerStock::Reload()
{
	mMapData.clear();
	Load();
}

void Landlord3_PlayerStock::Reload(const std::string& path)
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
		Landlord3_PlayerStockData data;
		data.mID = element->IntAttribute("ID");
		data.mCutRoundFlag = element->BoolAttribute("CutRoundFlag");
		{
			const char* TopUpRang = element->Attribute("TopUpRang");
			std::vector<std::string> vecTopUpRang;
			boost::split(vecTopUpRang, TopUpRang, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecTopUpRang.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecTopUpRang[i].c_str(), &temp))
				{
					data.mTopUpRang.push_back(temp);
				}
			}
		}
		{
			const char* Param = element->Attribute("Param");
			std::vector<std::string> vecParam;
			boost::split(vecParam, Param, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecParam.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecParam[i].c_str(), &temp))
				{
					data.mParam.push_back(temp);
				}
			}
		}
		mMapData[data.mID] = data;
		element = element->NextSiblingElement();
	}
}

void Landlord3_PlayerStock::Load(const std::string& path)
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
		Landlord3_PlayerStockData data;
		data.mID = element->IntAttribute("ID");
		data.mCutRoundFlag = element->BoolAttribute("CutRoundFlag");
		{
			const char* TopUpRang = element->Attribute("TopUpRang");
			std::vector<std::string> vecTopUpRang;
			boost::split(vecTopUpRang, TopUpRang, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecTopUpRang.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecTopUpRang[i].c_str(), &temp))
				{
					data.mTopUpRang.push_back(temp);
				}
			}
		}
		{
			const char* Param = element->Attribute("Param");
			std::vector<std::string> vecParam;
			boost::split(vecParam, Param, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecParam.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecParam[i].c_str(), &temp))
				{
					data.mParam.push_back(temp);
				}
			}
		}
		if (mMapData.find(data.mID) != mMapData.end())std::cout <<"data refind:" << data.mID << std::endl;
		assert(mMapData.find(data.mID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mID, data));
		element = element->NextSiblingElement();
	}
}

void Landlord3_PlayerStock::Load()
{
	Load("../Config/Landlord3_PlayerStock.xml");
}

Landlord3_PlayerStock* Landlord3_PlayerStock::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new Landlord3_PlayerStock());
	}
	return msSingleton.get();
}
