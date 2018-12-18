#include "stdafx.h"
#include "logic_cardmgr.h"
#include <random>

ROBCOWS_SPACE_USING

const uint32_t Cow_Rate[] = {
	1,
	1,1,1,1,1,1,
	2,2,2,
	3,
	4,
	5,
	5
};

logic_cardmgr::logic_cardmgr(void)
{
}


logic_cardmgr::~logic_cardmgr(void)
{
}

void robcows_space::logic_cardmgr::Reposition()
{
	m_vecCardData.clear();
	m_GameCardData.clear();
	m_SaveGameCardData.clear();

	m_nBasePair = 0;

	m_VecSupperID.clear();
	m_VecRobotID.clear();
	m_VecUserID_A.clear();
	m_VecUserID_B.clear();
	m_VecUserID_C.clear();

	m_Card2Supper.clear();
	m_Card2RobotS.clear();
	m_Card2RobotL.clear();
	m_Card2UserA.clear();
	m_Card2UserB.clear();
	m_Card2UserC.clear();

	m_MapGameCard.clear();

	m_MapUserPos.clear();
}

void robcows_space::logic_cardmgr::SetUserLabel(uint32_t uid,int32_t nLabel)
{
	if(nLabel==Tag_Supper)
	{
		m_VecSupperID.push_back(uid);
	}
	else if(nLabel==Tag_UserA)
	{
		m_VecUserID_A.push_back(uid);
	}
	else if(nLabel==Tag_UserB)
	{
		m_VecUserID_B.push_back(uid);
	}
	else if(nLabel==Tag_UserC)
	{
		m_VecUserID_C.push_back(uid);
	}
	else if(nLabel==Tag_Robot)
	{
		m_VecRobotID.push_back(uid);
	}
	else
	{
		assert(0);
	}
}

void robcows_space::logic_cardmgr::SetUserPos(uint32_t uid, int32_t pos)
{
	if (uid != 0)
	{
		m_MapUserPos.insert(std::pair<uint32_t,int32_t>(uid,pos));
	}
}


void robcows_space::logic_cardmgr::Wash_Card()
{
	int32_t nRndom = m_GameLogic.GetRand_100();
	m_vecCardData.clear();
	m_GameLogic.WashPai(m_vecCardData);
	m_nBasePair  =	m_VecUserID_A.size()+
					m_VecUserID_B.size()+
					m_VecUserID_C.size()+
					m_VecRobotID.size()/**ROBOT_GAME_CARD*/+
					m_VecSupperID.size();
	assert(m_nBasePair!=0);
	for (int i=0;i<m_nBasePair;i++)
	{
		double fRandom = global_random::instance().rand_double(1,100);
		Vector_Card findcard;
		GameCard card;
		memset(&card,0,sizeof(card));
#if 0
		if(fRandom<=40)
		{
			m_GameLogic.GetCard2Cows(m_vecCardData,findcard);
		}
		else
		{
			m_GameLogic.GetCard2NoCows(m_vecCardData,findcard);
		}
		assert(findcard.size()==MAX_CARDS_HAND);
		for (int i=0;i<MAX_CARDS_HAND;i++)
		{
			card.nCard[i] = findcard[i];
		}
#else
		for (int j = 0; j < MAX_CARDS_HAND; j++)
		{
			card.nCard[j] = m_vecCardData.back();
			m_vecCardData.pop_back();
		}
#endif

#if 1
		std::sort(card.nCard.begin(),card.nCard.end(),[](int32_t a,int32_t b){
			int32_t avalue = a&MASK_VALUE;
			int32_t bvalue = b&MASK_VALUE;
			if(avalue > bvalue)
			{
				return true;
			}
			else if(avalue==bvalue)
			{
				return (a&MASK_COLOR) > (b&MASK_COLOR);
			}
			else
			{
				return false;
			}
		//	return (a&MASK_VALUE) > (b&MASK_VALUE);
		});
#endif
		// check
		m_GameLogic.CheckCardModel_RobCows(card);
		// add
		m_GameCardData.push_back(card);
	}
	// sort
	std::sort(m_GameCardData.begin(),m_GameCardData.end());
	// index
	int32_t nIndex = 0;
	for (auto it=m_GameCardData.begin();it!=m_GameCardData.end();it++)
	{
		it->nIndex = nIndex++;
	}
	// pre-allocation
	//PreAllocationEx();
}

