#include "stdafx.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <iostream>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include "tinyxml2.h"
#include "RobCows_RobotGameCFG.h"
std::auto_ptr<RobCows_RobotGameCFG> RobCows_RobotGameCFG::msSingleton(nullptr);

int RobCows_RobotGameCFG::GetCount()
{
	return (int)mMapData.size();
}

const RobCows_RobotGameCFGData* RobCows_RobotGameCFG::GetData(int RoomID)
{
	auto it = mMapData.find(RoomID);
	if (it != mMapData.end())
	{
		return &it->second;
	}
	return NULL;
}

boost::unordered_map<int, RobCows_RobotGameCFGData>& RobCows_RobotGameCFG::GetMapData()
{
	return mMapData;
}

void RobCows_RobotGameCFG::Reload()
{
	mMapData.clear();
	Load();
}

void RobCows_RobotGameCFG::Load()
{
	std::ifstream readStream("../Config/RobCows_RobotGameCFG.xml", std::ios::binary);
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
		RobCows_RobotGameCFGData data;
		data.mRoomID = element->IntAttribute("RoomID");
		data.mReadyTime = element->IntAttribute("ReadyTime");
		data.mBankerTime = element->IntAttribute("BankerTime");
		{
			const char* RobList = element->Attribute("RobList");
			std::vector<std::string> vecRobList;
			boost::split(vecRobList, RobList, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecRobList.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecRobList[i].c_str(), &temp))
				{
					data.mRobList.push_back(temp);
				}
			}
		}
		data.mBetTime = element->IntAttribute("BetTime");
		{
			const char* BetList = element->Attribute("BetList");
			std::vector<std::string> vecBetList;
			boost::split(vecBetList, BetList, boost::is_any_of(","));
			int temp;
			for (unsigned int i = 0; i < vecBetList.size(); i++)
			{
				if (tinyxml2::XMLUtil::ToInt(vecBetList[i].c_str(), &temp))
				{
					data.mBetList.push_back(temp);
				}
			}
		}
		data.mOperaTime = element->IntAttribute("OperaTime");
		data.mResultTime = element->IntAttribute("ResultTime");
		if (mMapData.find(data.mRoomID) != mMapData.end())std::cout <<"data refind:" << data.mRoomID << std::endl;
		assert(mMapData.find(data.mRoomID) == mMapData.end());
		mMapData.insert(std::make_pair(data.mRoomID, data));
		element = element->NextSiblingElement();
	}
}

RobCows_RobotGameCFG* RobCows_RobotGameCFG::GetSingleton()
{
	if (msSingleton.get() == nullptr)
	{
		msSingleton.reset(new RobCows_RobotGameCFG());
	}
	return msSingleton.get();
}
