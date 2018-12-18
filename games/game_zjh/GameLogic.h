#pragma once

#include "card_def.h"

ZJH_SPACE_BEGIN


typedef struct _CARDLEVEL_
{
	int nModel;
	std::array<int,MAX_CARDS_HAND> nCard;
	int	nHua;
	char bCheck;
}CARDLEVEL;

#define Counter_Color		4
#define Counter_Single		1
#define Counter_Dui			2
#define Counter_Three		3

#define TOTAL_CARDS			52
#define TOTAL_CARDS_EX		40

#define	MASK_COLOR			0xF0		//花色掩码
#define	MASK_VALUE			0x0F		//数值掩码


class CGameLogic
{
public:
	CGameLogic(void);

	virtual ~CGameLogic(void);
private:
	static BYTE S_Cards[TOTAL_CARDS];

public:

public:
	void WashPai(Vector_Card &veccard);
	// 获取牌型
	BOOL CheckCardsModel(const BYTE* pCard,CARDLEVEL & CardLevel);

	BOOL CheckCardModel(GameCard &card);
	// 牌型比较
	int	CmpMostCard(const CARDLEVEL*pCardLevel,int nCmpCnt);

	void SortGameCard(Vec_GameCard &gamecard);

	void SortCard(BYTE bCardData[],BYTE bCardCount);
	void SortCard(BYTE bDesCardData[],const BYTE bSrcCardData[],BYTE bCardCount);
	//
	BOOL CanGetCardByModel(Vector_Card &vecCard,const BYTE bModel);

	BOOL GetCardByModel(Vector_Card &vecCard,Vector_Card &findCard,const BYTE bModel);

	uint32_t GetRand(uint32_t nRandS,uint32_t nRandE);
	uint32_t GetRand_100();
	uint32_t GetCardCountByValue(Vector_Card &vecCard,const BYTE bCard);

	uint32_t GetCardCountByColor(Vector_Card &vecCard,const BYTE bCard);

	void RemoveCardByCard(Vector_Card &vecCard,const BYTE bCard);

	BYTE GetEqualValueByCard(Vector_Card &vecCard,const BYTE bCard);

	BYTE GetUnEqualValueByCard(Vector_Card &vecCard,const BYTE bCard);

	BYTE GetEqualColorByCard(Vector_Card &vecCard,const BYTE bCard);

public:
	BYTE GetCardValue(BYTE bCard) { return bCard&MASK_VALUE; }
	BYTE GetCardColor(BYTE bCard) { return bCard&MASK_COLOR; }

};

ZJH_SPACE_END