void robcows_space::logic_cardmgr::Wash_Card2()
{
	int32_t nCounter = 0;
	m_vecCardData.clear();
	Vec_GameCard GameCardData;
	do 
	{	
		Vector_Card	vecCardData;
		m_GameLogic.WashPai(vecCardData);
		GameCardData.clear();
		nCounter = 0;
		for (int i = 0; i < GAME_PLAYER; i++)
		{
			GameCard card;
			memset(&card, 0, sizeof(card));
			for (int j = 0; j < MAX_CARDS_HAND; j++)
			{
				card.nCard[j] = vecCardData.back();
				vecCardData.pop_back();
			}
			std::sort(card.nCard.begin(), card.nCard.end(), [](int32_t a, int32_t b) {
				int32_t avalue = a&MASK_VALUE;
				int32_t bvalue = b&MASK_VALUE;
				if (avalue > bvalue)
				{
					return true;
				}
				else if (avalue == bvalue)
				{
					return (a&MASK_COLOR) > (b&MASK_COLOR);
				}
				else
				{
					return false;
				}
			});
			// check
			m_GameLogic.CheckCardModel_RobCows(card);
			if (card.nModel == CARD_VALUE_ZERO)
			{
				nCounter++;
			}
			// add
			GameCardData.push_back(card);
		}

	} while (nCounter >= 2);
	// 
	std::random_device rd;
	std::mt19937_64 g(rd());
	std::shuffle(GameCardData.begin(), GameCardData.end(), g);
	for (auto user : m_MapUserPos)
	{
		auto idx = user.second;
		if (idx < GameCardData.size())
		{
			m_GameCardData.push_back(GameCardData.at(idx));
		}
		else
		{
			//assert(0);
			SLOG_ERROR << "err: 225";
		}
	}
	// sort
	std::sort(m_GameCardData.begin(), m_GameCardData.end());
	// index
	int32_t nIndex = 0;
	for (auto it = m_GameCardData.begin(); it != m_GameCardData.end(); it++)
	{
		it->nIndex = nIndex++;
	}
}

void robcows_space::logic_cardmgr::Wash_Card3()
{
	m_vecCardData.clear();
	m_GameLogic.WashPai(m_vecCardData);
	Vec_GameCard GameCardData;
	GameCardData.clear();
	for (int i = 0; i <= GAME_PLAYER; i++)
	{
		GameCard card;
		memset(&card, 0, sizeof(card));
		for (int j = 0; j < MAX_CARDS_HAND; j++)
		{
			card.nCard[j] = m_vecCardData.back();
			m_vecCardData.pop_back();
		}
		std::sort(card.nCard.begin(), card.nCard.end(), [](int32_t a, int32_t b) {
			int32_t avalue = a&MASK_VALUE;
			int32_t bvalue = b&MASK_VALUE;
			if (avalue > bvalue)
			{
				return true;
			}
			else if (avalue == bvalue)
			{
				return (a&MASK_COLOR) > (b&MASK_COLOR);
			}
			else
			{
				return false;
			}
			//	return (a&MASK_VALUE) > (b&MASK_VALUE);
		});
		// check
		m_GameLogic.CheckCardModel_RobCows(card);
		// add
		GameCardData.push_back(card);
	}
	// sort
	std::sort(GameCardData.begin(), GameCardData.end());
	// index
	int32_t nIndex = 0;
	for (auto it = GameCardData.begin(); it != GameCardData.end(); it++)
	{
		it->nIndex = nIndex++;
		if (nIndex == 1)
		{
			it = GameCardData.erase(it);
		}
	}
	// 
	std::random_device rd;
	std::mt19937_64 g(rd());
	std::shuffle(GameCardData.begin(), GameCardData.end(), g);
	for (auto user : m_MapUserPos)
	{
		auto idx = user.second;
		if (idx < GameCardData.size())
		{
			m_GameCardData.push_back(GameCardData.at(idx));
		}
		else
		{
			//assert(0);
			SLOG_ERROR << "err: 301";
		}
	}
	// sort
	std::sort(m_GameCardData.begin(), m_GameCardData.end());
	// index
	nIndex = 0;
	for (auto it = m_GameCardData.begin(); it != m_GameCardData.end(); it++)
	{
		it->nIndex = nIndex++;
	}
}

