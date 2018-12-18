#pragma once

#include "stdafx.h"

typedef struct _POKER{
	char value,style;
	inline bool operator==(const _POKER&a){
		return value==a.value&&style==a.style;
	}
}POKER;

typedef std::vector<POKER> VECPOKER;

enum{
	POKER_STYLE_DIAMONDS,
	POKER_STYLE_CLUBS,
	POKER_STYLE_HEARTS,
	POKER_STYLE_SPADES,
	POKER_STYLE_EX,
};

enum{
	POKER_VALUE_A=1,
	POKER_VALUE_2,
	POKER_VALUE_3,
	POKER_VALUE_4,
	POKER_VALUE_5,
	POKER_VALUE_6,
	POKER_VALUE_7,
	POKER_VALUE_8,
	POKER_VALUE_9,
	POKER_VALUE_10,
	POKER_VALUE_J,
	POKER_VALUE_Q,
	POKER_VALUE_K,

	POKER_VALUE_JOKER_SMALL=1,
	POKER_VALUE_JOKER_LARGE,
	POKER_VALUE_BACK_0,
};
#define GPOKERS 54
extern POKER g_pokers[GPOKERS];

//
#define MAXPOKERDI                  3
#define MAXPOKER                    20
#define MAXPOKER1                   17

enum{
	GAME_JIAO_NO,
	GAME_JIAO_1,
	GAME_JIAO_2,
	GAME_JIAO_3,
};//char

enum{
	GAME_CHUN_NO,
	GAME_CHUN_YES,
	GAME_CHUN_FAN,
};

enum{
	GAME_STATE_NORMAL,
	GAME_STATE_JIAO,
	//GAME_STATE_MING,
	GAME_STATE_PAI,
	GAME_STATE_RESULT,
	GAME_STATE_END=GAME_STATE_NORMAL,
};

//////////////////////////////////////////////////////////////////////////
enum{
	STYLE_NO,
	STYLE_DAN,
	STYLE_DUI,
	STYLE_3TIAO,
	STYLE_3TIAO1,//+1
	STYLE_3TIAO2,//+dui
	STYLE_SHUN, //
	STYLE_LIANDUI,
	STYLE_FEIJI,
	STYLE_FEIJI1,//+1
	STYLE_FEIJI2,//+1+1 or dui
	STYLE_BOMB11,//+1 or +1+1
	STYLE_BOMB2,//+2dui
	STYLE_BOMB,
	STYLE_BOMBX,
};