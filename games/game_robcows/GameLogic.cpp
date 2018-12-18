#include "stdafx.h"
#include "GameLogic.h"
#include <random>
#include <algorithm>
#include <chrono>

ROBCOWS_SPACE_USING

BYTE CGameLogic:: S_Cards[TOTAL_CARDS] = {
	// 方块 A-K
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,	
	// 梅花 2-A
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,
	// 红桃 2-A
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,	
	// 黑桃 2-A
	0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,
	// 小 大 王
	0x41,0x42,
};


CGameLogic::CGameLogic(void)
{
	
}

CGameLogic::~CGameLogic(void)
{
}


void robcows_space::CGameLogic::WashPai(Vector_Card &veccard,bool bNeedKing/*=false*/)
{
	veccard.clear();
	for (int i=0;i<TOTAL_CARDS;i++)
	{
		BYTE cbCard = S_Cards[i];
		if(!bNeedKing && GetCardColor(cbCard)==0x40) continue;
		veccard.push_back(cbCard);
	}
#if 0
	unsigned seed = std::chrono::system_clock::now ().time_since_epoch ().count (); 
	std::shuffle(veccard.begin(),veccard.end(),std::default_random_engine(seed));
#else
	std::random_device rd;
	std::mt19937_64 g(rd());
	std::shuffle(std::begin(veccard),std::end(veccard),g);

	std::seed_seq seed2{ rd(),  rd(),  rd(), rd(),  rd(),  rd(),  rd(),  rd() };
	std::mt19937_64 g2(seed2);
	for (int i = 0; i < 8; i++)
	{
		std::shuffle(veccard.begin(), veccard.end(), g2);
	}
#endif
}


BOOL robcows_space::CGameLogic::CheckCardModel(GameCard &card)
{
	INT hua[ZJH_RANGE_VALUE] = {0};
	INT pai[ZJH_RANGE_VALUE] = {0};
	memset(pai,0,sizeof(pai));
	memset(hua,0,sizeof(hua));
	int nIndex = 0 ;
	for (int i=0;i<MAX_CARDS_HAND;i++)
	{
		// 牌的大小编号
		nIndex = (GetCardValue(card.nCard[i])-2)%13;
		++pai[nIndex];
		// 花色
		if(hua[nIndex] < GetCardColor(card.nCard[i]))
		{
			hua[nIndex] = GetCardColor(card.nCard[i]);
		}
	}

	// 特殊的顺子 [A 2 3]
	if (1==pai[12] && 1==pai[0] && 1==pai[1])
	{
		if(hua[12]==hua[0] && hua[0]==hua[1])
		{ // 同花顺
			card.nModel = CARDLEVEL_SHUNJIN;
		}
		else
		{ // 顺子
			card.nModel = CARDLEVEL_SHUNZI;
		}
		card.nCardIdx[0] = 1;
		card.nHua = hua[1];
	}
	INT nOneCard = 0;
	BOOL bHaveDui = FALSE;
	// 牌型计算
	for(int i=ZJH_RANGE_VALUE-1;i>=0;i--)
	{
		if(0==pai[i]) continue;
		switch(pai[i])
		{
		case 3: // 豹子
			nOneCard = 3;
			card.nModel = CARDLEVEL_BAOZI;
			card.nCardIdx[0] = i;
			card.nHua = hua[i];
			break;
		case 2: // 对子
			nOneCard += 2;
			bHaveDui = TRUE;
			card.nModel = CARDLEVEL_DUIZI;
			if (nOneCard==3)
			{
				card.nCardIdx[1] = card.nCardIdx[0];
			}
			else
			{
				assert(nOneCard==2);
			}
			card.nCardIdx[0] = i;
			card.nHua = hua[i];
			break;
		case 1:
			{
				nOneCard += 1;
				int Temp1 = i-1;
				int Temp2 = i-2;
				if (bHaveDui && nOneCard==3)
				{ // 对子
					card.nCardIdx[1] = i;
				}
				else if (nOneCard> 1)
				{ // 遍历时 已经有一张单牌
					if(GetCardColor(card.nCard[0]) == GetCardColor(card.nCard[1])
						&& GetCardColor(card.nCard[1])== GetCardColor(card.nCard[2]))
					{ // 同花
						card.nModel = CARDLEVEL_JINHUA;
					}
					else
					{
						card.nModel = CARDLEVEL_DANPAI;
					}
					card.nCardIdx[nOneCard-1] = i;
				}
				else if (i>=2 && 1==pai[Temp1] && 1==pai[Temp2])
				{ // 顺
					nOneCard = 3;
					if (hua[i]==hua[Temp1] && hua[Temp1] == hua[Temp2])
					{ // 同花顺
						card.nModel = CARDLEVEL_SHUNJIN;
					}
					else
					{ // 顺子
						card.nModel = CARDLEVEL_SHUNZI;
					}
					card.nCardIdx[0] = i;
					card.nHua = hua[i];
				}
				else
				{ // 单牌
					card.nCardIdx[0] = i;
					card.nHua = hua[i];
				}
			}
			break;
		default:
			assert(false);
			break;
		}
		// 判断
		if(nOneCard==MAX_CARDS_HAND)
		{
			assert(card.nModel);
			return TRUE;
		}
		else
		{
			assert(nOneCard<MAX_CARDS_HAND);
		}
	}
	return FALSE;
}


