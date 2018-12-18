#include "stdafx.h"
#include "GameLogic.h"
#include <random>
#include <algorithm>
#include <chrono>

ZJH_SPACE_USING

BYTE CGameLogic:: S_Cards[TOTAL_CARDS] = {
	// 方块 2-A
	0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,	
	// 梅花 2-A
	0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,
	// 红桃 2-A
	0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,	
	// 黑桃 2-A
	0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,	
};


CGameLogic::CGameLogic(void)
{
	std::srand(unsigned(std::time(0)));
}

CGameLogic::~CGameLogic(void)
{
}


void zjh_space::CGameLogic::WashPai(Vector_Card &veccard)
{

	std::random_device rd;
	std::mt19937_64 g(rd());
	std::uniform_int_distribution<> dis(1, 10000);

	//混乱准备
	BYTE bCardCount = TOTAL_CARDS;
	BYTE cbCardData[TOTAL_CARDS];
	BYTE cbCardBuffer[TOTAL_CARDS];
	memcpy(cbCardData, S_Cards, TOTAL_CARDS);

	//混乱扑克
	BYTE cbRandCount = 0, cbPosition = 0;
	do
	{
		cbPosition = dis(g) % (bCardCount - cbRandCount);
		cbCardBuffer[cbRandCount++] = cbCardData[cbPosition];
		cbCardData[cbPosition] = cbCardData[bCardCount - cbRandCount];
	} while (cbRandCount<TOTAL_CARDS);

	memcpy(S_Cards, cbCardBuffer, TOTAL_CARDS);

	veccard.clear();
	int32_t nAllCount = TOTAL_CARDS;
	for (int i=0;i<TOTAL_CARDS;i++)
	{

		veccard.push_back(S_Cards[i]);
	}

	unsigned seed = std::chrono::system_clock::now ().time_since_epoch ().count (); 
	std::shuffle(veccard.begin(),veccard.end(),std::default_random_engine(seed));


	std::seed_seq seed2{ rd(),  rd(),  rd(), rd(),  rd(),  rd(),  rd(),  rd() };
	std::mt19937_64 g2(seed2);
	for (int i = 0; i < 8; i++)
	{
		std::shuffle(veccard.begin(), veccard.end(), g2);
	}
	

}


