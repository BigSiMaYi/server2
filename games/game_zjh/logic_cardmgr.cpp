#include "stdafx.h"
#include "logic_cardmgr.h"
#include <random>

ZJH_SPACE_USING

logic_cardmgr::logic_cardmgr(void)
{
}


logic_cardmgr::~logic_cardmgr(void)
{
}


void zjh_space::logic_cardmgr::Reposition()
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
	m_Card2AllUser.clear();

	m_MapGameCard.clear();

	m_CardMgrRate.clear();

	m_MapUserPos.clear();
}

void zjh_space::logic_cardmgr::SetRate(std::vector<int>& nCardRate)
{
	if(nCardRate.size()==0) return;

	m_CardMgrRate = nCardRate;
}

void zjh_space::logic_cardmgr::SetUserLabel(uint32_t uid,int32_t nLabel)
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

void zjh_space::logic_cardmgr::SetUserPos(uint32_t uid, int32_t pos)
{
	if (uid != 0)
	{
		m_MapUserPos.insert(std::pair<uint32_t, int32_t>(uid, pos));
	}
}

void zjh_space::logic_cardmgr::Wash_Card()
{
	int32_t nRndom = m_GameLogic.GetRand_100();
	m_vecCardData.clear();
	m_GameLogic.WashPai(m_vecCardData);
	SLOG_DEBUG<<"=========Wash_Card=========";
	SLOG_DEBUG<<boost::format("Wash_Card::nRndom:%1% len:%2%")%nRndom%m_vecCardData.size();
	for (auto rate : m_CardMgrRate)
	{
		SLOG_DEBUG<<boost::format(":%1%,")%rate;
	}
	SLOG_DEBUG<<"=========Wash_Card=========";

	m_nBasePair  =	m_VecUserID_A.size()+
		m_VecUserID_B.size()+
		m_VecUserID_C.size()+
		m_VecRobotID.size()*ROBOT_GAME_CARD+
		m_VecSupperID.size();
	assert(m_nBasePair!=0);

	if(m_CardMgrRate.size() == 5)
	{
		int nRateA = 0,nRateB = 0,nRateC = 0, nRateD = 0 ,nRateE=0 ;
		nRateA = m_CardMgrRate[0];
		nRateB = m_CardMgrRate[1];
		nRateC = m_CardMgrRate[2];
		nRateD = m_CardMgrRate[3];
		nRateE = m_CardMgrRate[4];
		int nRateSum = nRateA+nRateB+nRateC+nRateD+nRateE;
		if(nRateSum>100 || nRateSum<0)
		{
			nRateA = 20;
			nRateB = 20;
			nRateC = 20;
			nRateD = 20;
			nRateE = 20;
		}
		bool bRes = false;
		if(nRndom<=nRateA)
		{ // 发牌 1
			bRes = WashCard_ModelA();
		}
		else if(nRndom<=nRateA+nRateB)
		{ // 发牌 2
			bRes = WashCard_ModelB();
		}
		else if (nRndom<=nRateA+nRateB+nRateC)
		{ // 发牌 3
			bRes = WashCard_ModelC();
		}
		else if (nRndom<=nRateA+nRateB+nRateC+nRateD)
		{ // 发牌 4
			bRes = WashCard_ModelD();
		}
		else
		{ // 发牌 5
			bRes = WashCard_ModelE();
		}
		//
		if(!bRes)
		{
			WashCard_ModelE();
		}
	}
	else
	{
		bool bRes = false;
		if(nRndom<=20)
		{ // 发牌 1
			bRes = WashCard_ModelA();
		}
		else if(nRndom<=40)
		{ // 发牌 2
			bRes = WashCard_ModelB();
		}
		else if (nRndom<=60)
		{ // 发牌 3
			bRes = WashCard_ModelC();
		}
		else if (nRndom<=80)
		{ // 发牌 4
			bRes = WashCard_ModelD();
		}
		else
		{ // 发牌 5
			bRes = WashCard_ModelE();
		}
		//
		if(!bRes)
		{
			WashCard_ModelE();
		}
	}
#if 0
	PreAllocation();
#else
	PreAllocationEx();
#endif
}