void robcows_space::CGameLogic::SortGameCard(Vec_GameCard &gamecard)
{
	INT nMaxModel = CARDLEVEL_ERROR;
	INT nMaxStool = -1;

	uint32_t nCmpCnt = gamecard.size();
	for(uint32_t i=0;i<nCmpCnt;i++)
	{
		if(gamecard[i].nModel>0 && gamecard[i].nModel > nMaxModel)
		{
			nMaxModel = gamecard[i].nModel;
			nMaxStool = i;
		}
	}
	assert(nMaxModel);
	if(nMaxModel==CARDLEVEL_ERROR)
	{
		assert(0);
	}

	for (uint32_t i=0;i<nCmpCnt;i++)
	{
		if(gamecard[i].nModel < nMaxModel) continue;
		if (i==nMaxStool) continue;
		assert(gamecard[i].nModel == nMaxModel);

		switch(nMaxModel)
		{
		case CARDLEVEL_BAOZI:
		case CARDLEVEL_SHUNJIN:
		case CARDLEVEL_SHUNZI:
			{ // 比较第一张--比较花色
				if(gamecard[i].nCardIdx[0] > gamecard[nMaxStool].nCardIdx[0])
					nMaxStool = i;
				else if (gamecard[i].nCardIdx[0] == gamecard[nMaxStool].nCardIdx[0])
				{
					if(gamecard[i].nHua > gamecard[nMaxStool].nHua)
						nMaxStool = i;
				}
			}
			break;
		case CARDLEVEL_JINHUA:
		case CARDLEVEL_DANPAI:
			{ // 比较第一张
				if(gamecard[i].nCardIdx[0] > gamecard[nMaxStool].nCardIdx[0])
				{//第一张比我大
					nMaxStool = i;
				}
				else if(gamecard[i].nCardIdx[0] < gamecard[nMaxStool].nCardIdx[0])
				{//
				}
				else if(gamecard[i].nCardIdx[1] > gamecard[nMaxStool].nCardIdx[1])
				{//第二张比我大
					nMaxStool = i;
				}
				else if(gamecard[i].nCardIdx[1] < gamecard[nMaxStool].nCardIdx[1])
				{
				}
				else if(gamecard[i].nCardIdx[2] > gamecard[nMaxStool].nCardIdx[2])
				{//第三张比我大
					nMaxStool = i;
				}
				else if(gamecard[i].nCardIdx[2] < gamecard[nMaxStool].nCardIdx[2])
				{
				}
				else if(gamecard[i].nHua > gamecard[nMaxStool].nHua)
				{
					nMaxStool = i;
				}
			}
			break;
		case CARDLEVEL_DUIZI:
			{
				if(gamecard[i].nCardIdx[0] > gamecard[nMaxStool].nCardIdx[0])
				{//第一张比我大
					nMaxStool = i;
				}
				else if(gamecard[i].nCardIdx[0] < gamecard[nMaxStool].nCardIdx[0])
				{
				}
				else if(gamecard[i].nCardIdx[1] > gamecard[nMaxStool].nCardIdx[1])
				{
					nMaxStool = i;
				}
				else if(gamecard[i].nCardIdx[1] < gamecard[nMaxStool].nCardIdx[1])
				{
				}
				else if(gamecard[i].nHua > gamecard[nMaxStool].nHua)
				{
					nMaxStool = i;
				}
			}
			break;
		default:
			assert(FALSE);
			break;

		}
	}
}

uint32_t robcows_space::CGameLogic::GetCardLogicValue(const BYTE bCard)
{
	uint32_t nValue = bCard&MASK_VALUE;
	uint32_t nColor = bCard&MASK_COLOR;
	if(nValue<s_nMaxValue)
	{
		if (nColor==0x40)
		{
			return s_nMaxValue+nValue;
		}
		else
		{
			return nValue;
		}
	}
	return s_nMaxValue;
}