void robcows_space::logic_cardmgr::Wash_CardNomal()
{
	m_vecCardData.clear();
	m_GameLogic.WashPai(m_vecCardData);
	Vec_GameCard GameCardData;
	GameCardData.clear();

	int32_t nCount[GAME_PLAYER] = { 0 };
	GameCard arrGameCard[GAME_PLAYER];
	memset(arrGameCard, 0, sizeof(arrGameCard));

	for (int i = 0; i < MAX_CARDS_HAND * GAME_PLAYER; i++)
	{
		int32_t nCardData = m_vecCardData.back();
		m_vecCardData.pop_back();
		int nMan = i%MAX_CARDS_HAND;	
		GameCard &card = arrGameCard[nMan];
		int32_t & nCnt = nCount[nMan];
		card.nCard[nCnt++] = nCardData;
		if (nCnt == MAX_CARDS_HAND)
		{
			std::sort(card.nCard.begin(), card.nCard.end(), [](int32_t a, int32_t b) {
				int32_t avalue = a&MASK_VALUE;
				int32_t bvalue = b&MASK_VALUE;
				if (avalue > bvalue)
				{
					return true;
				}
				else if (avalue == bvalue)
				{
					return (a&MASK_COLOR) > (b&MASK_COLOR);
				}
				else
				{
					return false;
				}
			});
			// check
			m_GameLogic.CheckCardModel_RobCows(card);
			// add
			GameCardData.push_back(card);
		}
	}

	for (auto user : m_MapUserPos)
	{
		auto uid = user.first;
		auto idx = user.second;
		if (idx < GameCardData.size())
		{
			m_GameCardData.push_back(GameCardData.at(idx));
		}
		else
		{
			SLOG_ERROR << "err Wash_CardNomal ";
		}
	}
}

void robcows_space::logic_cardmgr::Wash_CardControl()
{
	m_vecCardData.clear();
	m_GameLogic.WashPai(m_vecCardData);
	Vec_GameCard GameCardData;
	GameCardData.clear();
	for (int i = 0; i <= GAME_PLAYER; i++)
	{
		GameCard card;
		memset(&card, 0, sizeof(card));
		for (int j = 0; j < MAX_CARDS_HAND; j++)
		{
			card.nCard[j] = m_vecCardData.back();
			m_vecCardData.pop_back();
		}
		std::sort(card.nCard.begin(), card.nCard.end(), [](int32_t a, int32_t b) {
			int32_t avalue = a&MASK_VALUE;
			int32_t bvalue = b&MASK_VALUE;
			if (avalue > bvalue)
			{
				return true;
			}
			else if (avalue == bvalue)
			{
				return (a&MASK_COLOR) > (b&MASK_COLOR);
			}
			else
			{
				return false;
			}
			//	return (a&MASK_VALUE) > (b&MASK_VALUE);
		});
		// check
		m_GameLogic.CheckCardModel_RobCows(card);
		// add
		GameCardData.push_back(card);
	}
	// sort
	std::sort(GameCardData.begin(), GameCardData.end());
	// index
	int32_t nIndex = 0;
	for (auto it = GameCardData.begin(); it != GameCardData.end(); it++)
	{
		it->nIndex = nIndex++;
		if (nIndex == 1)
		{
			it = GameCardData.erase(it);
		}
	}
	// 
	std::random_device rd;
	std::mt19937_64 g(rd());
	std::shuffle(GameCardData.begin(), GameCardData.end(), g);
	for (auto user : m_MapUserPos)
	{
		auto idx = user.second;
		if (idx < GameCardData.size())
		{
			m_GameCardData.push_back(GameCardData.at(idx));
		}
		else
		{
			//assert(0);
			SLOG_ERROR << "err: Wash_CardControl";
		}
	}
	// sort
	std::sort(m_GameCardData.begin(), m_GameCardData.end());
	// index
	nIndex = 0;
	for (auto it = m_GameCardData.begin(); it != m_GameCardData.end(); it++)
	{
		it->nIndex = nIndex++;
	}
}

