#pragma once
#include "logic_def.h"

DRAGON_RED_BLACK_BEGIN

#define POKERCOUNT 52
#define POKERTYPECOUNT 13
#define CARDSPOKERCOUNT 5
#define OTHERCOUNT 3

//牌型大小顺序:豹子>顺金>金花>顺子>对子>单张
enum class cards_golden_type
{
	
	cards_golden_unknown,  //未知的牌
	cards_golden_danzhang, //单张
	cards_golden_duizi,		    //对子 
	cards_golden_shunzi,		//顺子
	cards_golden_jinhua,		//金花
	cards_golden_shunjin,		//顺金
	cards_golden_baozi,		//豹子

};

enum win_camp
{
	win_unknow = 0, //未知
	win_black, //黑方赢
	win_luck, 
	win_red, //红方赢
};

enum class poker_type
{
	poker_fangkuai,		//方块
	poker_meihua,			//梅花
	poker_hongtao,		//红桃
	poker_heitao,		    //黑桃	
};

struct poker
{
	int m_id;
	//花色
	poker_type m_poker_type;
	//扑克点数1，2，3，4，5，6，7，8，9，10，j,q,k
	int m_poker_point;

	poker()
		:m_poker_type(poker_type::poker_fangkuai)
		,m_poker_point(0)
	{

	}
	poker(poker_type type, int point)
		:m_poker_type(type)
		,m_poker_point(point)
	{
		m_id = POKERTYPECOUNT * (int)m_poker_type + m_poker_point;
	}
};



DRAGON_RED_BLACK_END
