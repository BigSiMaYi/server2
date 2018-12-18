#pragma once
ROBCOWS_SPACE_BEGIN

#include "logic_def.h"


typedef enum 
{
	CARDVALUE_MIN = 2,
	CARDVALUE_2 = CARDVALUE_MIN,
	CARDVALUE_3,
	CARDVALUE_4,
	CARDVALUE_5,
	CARDVALUE_6,
	CARDVALUE_7,
	CARDVALUE_8,
	CARDVALUE_9,
	CARDVALUE_10,
	CARDVALUE_J,
	CARDVALUE_Q,
	CARDVALUE_K,
	CARDVALUE_A,
}CARDVALUE;

#define ZJH_RANGE_VALUE		(CARDVALUE_A-CARDVALUE_MIN+1)

//enum class eCardLevel:UCHAR
enum 
{
	CARDLEVEL_ERROR = 0,	//������û��
	CARDLEVEL_DANPAI,		//����
	CARDLEVEL_DUIZI,		//����
	CARDLEVEL_SHUNZI,		//˳��
	CARDLEVEL_JINHUA,		//��
	CARDLEVEL_SHUNJIN,		//˳��
	CARDLEVEL_BAOZI,		//����

};

#define	CARD_VALUE_WUXIAO				13
#define CARD_VALUE_WUHUA				12
#define CARD_VALUE_SIZA					11
#define CARD_VALUE_NIUNIU				10
#define CARD_VALUE_NINE					9
#define CARD_VALUE_EIGHT				8
#define CARD_VALUE_SEVEN				7
#define CARD_VALUE_SIX					6
#define CARD_VALUE_FIVE					5
#define CARD_VALUE_FOUR					4
#define CARD_VALUE_THREE				3
#define CARD_VALUE_TWO					2
#define CARD_VALUE_ONE					1
#define CARD_VALUE_ZERO					0

typedef std::vector<BYTE> Vector_Card;
typedef std::vector<BYTE>::iterator Vector_CardIterator;

#define MAX_GAME_CARD	17

#define DEL_GAME_CARD	7

#define	GET_GAME_CARD	10

#define ROBOT_GAME_CARD	2

typedef struct _GAMECARD_
{
	// ����
	int32_t nModel;
	// ��ֵ
	std::array<int32_t,MAX_CARDS_HAND> nCard;
	//
	std::array<int32_t,MAX_CARDS_HAND> nCardIdx;
	// ����ţֵ��
	std::array<int32_t,MAX_CARDS_HAND> nCowsCard;
	// ��һ���ƵĻ�ɫ
	int32_t	nHua;
	// �Ƿ��Ѿ���ʹ��
	bool bUseing;
	// 
	char bCheck;
	// ����֮������к�
	int32_t	nIndex;

	bool operator <(const _GAMECARD_ & other) 
	{
		if(nModel > other.nModel)
		{
			return false;
		}
		else if(other.nModel==nModel)
		{
			if(nCardIdx[0] > other.nCardIdx[0])
			{
				return false;
			}
			else if (nCardIdx[0]==other.nCardIdx[0])
			{ 
				if(nHua > other.nHua)
					return false;
			}
		}
		return true;
	}
}GameCard;

typedef std::vector<GameCard> Vec_GameCard;

typedef std::vector<uint32_t> Vec_UserID;

typedef std::map<uint32_t,int32_t> Map_PlayerLabel;
typedef std::pair<uint32_t,int32_t> PAIR;
struct  CmpByValue{
	bool operator()(const PAIR& lhs, const PAIR& rhs) {
		return lhs.second < rhs.second;
	}
};

typedef std::map<uint32_t,Vec_GameCard> Map_PlayerCard;


ROBCOWS_SPACE_END