void robcows_space::logic_cardmgr::PreAllocation()
{
	if(m_GameCardData.size()==0) return;
	// 超级用户
	for (uint32_t i=0;i<m_VecSupperID.size();i++)
	{
		uint32_t uid = m_VecSupperID[i];
		Vec_GameCard vcard;
		GameCard card = m_GameCardData.back();
		m_GameCardData.pop_back();
		vcard.push_back(card);
		m_MapGameCard.insert(std::make_pair(uid,vcard));
	}
	// 机器
	for (uint32_t i=0;i<m_VecRobotID.size();i++)
	{
		uint32_t uid = m_VecRobotID[i];
		Vec_GameCard vcard;
		if(m_GameCardData.size()>=2)
		{
			GameCard card = m_GameCardData.back();
			m_GameCardData.pop_back();
			vcard.push_back(card);
			std::reverse(std::begin(m_GameCardData),std::end(m_GameCardData));
			card = m_GameCardData.back();
			m_GameCardData.pop_back();
			vcard.push_back(card);
			m_MapGameCard.insert(std::make_pair(uid,vcard));
			std::reverse(std::begin(m_GameCardData),std::end(m_GameCardData));
		}
	}
	// 玩家
	std::vector<PAIR> vec_playerid(m_MapPlayerID.begin(),m_MapPlayerID.end());
#if 0
	std::sort(vec_playerid.begin(),vec_playerid.end(),CmpByValue());
#else
	std::sort(vec_playerid.begin(),vec_playerid.end(),[](const PAIR& lhs, const PAIR& rhs) {
		return lhs.second > rhs.second;
	});
#endif
	for (int i=0;i != vec_playerid.size();i++)
	{
//		printf("[%d]:[%d]\n",vec_playerid[i].first,vec_playerid[i].second);
		uint32_t uid = vec_playerid[i].first;
		Vec_GameCard vcard;
		if(m_GameCardData.size()>=1)
		{
			GameCard card = m_GameCardData.back();
			m_GameCardData.pop_back();
			vcard.push_back(card);
			std::reverse(std::begin(m_GameCardData),std::end(m_GameCardData));
			m_MapGameCard.insert(std::make_pair(uid,vcard));
		}
	}
}

