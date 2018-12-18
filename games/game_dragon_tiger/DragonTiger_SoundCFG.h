#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct DragonTiger_SoundCFGData
{
	//����ID
	int mSoundID;
	//��������
	std::string mSoundName;
	//����·��
	std::string mSoundPath;
	//����ʱ��(����)
	int mSoundTime;
};

class DragonTiger_SoundCFG
{
public:
private:
	static std::auto_ptr<DragonTiger_SoundCFG> msSingleton;
public:
	int GetCount();
	const DragonTiger_SoundCFGData* GetData(int SoundID);
	boost::unordered_map<int, DragonTiger_SoundCFGData>& GetMapData();
	void Reload();
	void Load();
	static DragonTiger_SoundCFG* GetSingleton();
private:
	boost::unordered_map<int, DragonTiger_SoundCFGData> mMapData;
};
