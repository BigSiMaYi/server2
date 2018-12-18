#pragma once
#include <boost/unordered_map.hpp>
#include <vector>
struct Cows_SoundCFGData
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

class Cows_SoundCFG
{
public:
private:
	static std::auto_ptr<Cows_SoundCFG> msSingleton;
public:
	int GetCount();
	const Cows_SoundCFGData* GetData(int SoundID);
	boost::unordered_map<int, Cows_SoundCFGData>& GetMapData();
	void Reload();
	void Load();
	static Cows_SoundCFG* GetSingleton();
private:
	boost::unordered_map<int, Cows_SoundCFGData> mMapData;
};