BOOL CGameLogic::CheckCardsModel( const BYTE* pCard,CARDLEVEL & CardLevel )
{
	if(pCard==NULL) return FALSE;
	memset(&CardLevel, 0, sizeof(CARDLEVEL));
	INT hua[ZJH_RANGE_VALUE] = {0};
	INT pai[ZJH_RANGE_VALUE] = {0};
	memset(pai,0,sizeof(pai));
	memset(hua,0,sizeof(hua));
	int nIndex = 0 ;
	for (int i=0;i<MAX_CARDS_HAND;i++)
	{
		// 牌的大小编号
		nIndex = (GetCardValue(pCard[i])-2)%13;
		++pai[nIndex];
		// 花色
		if(hua[nIndex] < GetCardColor(pCard[i]))
		{
			hua[nIndex] = GetCardColor(pCard[i]);
		}
	}

	// 特殊的顺子 [A 2 3]
	if (1==pai[12] && 1==pai[0] && 1==pai[1])
	{
		if(hua[12]==hua[0] && hua[0]==hua[1])
		{ // 同花顺
			CardLevel.nModel = CARDLEVEL_SHUNJIN;
		}
		else
		{ // 顺子
			CardLevel.nModel = CARDLEVEL_SHUNZI;
		}
		CardLevel.nCard[0] = 1;
		CardLevel.nHua = hua[1];
		return TRUE;
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
			CardLevel.nModel = CARDLEVEL_BAOZI;
			CardLevel.nCard[0] = i;
			CardLevel.nHua = hua[i];
			break;
		case 2: // 对子
			nOneCard += 2;
			bHaveDui = TRUE;
			CardLevel.nModel = CARDLEVEL_DUIZI;
			if (nOneCard==3)
			{
				CardLevel.nCard[1] = CardLevel.nCard[0];
			}
			else
			{
				assert(nOneCard==2);
			}
			CardLevel.nCard[0] = i;
			CardLevel.nHua = hua[i];
			break;
		case 1:
			{
				nOneCard += 1;
				int Temp1 = i-1;
				int Temp2 = i-2;
				if (bHaveDui && nOneCard==3)
				{ // 对子
					CardLevel.nCard[1] = i;
				}
				else if (nOneCard> 1)
				{ // 遍历时 已经有一张单牌
					if(GetCardColor(pCard[0]) == GetCardColor(pCard[1])
						&& GetCardColor(pCard[1])== GetCardColor(pCard[2]))
					{ // 同花
						CardLevel.nModel = CARDLEVEL_JINHUA;
					}
					else
					{
						CardLevel.nModel = CARDLEVEL_DANPAI;
					}
					CardLevel.nCard[nOneCard-1] = i;
				}
				else if (i>=2 && 1==pai[Temp1] && 1==pai[Temp2])
				{ // 顺
					nOneCard = 3;
					if (hua[i]==hua[Temp1] && hua[Temp1] == hua[Temp2])
					{ // 同花顺
						CardLevel.nModel = CARDLEVEL_SHUNJIN;
					}
					else
					{ // 顺子
						CardLevel.nModel = CARDLEVEL_SHUNZI;
					}
					CardLevel.nCard[0] = i;
					CardLevel.nHua = hua[i];
				}
				else
				{ // 单牌
					CardLevel.nCard[0] = i;
					CardLevel.nHua = hua[i];
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
			assert(CardLevel.nModel);
			return TRUE;
		}
		else
		{
			assert(nOneCard<MAX_CARDS_HAND);
		}

	}
	return FALSE;
}

BOOL zjh_space::CGameLogic::CheckCardModel(GameCard &card)
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

		return TRUE;
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

int CGameLogic::CmpMostCard( const CARDLEVEL*pCardLevel,int nCmpCnt )
{
	INT nMaxModel = CARDLEVEL_ERROR;
	INT nMaxStool = -1;

	for(int i=0;i<nCmpCnt;i++)
	{
		if(pCardLevel[i].nModel>0 && pCardLevel[i].nModel > nMaxModel)
		{
			nMaxModel = pCardLevel[i].nModel;
			nMaxStool = i;
		}
	}
	assert(nMaxModel);
	if(nMaxModel==CARDLEVEL_ERROR)
	{
		return INVALID_CHAIR;
	}

	for (int i=0;i<nCmpCnt;i++)
	{
		if(pCardLevel[i].nModel < nMaxModel) continue;
		if (i==nMaxStool) continue;
		assert(pCardLevel[i].nModel == nMaxModel);

		switch(nMaxModel)
		{
		case CARDLEVEL_BAOZI:
		case CARDLEVEL_SHUNJIN:
		case CARDLEVEL_SHUNZI:
			{ // 比较第一张--比较花色
				if(pCardLevel[i].nCard[0] > pCardLevel[nMaxStool].nCard[0])
					nMaxStool = i;
				else if (pCardLevel[i].nCard[0] == pCardLevel[nMaxStool].nCard[0])
				{
					if(pCardLevel[i].nHua > pCardLevel[nMaxStool].nHua)
						nMaxStool = i;
				}
			}
			break;
		case CARDLEVEL_JINHUA:
		case CARDLEVEL_DANPAI:
			{ // 比较第一张
				if(pCardLevel[i].nCard[0] > pCardLevel[nMaxStool].nCard[0])
				{//第一张比我大
					nMaxStool = i;
				}
				else if(pCardLevel[i].nCard[0] < pCardLevel[nMaxStool].nCard[0])
				{//
				}
				else if(pCardLevel[i].nCard[1] > pCardLevel[nMaxStool].nCard[1])
				{//第二张比我大
					nMaxStool = i;
				}
				else if(pCardLevel[i].nCard[1] < pCardLevel[nMaxStool].nCard[1])
				{
				}
				else if(pCardLevel[i].nCard[2] > pCardLevel[nMaxStool].nCard[2])
				{//第三张比我大
					nMaxStool = i;
				}
				else if(pCardLevel[i].nCard[2] < pCardLevel[nMaxStool].nCard[2])
				{
				}
				else if(pCardLevel[i].nHua > pCardLevel[nMaxStool].nHua)
				{
					nMaxStool = i;
				}
			}
			break;
		case CARDLEVEL_DUIZI:
			{
				if(pCardLevel[i].nCard[0] > pCardLevel[nMaxStool].nCard[0])
				{//第一张比我大
					nMaxStool = i;
				}
				else if(pCardLevel[i].nCard[0] < pCardLevel[nMaxStool].nCard[0])
				{
				}
				else if(pCardLevel[i].nCard[1] > pCardLevel[nMaxStool].nCard[1])
				{
					nMaxStool = i;
				}
				else if(pCardLevel[i].nCard[1] < pCardLevel[nMaxStool].nCard[1])
				{
				}
				else if(pCardLevel[i].nHua > pCardLevel[nMaxStool].nHua)
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
	return nMaxStool;

}

void zjh_space::CGameLogic::SortGameCard(Vec_GameCard &gamecard)
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

void CGameLogic::SortCard( BYTE bCardData[],BYTE bCardCount )
{
	bool bSorted=true;
	BYTE bTempData,bLast=bCardCount-1;
	do
	{
		bSorted=true;
		for (BYTE i=0;i<bLast;i++)
		{
			if (GetCardValue(bCardData[i])<GetCardValue(bCardData[i+1])
				||((GetCardValue(bCardData[i])==GetCardValue(bCardData[i+1]))
				&&(GetCardColor(bCardData[i])<GetCardColor(bCardData[i+1]))))
			{
				bTempData=bCardData[i];
				bCardData[i]=bCardData[i+1];
				bCardData[i+1]=bTempData;
				bSorted=false;
			}	
		}
		bLast--;
	} while(bSorted==false);

}

void CGameLogic::SortCard( BYTE bDesCardData[],const BYTE bSrcCardData[],BYTE bCardCount )
{
	bool bSorted=true;
	BYTE bTempData,bLast=bCardCount-1;
	memcpy(bDesCardData,bSrcCardData,bCardCount);
	do
	{
		bSorted=true;
		for (BYTE i=0;i<bLast;i++)
		{
			if (GetCardValue(bDesCardData[i])<GetCardValue(bDesCardData[i+1])
				||((GetCardValue(bDesCardData[i])==GetCardValue(bDesCardData[i+1]))
				&&(GetCardColor(bDesCardData[i])<GetCardColor(bDesCardData[i+1]))))
			{
				bTempData=bDesCardData[i];
				bDesCardData[i]=bDesCardData[i+1];
				bDesCardData[i+1]=bTempData;
				bSorted=false;
			}	
		}
		bLast--;
	} while(bSorted==false);
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