void robcows_space::logic_cardmgr::PreAllocationEx()
{
	uint32_t nPairs = m_GameCardData.size();
	if(nPairs==0) return;
	if(nPairs != m_nBasePair)
	{
		SLOG_ERROR<<"PreAllocationEx Card Pair err:BasePair"<<m_nBasePair<<",pair:"<<nPairs;
		assert(0);
		return;
	}
	m_SaveGameCardData = m_GameCardData;
	// 超级用户
	m_Card2Supper.clear();
	for (uint32_t i=0;i<m_VecSupperID.size();i++)
	{
		GameCard card = m_GameCardData.back();
		m_GameCardData.pop_back();
		m_Card2Supper.push_back(card);
	}
	// 机器人
	m_Card2RobotL.clear();
	for (uint32_t i=0;i<m_VecRobotID.size();i++)
	{
		GameCard card = m_GameCardData.back();
		m_GameCardData.pop_back();
		m_Card2RobotL.push_back(card);
	}
	// A 类玩家
	m_Card2UserA.clear();
	for (uint32_t i=0;i<m_VecUserID_A.size();i++)
	{
		GameCard card = m_GameCardData.back();
		m_GameCardData.pop_back();
		m_Card2UserA.push_back(card);
	}
	// B 类玩家
	m_Card2UserB.clear();
	for (uint32_t i=0;i<m_VecUserID_B.size();i++)
	{
		GameCard card = m_GameCardData.back();
		m_GameCardData.pop_back();
		m_Card2UserB.push_back(card);
	}
	// C 类玩家
	m_Card2UserC.clear();
	for (uint32_t i=0;i<m_VecUserID_C.size();i++)
	{
		GameCard card = m_GameCardData.back();
		m_GameCardData.pop_back();
		m_Card2UserC.push_back(card);
	}
	// 机器人
	m_Card2RobotS.clear();
	for (uint32_t i=0;i<m_VecRobotID.size();i++)
	{
		GameCard card = m_GameCardData.back();
		m_GameCardData.pop_back();
		m_Card2RobotS.push_back(card);
	}
	
	// 
	std::random_device rd;
	std::mt19937_64 g(rd());
	std::shuffle(m_Card2Supper.begin(),m_Card2Supper.end(),g);

	std::srand(unsigned(std::time(0)));
	std::random_shuffle(m_Card2RobotL.begin(),m_Card2RobotL.end());
	std::random_shuffle(m_Card2RobotS.begin(),m_Card2RobotS.end());

	std::shuffle(m_Card2UserA.begin(),m_Card2UserA.end(),g);
	std::shuffle(m_Card2UserB.begin(),m_Card2UserB.end(),g);
	std::shuffle(m_Card2UserC.begin(),m_Card2UserC.end(),g);

}

bool robcows_space::logic_cardmgr::GetUserCard(GameCard &card)
{
	if (m_GameCardData.empty()) return false;
	card = m_GameCardData.back();
	m_GameCardData.pop_back();
	return true;
}

bool robcows_space::logic_cardmgr::GetUserCard(uint32_t playerid,GameCard &card)
{
#if 0
	auto it = m_MapGameCard.find(playerid);
	if(it==m_MapGameCard.end()) return false;
	Vec_GameCard vcard = it->second;
	if(vcard.size()==0) return false;
	card = vcard.at(0);
	return true;
#else
	auto result = std::find(m_VecSupperID.begin(),m_VecSupperID.end(),playerid);
	if(result!=m_VecSupperID.end())
	{
		card = m_Card2Supper.back();
		m_Card2Supper.pop_back();
		return true;
	}
	auto result1 = std::find(m_VecRobotID.begin(),m_VecRobotID.end(),playerid);
	if(result1 != m_VecRobotID.end())
	{
		card = m_Card2RobotS.back();
		m_Card2RobotS.pop_back();
		return true;
	}
	// User
	auto result2 = std::find(m_VecUserID_A.begin(),m_VecUserID_A.end(),playerid);
	if(result2 != m_VecUserID_A.end())
	{
		card = m_Card2UserA.back();
		m_Card2UserA.pop_back();
		return true;
	}
	auto result3 = std::find(m_VecUserID_B.begin(),m_VecUserID_B.end(),playerid);
	if(result3 != m_VecUserID_B.end())
	{
		card = m_Card2UserB.back();
		m_Card2UserB.pop_back();
		return true;
	}
	auto result4 = std::find(m_VecUserID_C.begin(),m_VecUserID_C.end(),playerid);
	if(result4 != m_VecUserID_C.end())
	{
		card = m_Card2UserC.back();
		m_Card2UserC.pop_back();
		return true;
	}
#endif
	return false;
}