void zjh_space::logic_cardmgr::Wash_CardEx()
{
	m_vecCardData.clear();
	m_GameLogic.WashPai(m_vecCardData);
	SLOG_DEBUG<<"=========Wash_Card=========";
	SLOG_DEBUG<<boost::format("Wash_Card::len:%1%")%m_vecCardData.size();
// 	for (auto rate : m_CardMgrRate)
// 	{
// 		SLOG_DEBUG<<boost::format(":%1%,")%rate;
// 	}
	SLOG_DEBUG<<"=========Wash_Card=========";
	m_nBasePair  =	m_VecUserID_A.size()+
		m_VecUserID_B.size()+
		m_VecUserID_C.size()+
		m_VecRobotID.size()*ROBOT_GAME_CARD+
		m_VecSupperID.size();
	assert(m_nBasePair!=0);
	if(m_VecRobotID.size() > 0)
	{ // 有机器人
// 		if (m_VecRobotID.size() == GAME_PLAYER)
// 			WashCard_ModelAEx();
// 		else
// 			WashCard_ModelAEx2();
		WashCard_ModelAEx();

		PreAllocationEx(true);
	}
	else
	{ // 没有机器人
		WashCard_ModelE();
		
		PreAllocationEx(false);
	}

	// PreAllocationEx();
}

void zjh_space::logic_cardmgr::Wash_CardEx2()
{
	m_vecCardData.clear();
	m_GameLogic.WashPai(m_vecCardData);
	m_nBasePair = m_VecUserID_A.size() +
		m_VecUserID_B.size() +
		m_VecUserID_C.size() +
		m_VecRobotID.size()  +
		m_VecSupperID.size() + 1;
	//
	//WashCard_ModelE();
	// 
	WashCard_ModelF();

	m_Card2RobotL.clear();
	for (uint32_t i = 0; i < 1; i++)
	{
		GameCard card = m_GameCardData.back();
		m_GameCardData.pop_back();
		m_Card2RobotL.push_back(card);
	}
	// random
	std::random_device rd;
	std::seed_seq seed2{ rd(),  rd(),  rd(), rd(),  rd(),  rd(),  rd(),  rd() };
	std::mt19937_64 g2(seed2);
	for (int i = 0; i < 4; i++)
	{
		std::shuffle(m_GameCardData.begin(), m_GameCardData.end(), g2);
	}
}