BOOL robcows_space::CGameLogic::GetCard2Cows(Vector_Card &vecCard,Vector_Card &findCard)
{
	/* 随机选3张是否有牛
	*	是  --- 第 4 5 张 随机
	*	否  --- 第 4 张 凑牛  如不可则第5张 否则第5张随机 
	*/
	std::srand(unsigned(std::time(0)));
	
	uint32_t nCardCount = vecCard.size();
	if(nCardCount<1) return FALSE;
	uint32_t nFindCount = 0;
	SLOG_DEBUG<<"CGameLogic::GetCard2Cows CardCount:"<<nCardCount;
	do 
	{
		std::random_shuffle(vecCard.begin(),vecCard.end());
		BYTE card = vecCard.back();
		// remove
		vecCard.pop_back();
		// add
		findCard.push_back(card);
		nFindCount++;

	} while (nFindCount<3);
	int32_t nThreeValue = CalculateCows(findCard);
	if(nThreeValue%10 ==0)
	{
		for (int32_t i=0;i<2;i++)
		{
			std::random_shuffle(vecCard.begin(),vecCard.end());
			BYTE card = vecCard.back();
			// remove
			vecCard.pop_back();
			// add
			findCard.push_back(card);
		}
	}
	else
	{
		// 第4张牌
		int32_t nFourthValue = 0;
		BYTE card = 0;
		bool bFind = false;
		for (auto vec : vecCard)
		{
			card = vec;
			nFourthValue = GetCardLogicValue(card);
			if((nThreeValue+nFourthValue)%10==0)
			{
				bFind = true;
				break;
			}
		}
		if(!bFind)
		{
			std::random_shuffle(vecCard.begin(),vecCard.end());
			card = vecCard.back();
		}
// 		do 
// 		{
// 			std::random_shuffle(vecCard.begin(),vecCard.end());
// 			card = vecCard.back();
// 			nFourthValue = GetCardLogicValue(card);
// 		} while ((nThreeValue+nFourthValue)%10!=0);
		// remove
		RemoveCardByCard(vecCard,card);
		// add
		findCard.push_back(card);

		// 第五张牌
		std::random_shuffle(vecCard.begin(),vecCard.end());
		card = vecCard.back();
		// remove
		vecCard.pop_back();
		// add
		findCard.push_back(card);
	}
	return TRUE;
}

BOOL robcows_space::CGameLogic::GetCard2NoCows(Vector_Card &vecCard,Vector_Card &findCard)
{
	uint32_t nCardCount = vecCard.size();
	uint32_t nFindCount = 0;
	nFindCount = 0;
	do 
	{
		nCardCount = vecCard.size();
		uint32_t nIndex = GetRand(0,nCardCount-1);
		BYTE card = vecCard.at(nIndex);
		// remove
		RemoveCardByCard(vecCard,card);
		// add
		findCard.push_back(card);
		nFindCount++;

	} while (nFindCount<MAX_CARDS_HAND);
	return TRUE;
}

int32_t robcows_space::CGameLogic::CalculateCows(Vector_Card &vecCard)
{
	int32_t nLogicValue = 0;
	for (auto card : vecCard)
	{
		nLogicValue += GetCardLogicValue(card);
	}
	return nLogicValue;
}

BOOL robcows_space::CGameLogic::CheckCardModel_RobCows(GameCard &card)
{
	// 
	std::array<int32_t,MAX_CARDS_HAND> bCard = card.nCard;
	for (int i=0;i<MAX_CARDS_HAND;i++)
	{
		card.nCardIdx[i] = GetCardValue(bCard[i]);
	}
	card.nHua = GetCardColor(bCard[0]);
	// 五小牛
	if(IsWuXiaoNiu(card))
	{
		return TRUE;
	}
	else if(IsWuHuaNiu(card))
	{
		return TRUE;
	}
	else if(IsSiZa(card))
	{
		return TRUE;
	}
	else
	{
		if(IsHasCows(card))
		{ // 有牛
			return TRUE;
		}
		else
		{ // 无牛
			card.nModel = CARD_VALUE_ZERO;
			card.nHua = GetCardColor(bCard[0]);
			card.nCowsCard = bCard;
		}
	}
	
	return FALSE;
}

bool robcows_space::CGameLogic::IsWuXiaoNiu(GameCard &card)
{
	// 五小牛
	std::array<int32_t,MAX_CARDS_HAND> bCard = card.nCard;
	bool bCheckNiu = true;
	uint32_t nSumValue = 0;
	for (int i=0;i<MAX_CARDS_HAND;i++)
	{
		uint32_t nLogicValue = GetCardLogicValue(bCard[i]);
		if(nLogicValue>=5)
		{
			bCheckNiu = false;
			break;
		}
		else
		{
			nSumValue += nLogicValue;
		}
	}
	if(bCheckNiu && nSumValue<10 && nSumValue>0)
	{
		card.nModel = CARD_VALUE_WUXIAO;
		card.nHua = GetCardColor(bCard[0]);
		card.nCowsCard = bCard;
		return true;
	}
	return false;
}

bool robcows_space::CGameLogic::IsWuHuaNiu(GameCard &card)
{
	// 五花牛
	std::array<int32_t,MAX_CARDS_HAND> bCard = card.nCard;
	bool bCheckNiu = true;
	uint32_t nSumValue = 0;
	for (int i=0;i<MAX_CARDS_HAND;i++)
	{
		uint32_t nLogicValue = GetCardLogicValue(bCard[i]);
		uint32_t nValue = GetCardValue(bCard[i]);
		if(nValue<=s_nMaxValue)
		{
			bCheckNiu = false;
			break;
		}
		else
		{
			nSumValue += nLogicValue;
		}
	}
	if(bCheckNiu && nSumValue%10==0 && nSumValue>0)
	{
		card.nModel = CARD_VALUE_WUHUA;
		card.nHua = GetCardColor(bCard[0]);
		card.nCowsCard = bCard;
		return true;
	}
	return false;
}

