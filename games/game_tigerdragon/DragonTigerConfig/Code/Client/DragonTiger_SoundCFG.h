#pragma once
#include <map>
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
	const std::map<int, DragonTiger_SoundCFGData>& GetMapData();
	void Load();
	void LoadLua();
	void Reload();
	static DragonTiger_SoundCFG* GetSingleton();
private:
	std::map<int, DragonTiger_SoundCFGData> mMapData;
};
