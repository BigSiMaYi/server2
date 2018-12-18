#pragma once

#include "card_def.h"

ROBCOWS_SPACE_BEGIN


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

#define TOTAL_CARDS			54

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

	const static uint32_t s_nMaxValue = 10;
public:
	void WashPai(Vector_Card &veccard,bool bNeedKing=false);

public:
	BOOL CheckCardModel(GameCard &card);
	
	void SortGameCard(Vec_GameCard &gamecard);

	// RobCows
	uint32_t GetCardLogicValue(const BYTE bCard);

	BOOL GetCard2Cows(Vector_Card &vecCard,Vector_Card &findCard);

	BOOL GetCard2NoCows(Vector_Card &vecCard,Vector_Card &findCard);

	int32_t CalculateCows(Vector_Card &vecCard);
	
	BOOL CheckCardModel_RobCows(GameCard &card);
	bool IsWuXiaoNiu(GameCard &card);
	bool IsWuHuaNiu(GameCard &card);	
	bool IsSiZa(GameCard &card);

	bool Analysis_Cows(GameCard &card);

	bool IsHasCows(GameCard &srccard);
	//////////////////////////////////////////////////////////////////////////

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

ROBCOWS_SPACE_END