bool robcows_space::CGameLogic::IsSiZa(GameCard &card)
{
	// 四炸
	std::array<int32_t,MAX_CARDS_HAND> bCard = card.nCard;
	int32_t cbSameCard = 0;
	for (int i=0;i<MAX_CARDS_HAND;i++)
	{
		if(i==1 || i==2 || i==3) continue;
		BYTE cbSameValue = GetCardValue(bCard[i]);
		int32_t cbSameCount = 0;
		for (int j=0;j<MAX_CARDS_HAND;j++)
		{
			uint32_t nValue = GetCardValue(bCard[j]);
			if(cbSameValue==nValue)
			{
				cbSameCard = bCard[j];
				cbSameCount++;
			}
		}
		if(cbSameCount==Counter_Color) break;
		else  cbSameCard = 0;
		
	}
	if (cbSameCard!=0)
	{
		card.nModel = CARD_VALUE_SIZA;
		card.nCardIdx[0] = GetCardValue(cbSameCard);
		card.nHua = GetCardColor(cbSameCard);
		card.nCowsCard = bCard;
		return true;
	}
	return false;
}

bool robcows_space::CGameLogic::Analysis_Cows(GameCard &card)
{
	// 通用算法
	std::array<int32_t,MAX_CARDS_HAND> bCard = card.nCard;
	int bCardCount = MAX_CARDS_HAND;

	int bIndex1 = 0;
	int bIndex2 = 0;

	int bMaxValue1 = 0;
	int bMaxValue2 = 0;
	int bMaxIndex1 = 0;
	int bMaxIndex2 = 0;
	// 分析大小王
	for (int i=0;i<bCardCount;i++)
	{
		uint32_t nLogicV = GetCardLogicValue(bCard[i]);
		if(nLogicV>s_nMaxValue)
		{
			bMaxValue2=bMaxValue1;
			bMaxIndex2=bMaxIndex1;
			bMaxValue1=nLogicV;
			bMaxIndex1=i;
		}
	}
	if(bMaxValue1>s_nMaxValue && bMaxValue2>s_nMaxValue)
	{ // 含有两张王
		int32_t nSum = 0;
		for (int i=0;i<bCardCount;i++)
		{
			nSum += GetCardLogicValue(bCard[i]);
		}
		if((nSum-bMaxValue1-bMaxValue2)%10==0)
		{
			bIndex1 = bMaxIndex1;
			bIndex2 = bMaxIndex2;
		}
		else
		{
			bIndex1 = bMaxIndex1;
			bIndex2 = 0;
			// 随机一张 不是王
			while(bIndex2==bMaxIndex1 || bIndex2==bMaxIndex2)
			{
				bIndex2 = (bIndex2+1)%bCardCount;
			}
		}
		return true;
	}
	else if(bMaxValue1>s_nMaxValue && bMaxValue2<=s_nMaxValue)
	{ // 含有一张王
		// 计算任意三张和为零
		for (int i=0;i<bCardCount;i++)
		{
			if(i==bMaxIndex1) continue;
			for (int j=i+1;j<bCardCount;j++)
			{
				if(j==bMaxIndex1) continue;
				for (int k=j+1;k<bCardCount;k++)
				{
					if(k==bMaxIndex1) continue;
					if((GetCardLogicValue(bCard[i])+
						 GetCardLogicValue(bCard[j])+GetCardLogicValue(bCard[k]))%10==0)
					{
						bIndex1 = bMaxIndex1;
						// 找一张跟王配成牛值
						for (int m=0;m<bCardCount;m++)
						{
							if(m!=i&&m!=j&&m!=k&&m!=bMaxIndex1)
							{
								bIndex2 = m;
								return true;
							}
						}
					}
				}
			}
		}
		// 计算两张和的最大值
		uint32_t nMax = 0;
		for (int i=0;i<bCardCount;i++)
		{
			if(i==bMaxIndex1) continue;
			for (int j=i+1;j<bCardCount;j++)
			{
				if(j==bMaxIndex1) continue;
				uint32_t nTwoValue = GetCardLogicValue(bCard[i])+GetCardLogicValue(bCard[j]);
				nTwoValue = nTwoValue%10;
				if(nTwoValue==0)
				{
					bIndex1 = i;
					bIndex2 = j;
					i = j = bCardCount;
				}
				else if(nTwoValue>nMax)
				{
					nMax = nTwoValue;
					bIndex1 = i;
					bIndex2 = j;
				}
			}
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	uint32_t nThreeValue = 0;
	for (int i=0;i<bCardCount;i++)
	{
		for (int j=i+1;j<bCardCount;j++)
		{
			for(int k=0;k<bCardCount;k++)
			{
				if(k!=i && k!=j)
				{
					nThreeValue += GetCardLogicValue(bCard[k]);
				}
			}
			if(nThreeValue%10==0)
			{
				bIndex1 = i;
				bIndex2 = j;
				return true;
			}
			else
			{
				nThreeValue = 0;
			}
		}
	}
	return false;
}

bool robcows_space::CGameLogic::IsHasCows(GameCard &srccard)
{
	typedef struct __CMPNIU{
		std::array<int32_t,MAX_CARDS_HAND> data;
		int32_t val;
	}_CMPNIU;

	// 排列组合 C(5:3)
	__CMPNIU cmn[10] = {0};

	std::array<int32_t,MAX_CARDS_HAND> bCard = srccard.nCard;
	int bCardCount = MAX_CARDS_HAND;

	int32_t nIdex = 0;
	for (int32_t i=0;i<bCardCount-2;i++)
	{
		int32_t nValue = 0;
		for (int32_t j=i+1;j<bCardCount-1; j++)
		{
			for (int32_t k=j+1;k<bCardCount;k++)
			{
				nValue = GetCardLogicValue(bCard[i])+GetCardLogicValue(bCard[j])+GetCardLogicValue(bCard[k]);
				if((nValue%10) == 0)
				{
					cmn[nIdex].data[0] = bCard[i]; 
					cmn[nIdex].data[1] = bCard[j]; 
					cmn[nIdex].data[2] = bCard[k]; 
					nValue = 0;
					int32_t n = 0;
					for (int32_t m=0;m<bCardCount;m++)
					{
						if(m!=i && m!=j && m!=k)
						{
							nValue += GetCardLogicValue(bCard[m]);
							cmn[nIdex].data[3+n] = bCard[m];
							n++;
						}
					}
					nValue %= 10;
					nValue = (nValue==0?10:nValue);
					cmn[nIdex].val = nValue;
					nIdex++;
				}
			}
		}
	}
	// 有牛 取最大牛
	int nMaxValue = 0;
	int nMaxIdex = -1;
	for (int i=0;i<nIdex;i++)
	{
		if (cmn[i].val > nMaxValue)
		{
			nMaxValue = cmn[i].val;
			nMaxIdex = i;
		}
	}
	if(nMaxIdex!=-1)
	{
		srccard.nModel = cmn[nMaxIdex].val;
		srccard.nCowsCard = cmn[nMaxIdex].data;
		return true;
	}

	return false;
}

BOOL CGameLogic::CanGetCardByModel(Vector_Card &vecCard,const BYTE bModel)
{
	BYTE bDoing = FALSE;
	INT pai[ZJH_RANGE_VALUE] = {0};
	INT hua[ZJH_RANGE_VALUE] = {0};
	int nIndex = 0 ;
	int nHuaIdx = 0;
	int nHuaCount[4] = {0};
	int32_t bCount = vecCard.size();
	for (int i=0;i<bCount;i++)
	{
		BYTE bCardTmp = vecCard.at(i);
		nIndex = (GetCardValue(bCardTmp)-2)%13;
		++pai[nIndex];
		hua[nIndex] = GetCardColor(bCardTmp);
		nHuaIdx = GetCardColor(bCardTmp);
		nHuaCount[nHuaIdx>>4]++;
	}
	// 特殊顺子判断
	BYTE bShunZi = FALSE;
	BYTE bShunJin = FALSE;
	if (pai[12]>=1 && pai[0]>=1 && pai[1]>=1 )
	{
		if(hua[12]==hua[0] && hua[0]==hua[1])
		{ // 同花顺
			bShunJin = TRUE;
			bShunZi = TRUE;
		}
		else
		{ // 顺子
			bShunZi = TRUE;
		}
	}
	if(bModel==CARDLEVEL_DANPAI)
	{
		bDoing = TRUE;
	}
	else if(bModel==CARDLEVEL_DUIZI)
	{
		for(int i=ZJH_RANGE_VALUE-1;i>=0;i--)
		{
			if (pai[i]>=2)
			{
				bDoing = TRUE;
				break;
			}
		}
	}
	else if(bModel==CARDLEVEL_SHUNZI)
	{
		for(int i=ZJH_RANGE_VALUE-1;i>=2;i--)
		{
			if(pai[i]>=1 && pai[i-1]>=1 && pai[i-2]>=1 )
			{
				// 顺子
				bShunZi = TRUE;
				break;
			}
		}
		bDoing = bShunZi;
	}
	else if (bModel==CARDLEVEL_JINHUA)
	{
		for(int i=ZJH_RANGE_VALUE-1;i>=0;i--)
		{
			if (nHuaCount[i]>=3)
			{
				bDoing = TRUE;
				break;
			}
		}
	}
	else if(bModel==CARDLEVEL_SHUNJIN)
	{
		int32_t nfIdx = 0;
		BYTE cardtemp1,cardtemp2,card;
		for ( int j=0;j<bCount;j++)
		{
			BOOL bFirst = FALSE;
			BOOL bSecond = FALSE;
			nfIdx = j;
			card = vecCard.at(j);
			if(GetCardValue(card)==CARDVALUE_MIN)
			{
				// 向后推移两位
				for (int k=0;k<bCount;k++)
				{
					if(!bFirst&&
						GetCardValue(vecCard.at(k))==GetCardValue(vecCard.at(nfIdx))+1
						&& GetCardColor(vecCard.at(k))==GetCardColor(vecCard.at(nfIdx)))
					{
						cardtemp1=vecCard.at(k);
						bFirst = TRUE;
					}
					if(!bSecond&&
						GetCardValue(vecCard.at(k))==GetCardValue(vecCard.at(nfIdx))+2
						&& GetCardColor(vecCard.at(k))==GetCardColor(vecCard.at(nfIdx)))
					{
						cardtemp2 = vecCard.at(k);
						bSecond = TRUE;
					}
				}
			}
			else if(GetCardValue(card)==CARDVALUE_A)
			{
				// 向前推移两位 
				for (int k=0;k<bCount;k++)
				{
					if(!bFirst&&
						GetCardValue(vecCard.at(k))==GetCardValue(vecCard.at(nfIdx))-1
						&& GetCardColor(vecCard.at(k))==GetCardColor(vecCard.at(nfIdx)))
					{
						cardtemp1=vecCard.at(k);
						bFirst = TRUE;
					}
					if(!bSecond&&
						GetCardValue(vecCard.at(k))==GetCardValue(vecCard.at(nfIdx))-2
						&& GetCardColor(vecCard.at(k))==GetCardColor(vecCard.at(nfIdx)))
					{
						cardtemp2=vecCard.at(k);
						bSecond = TRUE;
					}
				}
			}
			else
			{
				// 向前推移一位，向后推移一位
				for (int k=0;k<bCount;k++)
				{
					if(!bFirst&&
						GetCardValue(vecCard.at(k))==GetCardValue(vecCard.at(nfIdx))-1
						&& GetCardColor(vecCard.at(k))==GetCardColor(vecCard.at(nfIdx)))
					{
						cardtemp1=vecCard.at(k);
						bFirst = TRUE;
					}
					if(!bSecond&&
						GetCardValue(vecCard.at(k))==GetCardValue(vecCard.at(nfIdx))+1
						&& GetCardColor(vecCard.at(k))==GetCardColor(vecCard.at(nfIdx)))
					{
						cardtemp2=vecCard.at(k);
						bSecond = TRUE;
					}
				}		
			}
			if(bFirst&&bSecond)
			{
				bShunJin = TRUE;
				break;
			}
		}
		bDoing = bShunJin;
	}
	else if (bModel==CARDLEVEL_BAOZI)
	{
		for(int i=ZJH_RANGE_VALUE-1;i>=0;i--)
		{
			if (pai[i]>=3)
			{
				bDoing = TRUE;
				break;
			}
		}
	}

	return bDoing;

}

BOOL CGameLogic::GetCardByModel(Vector_Card &vecCard,Vector_Card &findCard,const BYTE bModel)
{
	if(CanGetCardByModel(vecCard,bModel)==FALSE) return FALSE;
	uint32_t nCardCount = vecCard.size();
	uint32_t nFindCount = 0;
	SLOG_DEBUG<<"CGameLogic::GetCardByModel CardCount:"<<nCardCount;
	switch (bModel)
	{
	case CARDLEVEL_DANPAI:
		{// 单牌随机----保证牌型----可能是顺子、顺金
			nFindCount = 0;
			BYTE bFindCard = 0;
			do 
			{
				nCardCount = vecCard.size();
				uint32_t nIndex = GetRand(0,nCardCount-1);
				BYTE card = vecCard.at(nIndex);
				if(GetCardValue(card) != GetCardValue(bFindCard))
				{ 
					bFindCard = card;
					// remove
					RemoveCardByCard(vecCard,card);
					// add
					findCard.push_back(card);
					nFindCount++;
				}	
			} while (nFindCount<MAX_CARDS_HAND);
		}
		break;
	case CARDLEVEL_DUIZI:
		{// 对子
			BYTE bFindCard = 0;
			for (int i=0;i<MAX_CARDS_HAND;i++)
			{
				BYTE card = 0;
				// 第一次随机-找对子
				if(i==0)
				{
					nCardCount = vecCard.size();
					uint32_t nIndex = GetRand(0,nCardCount-1);
					card = vecCard.at(nIndex);
					if(GetCardCountByValue(vecCard,card)>=Counter_Dui)
					{
						bFindCard = card;
						// remove
						RemoveCardByCard(vecCard,card);
						// add
						findCard.push_back(card);
					}
					else
					{
						i--;
					}
				}
				else if(i==1)
				{
					card = GetEqualValueByCard(vecCard,bFindCard);
					// remove
					RemoveCardByCard(vecCard,card);
					// add
					findCard.push_back(card);
				}
				else if(i==2)
				{ // 第三张牌 ---保证牌型
					card = GetUnEqualValueByCard(vecCard,bFindCard);
					// remove
					RemoveCardByCard(vecCard,card);
					// add
					findCard.push_back(card);
				}
			}
		}
		break;
	case CARDLEVEL_SHUNZI:
		{// 顺子----保证牌型----可能是顺金
			nFindCount = 0;
			BYTE bFindCard = 0;
			do 
			{
				bool bFirst = false;
				bool bSecond = false;
				nCardCount = vecCard.size();
				uint32_t nIndex = GetRand(0,nCardCount-1);
				BYTE cbCard = vecCard.at(nIndex);
				BYTE cbValue = GetCardValue(cbCard);
				BYTE cbFindCard[MAX_CARDS_HAND] = {0};
				cbFindCard[0] = cbCard;
				if( cbValue == CARDVALUE_2)
				{// 如果是2向后取两个
					for (uint32_t i=0;i<nCardCount;i++)
					{
						BYTE tmpCard = vecCard.at(i);
						if(!bFirst && GetCardValue(tmpCard) == cbValue+1)
						{
							cbFindCard[1] = tmpCard ;
							bFirst = true;
						}
						if(!bSecond && GetCardValue(tmpCard) == cbValue+2)
						{
							cbFindCard[2] = tmpCard ;
							bSecond = true;
						}
					}
				}
				else if(cbValue == CARDVALUE_A)
				{// 如果是 A 向前取两个
					for (uint32_t i=0;i<nCardCount;i++)
					{
						BYTE tmpCard = vecCard.at(i);
						if(!bFirst && GetCardValue(tmpCard) == cbValue-1)
						{
							cbFindCard[1] = tmpCard ;
							bFirst = true;
						}
						if(!bSecond && GetCardValue(tmpCard) == cbValue-2)
						{
							cbFindCard[2] = tmpCard ;
							bSecond = true;
						}
					}
				}
				else
				{
					for (uint32_t i=0;i<nCardCount;i++)
					{
						BYTE tmpCard = vecCard.at(i);
						if(!bFirst && GetCardValue(tmpCard) == cbValue-1)
						{
							cbFindCard[1] = tmpCard ;
							bFirst = true;
						}
						if(!bSecond && GetCardValue(tmpCard) == cbValue+1)
						{
							cbFindCard[2] = tmpCard ;
							bSecond = true;
						}
					}
				}

				// 验证
				if(bFirst && bSecond)
				{
					for (int i = 0; i < MAX_CARDS_HAND; i++)
					{
						BYTE card = cbFindCard[i];
						// remove
						RemoveCardByCard(vecCard,card);
						// add
						findCard.push_back(card);
						nFindCount++;
					}
					break;
				}

			} while (nFindCount<MAX_CARDS_HAND);
		}
		break;
	case  CARDLEVEL_JINHUA:
		{// 金花----保证牌型----可能是顺金
			BYTE bFindCard = 0;
			for (int i=0;i<MAX_CARDS_HAND;i++)
			{
				BYTE card = 0;
				// 第一次随机-
				if(i==0)
				{
					nCardCount = vecCard.size();
					uint32_t nIndex = GetRand(0,nCardCount-1);
					card = vecCard.at(nIndex);
					if(GetCardCountByColor(vecCard,card)>=Counter_Three)
					{
						bFindCard = card;
						// remove
						RemoveCardByCard(vecCard,card);
						// add
						findCard.push_back(card);
					}
					else
					{
						i--;
					}
				}
				else if(i==1)
				{
					card = GetEqualColorByCard(vecCard,bFindCard);
					// remove
					RemoveCardByCard(vecCard,card);
					// add
					findCard.push_back(card);
				}
				else if(i==2)
				{ // 第三张牌
					card = GetEqualColorByCard(vecCard,bFindCard);
					// remove
					RemoveCardByCard(vecCard,card);
					// add
					findCard.push_back(card);
				}
			}
		}
		break;
	case CARDLEVEL_SHUNJIN:
		{// 顺金
			nFindCount = 0;
			BYTE bFindCard = 0;
			do 
			{
				bool bFirst = false;
				bool bSecond = false;
				nCardCount = vecCard.size();
				uint32_t nIndex = GetRand(0,nCardCount-1);
				BYTE cbCard = vecCard.at(nIndex);
				BYTE cbValue = GetCardValue(cbCard);
				BYTE cbColor = GetCardColor(cbCard);
				BYTE cbFindCard[MAX_CARDS_HAND] = {0};
				cbFindCard[0] = cbCard;
				if( cbValue == CARDVALUE_2)
				{// 如果是2向后取两个
					for (uint32_t i=0;i<nCardCount;i++)
					{
						BYTE tmpCard = vecCard.at(i);
						if(!bFirst && GetCardValue(tmpCard) == cbValue+1 &&
							GetCardColor(tmpCard)==cbColor)
						{
							cbFindCard[1] = tmpCard ;
							bFirst = true;
						}
						if(!bSecond && GetCardValue(tmpCard) == cbValue+2 &&
							GetCardColor(tmpCard)==cbColor)
						{
							cbFindCard[2] = tmpCard ;
							bSecond = true;
						}
					}
				}
				else if(cbValue == CARDVALUE_A)
				{// 如果是 A 向前取两个
					for (uint32_t i=0;i<nCardCount;i++)
					{
						BYTE tmpCard = vecCard.at(i);
						if(!bFirst && GetCardValue(tmpCard) == cbValue-1 &&
							GetCardColor(tmpCard)==cbColor)
						{
							cbFindCard[1] = tmpCard ;
							bFirst = true;
						}
						if(!bSecond && GetCardValue(tmpCard) == cbValue-2 &&
							GetCardColor(tmpCard)==cbColor)
						{
							cbFindCard[2] = tmpCard ;
							bSecond = true;
						}
					}
				}
				else
				{
					for (uint32_t i=0;i<nCardCount;i++)
					{
						BYTE tmpCard = vecCard.at(i);
						if(!bFirst && GetCardValue(tmpCard) == cbValue-1 &&
							GetCardColor(tmpCard)==cbColor)
						{
							cbFindCard[1] = tmpCard ;
							bFirst = true;
						}
						if(!bSecond && GetCardValue(tmpCard) == cbValue+1 &&
							GetCardColor(tmpCard)==cbColor)
						{
							cbFindCard[2] = tmpCard ;
							bSecond = true;
						}
					}
				}

				// 验证
				if(bFirst && bSecond)
				{
					for (int i = 0; i < MAX_CARDS_HAND; i++)
					{
						BYTE card = cbFindCard[i];
						// remove
						RemoveCardByCard(vecCard,card);
						// add
						findCard.push_back(card);
						nFindCount++;
					}
					break;
				}

			} while (nFindCount<MAX_CARDS_HAND);
		}
		break;
	case CARDLEVEL_BAOZI:
		{// 豹子
			BYTE bFindCard = 0;
			for (int i=0;i<MAX_CARDS_HAND;i++)
			{
				BYTE card = 0;
				// 第一次随机-找对子
				if(i==0)
				{
					nCardCount = vecCard.size();
					uint32_t nIndex = GetRand(0,nCardCount-1);
					card = vecCard.at(nIndex);
					if(GetCardCountByValue(vecCard,card)>=Counter_Three)
					{
						bFindCard = card;
						// remove
						RemoveCardByCard(vecCard,card);
						// add
						findCard.push_back(card);
					}
					else
					{
						i--;
					}
				}
				else if(i==1)
				{
					card = GetEqualValueByCard(vecCard,bFindCard);
					// remove
					RemoveCardByCard(vecCard,card);
					// add
					findCard.push_back(card);
				}
				else if(i==2)
				{ // 第三张牌 ---保证牌型
					card = GetEqualValueByCard(vecCard,bFindCard);
					// remove
					RemoveCardByCard(vecCard,card);
					// add
					findCard.push_back(card);
				}
			}
		}
		break;
	default:
		break;
	}

	return TRUE;
}

uint32_t CGameLogic::GetRand(uint32_t nRandS,uint32_t nRandE)
{
	return global_random::instance().rand_int(nRandS,nRandE);
}

uint32_t CGameLogic::GetRand_100()
{
	return global_random::instance().rand_100();
}

uint32_t CGameLogic::GetCardCountByValue(Vector_Card &vecCard,const BYTE bCard)
{
	uint32_t nCount = 0;
	for (int i = 0; i < vecCard.size(); i++)
	{
		if(GetCardValue(bCard) == GetCardValue(vecCard[i]))
			nCount++;
	}
	return nCount;
}

uint32_t CGameLogic::GetCardCountByColor(Vector_Card &vecCard,const BYTE bCard)
{
	uint32_t nCount = 0;
	for (int i = 0; i < vecCard.size(); i++)
	{
		if(GetCardColor(bCard) == GetCardColor(vecCard[i]))
			nCount++;
	}
	return nCount;
}

void CGameLogic::RemoveCardByCard(Vector_Card &vecCard,const BYTE bCard)
{
	uint32_t nCardCount = vecCard.size();
	for (uint32_t i = 0; i < nCardCount; i++)
	{
		if (vecCard[i] == bCard)
		{
			vecCard[i] = vecCard[nCardCount-1];
			vecCard.pop_back();
			break;
		}
	}
}

BYTE CGameLogic::GetEqualValueByCard(Vector_Card &vecCard,const BYTE bCard)
{
	BYTE cbCard = 0;
	for (int i=0;i<vecCard.size();i++)
	{
		if(vecCard[i]!=bCard &&
			GetCardValue(vecCard[i])==GetCardValue(bCard))
		{
			cbCard = vecCard[i];
			break;
		}
	}
	return cbCard;
}

BYTE CGameLogic::GetUnEqualValueByCard(Vector_Card &vecCard,const BYTE bCard)
{
	BYTE cbCard = 0;
	for (int i=0;i<vecCard.size();i++)
	{
		if(GetCardValue(vecCard[i])!=GetCardValue(bCard))
		{
			cbCard = vecCard[i];
			break;
		}
	}
	return cbCard;
}

BYTE CGameLogic::GetEqualColorByCard(Vector_Card &vecCard,const BYTE bCard)
{
	BYTE cbCard = 0;
	for (int i=0;i<vecCard.size();i++)
	{
		if(vecCard[i]!=bCard &&
			GetCardColor(vecCard[i])==GetCardColor(bCard))
		{
			cbCard = vecCard[i];
			break;
		}
	}
	return cbCard;
}


