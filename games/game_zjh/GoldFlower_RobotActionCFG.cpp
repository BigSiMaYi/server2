#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "GoldFlower_RobotActionCFG.h"
std::auto_ptr<GoldFlower_RobotActionCFG> GoldFlower_RobotActionCFG::msSingleton(nullptr);

int GoldFlower_RobotActionCFG::GetCount()
{
	return (int)mMapData.size();
}

const GoldFlower_RobotActionCFGData* GoldFlower_RobotActionCFG::GetData(int ModelID)
{
	auto it = mMapData.find(ModelID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, GoldFlower_RobotActionCFGData>& GoldFlower_RobotActionCFG::GetMapData()
{
	return mMapData;
}

void GoldFlower_RobotActionCFG::Reload()
{
	mMapData.clear();
	Load();
}

void GoldFlower_RobotActionCFG::Load()
{
	std::ifstream readStream("../Config/GoldFlower_RobotActionCFG.xml", std::ios::binary);
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
		GoldFlower_RobotActionCFGData data;
		data.mModelID = element->IntAttribute("ModelID");
		data.mModelName = element->Attribute("ModelName");
		{
			const char* ModelData = element->Attribute("ModelData");
			std::vector<std::string> vecModelData;
			boost::split(vecModelData, ModelData, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecModelData.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecModelData[i].c_str(), &temp))
				{
					data.mModelData.push_back(temp);
				}
			}
		}
		data.mAddRateParam = element->FloatAttribute("AddRateParam");
		{
			const char* AddRateList = element->Attribute("AddRateList");
			std::vector<std::string> vecAddRateList;
			boost::split(vecAddRateList, AddRateList, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecAddRateList.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecAddRateList[i].c_str(), &temp))
				{
					data.mAddRateList.push_back(temp);
				}
			}
		}
		data.mCmpParam1 = element->FloatAttribute("CmpParam1");
		data.mCmpParam2 = element->FloatAttribute("CmpParam2");
		data.mCmpParam3 = element->FloatAttribute("CmpParam3");
		data.mAllInParam1 = element->FloatAttribute("AllInParam1");
		data.mAllInParam2 = element->FloatAttribute("AllInParam2");
		data.mGiveUpParam = element->FloatAttribute("GiveUpParam");
		if (mMapData.find(data.mModelID) != mMapData.end())std::cout <<"data refind:" << data.mModelID << std::endl;
		assert(mMapData.find(data.mModelID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mModelID, data));
		element = element->NextSiblingElement();
	}
}

GoldFlower_RobotActionCFG* GoldFlower_RobotActionCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new GoldFlower_RobotActionCFG());
	}
	return msSingleton.get();
}