uint32_t robcows_space::logic_cardmgr::GetCardRate(GameCard &card) const
{
	int32_t nModel = card.nModel;
	if(nModel>=0 && nModel<=CARD_VALUE_WUXIAO)
		return Cow_Rate[nModel];
	return 0;
}

bool robcows_space::logic_cardmgr::SwitchCard(uint32_t playerid,GameCard &card/*,bool bLarge/ *=false* /*/)
{
	auto result = std::find(m_VecRobotID.begin(),m_VecRobotID.end(),playerid);
	if(result==m_VecRobotID.end()) return false;
	if(m_Card2RobotL.size()>0)
	{
		card = m_Card2RobotL.back();
		m_Card2RobotL.pop_back();
		return true;
	}
	return false;
}

bool robcows_space::logic_cardmgr::WashCard_ModelA()
{
	// 计算发牌数量
	int32_t nCount =DEL_GAME_CARD+m_nBasePair;
	assert(nCount<=MAX_GAME_CARD);
	for (int i = 0; i < nCount; i++)
	{
		GameCard card;
		memset(&card,0,sizeof(card));
		for (int j=0;j<MAX_CARDS_HAND;j++)
		{
			card.nCard[j] = m_vecCardData.back();
			m_vecCardData.pop_back();
		}
#if 1
		std::sort(card.nCard.begin(),card.nCard.end(),[](int32_t a,int32_t b){
			return (a&MASK_VALUE) > (b&MASK_VALUE);
		});
#endif
		m_GameLogic.CheckCardModel(card);
		m_GameCardData.push_back(card);
	}

	std::sort(m_GameCardData.begin(),m_GameCardData.end());
	
	std::reverse(m_GameCardData.begin(),m_GameCardData.end());
	for (int32_t i=0;i<DEL_GAME_CARD;i++)
	{
		m_GameCardData.pop_back();
	}
	std::reverse(std::begin(m_GameCardData),std::end(m_GameCardData));
	// index
	int32_t nIndex = 0;
	for (auto it=m_GameCardData.begin();it!=m_GameCardData.end();it++)
	{
		it->nIndex = nIndex++;
	}
	return true;
}

bool robcows_space::logic_cardmgr::WashCard_ModelB()
{ // 5个机器人不能使用
	// 计算发牌数量
	int32_t nCount =2*m_nBasePair-1;
	assert(nCount<=MAX_GAME_CARD);
	for (int i = 0; i < nCount; i++)
	{
		GameCard card;
		memset(&card,0,sizeof(card));
		for (int j=0;j<MAX_CARDS_HAND;j++)
		{
			card.nCard[j] = m_vecCardData.back();
			m_vecCardData.pop_back();
		}
#if 1
		std::sort(card.nCard.begin(),card.nCard.end(),[](int32_t a,int32_t b){
			return (a&MASK_VALUE) > (b&MASK_VALUE);
		});
#endif
		m_GameLogic.CheckCardModel(card);
		m_GameCardData.push_back(card);
	}

	std::sort(m_GameCardData.begin(),m_GameCardData.end());
	// index
	int32_t nIndex = 0;
	for (auto it=m_GameCardData.begin();it!=m_GameCardData.end();it++)
	{
		it->nIndex = nIndex++;
	}
	if(nIndex<(m_nBasePair-1)*2)
	{
		assert(0);
		return false;
	}
	int32_t nDelCount = 0;
	while (nDelCount<m_nBasePair-1)
	{
		auto it_find = m_GameCardData.end();
		for (auto it=m_GameCardData.begin();it!=m_GameCardData.end();it++)
		{
			if(it->nIndex == nDelCount*2)
			{
				it_find = it;
				break;
			}
		}
		if(it_find!=m_GameCardData.end())
		{
			m_GameCardData.erase(it_find);
			nDelCount++;
		}
	}
	//
	nIndex = 0;
	for (auto it=m_GameCardData.begin();it!=m_GameCardData.end();it++)
	{
		it->nIndex = nIndex++;
	}
	return true;
}