void zjh_space::logic_cardmgr::Wash_CardNomal()
{
	// @随机--分次随机
	m_vecCardData.clear();
	m_GameLogic.WashPai(m_vecCardData);

	Vec_GameCard GameCardData;
	GameCardData.clear();
	int32_t nCount[GAME_PLAYER] = { 0 };
	GameCard arrGameCard[GAME_PLAYER];

	for (int i = 0; i < MAX_CARDS_HAND * GAME_PLAYER; i++)
	{
		int32_t nCardData = m_vecCardData.back();
		m_vecCardData.pop_back();
		int nMan = i%GAME_PLAYER;
		GameCard &card = arrGameCard[nMan];
		int32_t & nCnt = nCount[nMan];
		card.nCard[nCnt++] = nCardData;
		if (nCnt == MAX_CARDS_HAND)
		{
			//
			std::sort(card.nCard.begin(), card.nCard.end(), [](int32_t a, int32_t b) {
				return (a&MASK_VALUE) > (b&MASK_VALUE);
			});
			m_GameLogic.CheckCardModel(card);
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
// 	// sort
// 	std::sort(m_GameCardData.begin(), m_GameCardData.end());
// 	// index
// 	int32_t nIndex = 0;
// 	for (auto it = m_GameCardData.begin(); it != m_GameCardData.end(); it++)
// 	{
// 		it->nIndex = nIndex++;
// 	}
	// random
	std::random_device rd;
	std::seed_seq seed2{ rd(),  rd(),  rd(), rd(),  rd(),  rd(),  rd(),  rd() };
	std::mt19937_64 g2(seed2);
	for (int i = 0; i < 4; i++)
	{
		std::shuffle(m_GameCardData.begin(), m_GameCardData.end(), g2);
	}
}

void zjh_space::logic_cardmgr::Wash_CardControl()
{
	// @杀分模式下: 最大的牌排在第一位，其他位置的牌随机
	const static int s_Card_Count = 10;
	const static int s_First_Count = 2;

	m_vecCardData.clear();
	m_GameLogic.WashPai(m_vecCardData);

	Vec_GameCard GameCardData;
	GameCardData.clear();
	int32_t nCount[/*GAME_PLAYER*/s_Card_Count] = { 0 };
	GameCard arrGameCard[/*GAME_PLAYER*/s_Card_Count];
	
	for (int i = 0; i < MAX_CARDS_HAND * /*GAME_PLAYER*/s_Card_Count; i++)
	{
		int32_t nCardData = m_vecCardData.back();
		m_vecCardData.pop_back();
		int nMan = i%/*GAME_PLAYER*/s_Card_Count;
		GameCard &card = arrGameCard[nMan];
		int32_t & nCnt = nCount[nMan];
		card.nCard[nCnt++] = nCardData;
		if (nCnt == MAX_CARDS_HAND)
		{
			std::sort(card.nCard.begin(), card.nCard.end(), [](int32_t a, int32_t b) {
				return (a&MASK_VALUE) > (b&MASK_VALUE);
			});
			m_GameLogic.CheckCardModel(card);
			GameCardData.push_back(card);
		}
	}
	std::sort(GameCardData.begin(), GameCardData.end());

	// @取牌
	for (int i=0; i < s_First_Count; i++)
	{
		if (i < GameCardData.size())
		{
			m_GameCardData.push_back(GameCardData.back());
			GameCardData.pop_back();
		}
		else
		{
			SLOG_ERROR << "err Wash_CardControl ";
		}
	}
	// random
	std::random_device rd;
	std::seed_seq seed2{ rd(),  rd(),  rd(), rd(),  rd(),  rd(),  rd(),  rd() };
	std::mt19937_64 g2(seed2);
	for (int i = 0; i < 4; i++)
	{
		std::shuffle(GameCardData.begin(), GameCardData.end(), g2);
	}
	auto nSize = m_MapUserPos.size();
	if (nSize <= s_First_Count) nSize = s_First_Count;
	for (int i=0;i<nSize-s_First_Count;i++)
	{
		m_GameCardData.push_back(GameCardData.back());
		GameCardData.pop_back();
	}
// 	for (auto user : m_MapUserPos)
// 	{
// 		auto uid = user.first;
// 		auto idx = user.second;
// 		if (idx < GameCardData.size())
// 		{
// 			m_GameCardData.push_back(GameCardData.at(idx));
// 		}
// 		else
// 		{
// 			SLOG_ERROR << "err Wash_CardControl ";
// 		}
// 	}
	// 
	std::sort(m_GameCardData.begin(), m_GameCardData.end());

// 	// index
// 	int32_t nIndex = 0;
// 	for (auto it = m_GameCardData.begin(); it != m_GameCardData.end(); it++)
// 	{
// 		it->nIndex = nIndex++;
// 	}

}


void zjh_space::logic_cardmgr::PreAllocation()
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

void zjh_space::logic_cardmgr::PreAllocationEx(bool bSort/*=true*/)
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
	if (bSort == false)
	{
		std::random_device rd;
		std::mt19937_64 g(rd());
		std::shuffle(m_GameCardData.begin(), m_GameCardData.end(), g);
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

bool zjh_space::logic_cardmgr::GetUserCard(GameCard &card, char bCardLvL/*=eCardlvl_Normal*/)
{
	bool bSucceed = true;
	if (bCardLvL == eCardLvL_Large)
	{
		if (m_Card2RobotL.empty()) return false;
		card = m_Card2RobotL.back();
		m_Card2RobotL.pop_back();
	}
	else if(bCardLvL == eCardlvl_Normal)
	{
		if (m_GameCardData.empty()) return false;
		card = m_GameCardData.back();
		m_GameCardData.pop_back();
	}
	else if (bCardLvL == eCardLvL_Random)
	{
		memset(&card, 0, sizeof(card));
		for (int j = 0; j < MAX_CARDS_HAND; j++)
		{
			card.nCard[j] = m_vecCardData.back();
			m_vecCardData.pop_back();
		}
		std::sort(card.nCard.begin(), card.nCard.end(), [](int32_t a, int32_t b) {
			return (a&MASK_VALUE) > (b&MASK_VALUE);
		});
		m_GameLogic.CheckCardModel(card);
	}
	else if(bCardLvL == eCardLvL_Small)
	{//
		std::random_device rd;
		std::mt19937_64 g(rd());

		GameCard savecard = card;
		Vector_Card	CardData = m_vecCardData;
		int32_t nCounter = 0;
		do
		{
			memset(&card, 0, sizeof(card));
			for (int j = 0; j < MAX_CARDS_HAND; j++)
			{
				card.nCard[j] = CardData.back();
				CardData.pop_back();
			}
			std::sort(card.nCard.begin(), card.nCard.end(), [](int32_t a, int32_t b) {
				return (a&MASK_VALUE) > (b&MASK_VALUE); });
			m_GameLogic.CheckCardModel(card);
			if (card < savecard)
			{
				bSucceed = true;
				break;
			}
			else
			{
				CardData = m_vecCardData;
				std::shuffle(CardData.begin(), CardData.end(), g);
			}
			if (++nCounter > 2000)
			{
				bSucceed = false;
				card = savecard;
				break;
			}
			
		} while (true);
	}
	return bSucceed;
}

bool zjh_space::logic_cardmgr::GetUserCard(uint32_t playerid,GameCard &card)
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

bool zjh_space::logic_cardmgr::SwitchCard(GameCard &card)
{
	if (m_Card2RobotL.size() > 0)
	{
		card = m_Card2RobotL.back();
		m_Card2RobotL.pop_back();
		return true;
	}
	return false;
}

bool zjh_space::logic_cardmgr::SwitchCard(uint32_t playerid,GameCard &card/*,bool bLarge/ *=false* /*/)
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

bool zjh_space::logic_cardmgr::SmallofBaseCard(GameCard &card)
{
	GameCard base;
	memset(&base, 0, sizeof(base));
	std::array<int32_t, MAX_CARDS_HAND> basecard{ 0x3B,0x05,0x02 };
	std::sort(basecard.begin(), basecard.end());
	base.nCard = basecard;
	m_GameLogic.CheckCardModel(base);
	if (card < base)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool zjh_space::logic_cardmgr::SmallofSingleCard(GameCard &card)
{
	m_GameLogic.CheckCardModel(card);
	if(CARDLEVEL_DANPAI == card.nModel)
	{
		int32_t nfirstData = card.nCard[0];
		if ( (nfirstData &MASK_VALUE) < 0x0E)
		{
			return true;
		}
	}
	return false;
}


bool zjh_space::logic_cardmgr::WashCard_ModelA()
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

bool zjh_space::logic_cardmgr::WashCard_ModelB()
{ // 5个机器人不能使用
	// 计算发牌数量
	int32_t nCount =2*m_nBasePair-1;
//	assert(nCount<=MAX_GAME_CARD);
	if(nCount>MAX_GAME_CARD)
	{
		return false;
	}
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

bool zjh_space::logic_cardmgr::WashCard_ModelC()
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

		if(!m_GameLogic.GetCardByModel(m_vecCardData,findcard,nCardLvl))
		{
			i--;
			continue;
		}
		assert(findcard.size()==MAX_CARDS_HAND);
		for (int j=0;j<MAX_CARDS_HAND;j++)
		{
			card.nCard[j] = findcard[j];
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

bool zjh_space::logic_cardmgr::WashCard_ModelD()
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
		if(!m_GameLogic.GetCardByModel(m_vecCardData,findcard,nCardLvl))
		{
			i--;
			continue;
		}
		assert(findcard.size()==MAX_CARDS_HAND);
		for (int j=0;j<MAX_CARDS_HAND;j++)
		{
			card.nCard[j] = findcard[j];
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


bool zjh_space::logic_cardmgr::WashCard_ModelE()
{
	// 计算发牌数量
	int32_t nCount = m_nBasePair;
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
	return true;
}

bool zjh_space::logic_cardmgr::WashCard_ModelF()
{
	// 计算发牌数量
	int nDelCount = 2;
	int32_t nCount = nDelCount + m_nBasePair;
	assert(nCount <= MAX_GAME_CARD);
	for (int i = 0; i < nCount; i++)
	{
		GameCard card;
		memset(&card, 0, sizeof(card));
		for (int j = 0; j < MAX_CARDS_HAND; j++)
		{
			card.nCard[j] = m_vecCardData.back();
			m_vecCardData.pop_back();
		}
#if 1
		std::sort(card.nCard.begin(), card.nCard.end(), [](int32_t a, int32_t b) {
			return (a&MASK_VALUE) > (b&MASK_VALUE);
		});
#endif
		m_GameLogic.CheckCardModel(card);
		m_GameCardData.push_back(card);
	}

	std::sort(m_GameCardData.begin(), m_GameCardData.end());

	std::reverse(m_GameCardData.begin(), m_GameCardData.end());
	for (int32_t i = 0; i < nDelCount; i++)
	{
		m_GameCardData.pop_back();
	}
	std::reverse(std::begin(m_GameCardData), std::end(m_GameCardData));
	// index
	int32_t nIndex = 0;
	for (auto it = m_GameCardData.begin(); it != m_GameCardData.end(); it++)
	{
		it->nIndex = nIndex++;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool zjh_space::logic_cardmgr::WashCard_ModelAEx()
{
	// 计算发牌数量
	int nDelCount = global_random::instance().rand_int(0, DEL_GAME_CARDEx);
	int32_t nCount =/*DEL_GAME_CARDEx*/nDelCount +m_nBasePair;
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
	for (int32_t i=0;i</*DEL_GAME_CARDEx*/nDelCount;i++)
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

bool zjh_space::logic_cardmgr::WashCard_ModelAEx2()
{
// 	const static int32_t nFront = 4;
// 	const static int32_t nEnd = 4;
	int32_t nFront = global_random::instance().rand_int(1, 4);
	int32_t nEnd = global_random::instance().rand_int(1, 4);
	/*
	* @ 去头去尾 4 
	*/
	// 计算发牌数量
	int32_t nCount = nFront + m_nBasePair + nEnd;
	assert(nCount <= MAX_GAME_CARD);
	if (nCount > MAX_GAME_CARD)
	{
		WashCard_ModelAEx();
		return false;
	}
	for (int i = 0; i < nCount; i++)
	{
		GameCard card;
		memset(&card, 0, sizeof(card));
		for (int j = 0; j < MAX_CARDS_HAND; j++)
		{
			card.nCard[j] = m_vecCardData.back();
			m_vecCardData.pop_back();
		}
#if 1
		std::sort(card.nCard.begin(), card.nCard.end(), [](int32_t a, int32_t b) {
			return (a&MASK_VALUE) > (b&MASK_VALUE);
		});
#endif
		m_GameLogic.CheckCardModel(card);
		m_GameCardData.push_back(card);
	}

	std::sort(m_GameCardData.begin(), m_GameCardData.end());
	//
	for (int32_t i = 0; i < nEnd; i++)
	{
		m_GameCardData.pop_back();
	}
	//
	std::reverse(m_GameCardData.begin(), m_GameCardData.end());
	for (int32_t i = 0; i < nFront; i++)
	{
		m_GameCardData.pop_back();
	}
	std::reverse(std::begin(m_GameCardData), std::end(m_GameCardData));
	// index
	int32_t nIndex = 0;
	for (auto it = m_GameCardData.begin(); it != m_GameCardData.end(); it++)
	{
		it->nIndex = nIndex++;
	}
	return true;
}

