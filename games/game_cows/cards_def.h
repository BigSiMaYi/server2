#pragma once
#include "logic_def.h"

COWS_SPACE_BEGIN

#define POKERCOUNT 52
#define POKERTYPECOUNT 13
#define CARDSPOKERCOUNT 5
#define OTHERCOUNT 4
	//牌类型
enum class cards_type
{
	cards_cow_0,	//没有牛
	cards_cow_1,	//牛1 
	cards_cow_2,	//牛2
	cards_cow_3,	//牛3
	cards_cow_4,	//牛4
	cards_cow_5,	//牛5
	cards_cow_6,	//牛6
	cards_cow_7,	//牛7	
	cards_cow_8,	//牛8	
	cards_cow_9,	//牛9
	cards_cow_10,	//牛牛
	cards_silver_cow,		//银牛
	cards_bomb,				//炸弹
	cards_gold_cow,			//金牛
	cards_small_cow,		//五小牛	
};

enum class poker_type
{
	poker_diamond,		//方块
	poker_club,			//梅花
	poker_hearts,		//红桃
	poker_spade,		//黑桃
};

struct poker
{
	int m_id;
	//花色
	poker_type m_poker_type;
	//扑克点数1，2，3，4，5，6，7，8，9，10，j,q,k
	int m_poker_point;
	//大小
	int m_poker_value;
	//分数
	int m_poker_score;

	poker()
		:m_poker_type(poker_type::poker_diamond)
		,m_poker_point(0)
		,m_poker_value(0)
	{

	}

	poker(poker_type type, int point)
		:m_poker_type(type)
		,m_poker_point(point)
	{
		m_id = POKERTYPECOUNT * (int)m_poker_type + m_poker_point;
		m_poker_value = m_poker_point * 10 + (int)m_poker_type;
		if (point >= 1 && point <= 9)
		{
			m_poker_score = point;
		}
		else
		{
			m_poker_score = 10;
		}
	}
};

//转换牌的格式为16进制: (0-10-j-q-k-a)
static char Covert(poker& pk)
{
	char color = (char)pk.m_poker_type;
	
	char val = (char)pk.m_poker_point;
	if (pk.m_poker_point==10)
	{
		val = 0x0A;
	}
	else if (pk.m_poker_point == 11)
	{
		val = 0x0B;
	}
	if (pk.m_poker_point == 12)
	{
		val = 0x0C;
	}
	if (pk.m_poker_point == 13)
	{
		val = 0x0D;
	}
	if (pk.m_poker_point == 1)
	{
		val = 0x0E;
	}

	char color0 = color << 4;
	char card = color0 + val;

	return card;
}

COWS_SPACE_END