bool robcows_space::logic_cardmgr::WashCard_ModelC()
{
	int32_t nCount = m_nBasePair;
	assert(nCount<=MAX_GAME_CARD);
	for (int32_t i=0;i<nCount;i++)
	{
		Vector_Card findcard;
		GameCard card;
		memset(&card,0,sizeof(card));
		uint32_t nRand = m_GameLogic.GetRand_100();
		int32_t nCardLvl = CARDLEVEL_DANPAI;
		if(nRand<=5)
		{ // 
			nCardLvl = CARDLEVEL_BAOZI;
		}
		else if (nRand<=10)
		{
			nCardLvl = CARDLEVEL_SHUNJIN;
		}
		else if (nRand<=20)
		{
			nCardLvl = CARDLEVEL_JINHUA;
		}
		else if(nRand<=30)
		{
			nCardLvl = CARDLEVEL_SHUNZI;
		}
		else if (nRand<=70)
		{
			nCardLvl = CARDLEVEL_DUIZI;
		}
		else
		{
			nCardLvl = CARDLEVEL_DANPAI;
		}

		m_GameLogic.GetCardByModel(m_vecCardData,findcard,nCardLvl);
		assert(findcard.size()==MAX_CARDS_HAND);
		for (int i=0;i<MAX_CARDS_HAND;i++)
		{
			card.nCard[i] = findcard[i];
		}
#if 1
		std::sort(card.nCard.begin(),card.nCard.end(),[](int32_t a,int32_t b){
			return (a&MASK_VALUE) > (b&MASK_VALUE);
		});
#endif
		m_GameLogic.CheckCardModel(card);
		m_GameCardData.push_back(card);
	}

	std::sort(m_GameCardData.begin(),m_GameCardData.end());
	// index
	int32_t nIndex = 0;
	for (auto it=m_GameCardData.begin();it!=m_GameCardData.end();it++)
	{
		it->nIndex = nIndex++;
	}
	return true;
}

bool robcows_space::logic_cardmgr::WashCard_ModelD()
{
	int32_t nCount = m_nBasePair;
	assert(nCount<=MAX_GAME_CARD);
	for (int32_t i=0;i<nCount;i++)
	{
		Vector_Card findcard;
		GameCard card;
		memset(&card,0,sizeof(card));
		uint32_t nRand = m_GameLogic.GetRand_100();
		int32_t nCardLvl = CARDLEVEL_DANPAI;
		if (nRand<=10)
		{
			nCardLvl = CARDLEVEL_SHUNJIN;
		}
		else if (nRand<=40)
		{
			nCardLvl = CARDLEVEL_JINHUA;
		}
		else if (nRand<=60)
		{
			nCardLvl = CARDLEVEL_SHUNZI;
		}
		else
		{
			nCardLvl = CARDLEVEL_DANPAI;
		}
		m_GameLogic.GetCardByModel(m_vecCardData,findcard,nCardLvl);
		assert(findcard.size()==MAX_CARDS_HAND);
		for (int i=0;i<MAX_CARDS_HAND;i++)
		{
			card.nCard[i] = findcard[i];
		}
#if 1
		std::sort(card.nCard.begin(),card.nCard.end(),[](int32_t a,int32_t b){
			return (a&MASK_VALUE) > (b&MASK_VALUE);
		});
#endif
		m_GameLogic.CheckCardModel(card);
		m_GameCardData.push_back(card);
	}

	std::sort(m_GameCardData.begin(),m_GameCardData.end());
	// index
	int32_t nIndex = 0;
	for (auto it=m_GameCardData.begin();it!=m_GameCardData.end();it++)
	{
		it->nIndex = nIndex++;
	}
	return true;
}


