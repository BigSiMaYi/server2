#pragma once
#include "logic_def.h"

DRAGON_TIGER_SPACE_BEGIN

#define POKERCOUNT 52
#define POKERTYPECOUNT 13
#define CARDSPOKERCOUNT 5
#define OTHERCOUNT 5
#define TOTAL_POKES_COUNT 5  //�ܹ��帱��
	//������
enum class cards_type
{
	cards_cow_0,	//û��ţ
	cards_cow_1,	//ţ1 
	cards_cow_2,	//ţ2
	cards_cow_3,	//ţ3
	cards_cow_4,	//ţ4
	cards_cow_5,	//ţ5
	cards_cow_6,	//ţ6
	cards_cow_7,	//ţ7	
	cards_cow_8,	//ţ8	
	cards_cow_9,	//ţ9
	cards_cow_10,	//ţţ
	cards_silver_cow,		//��ţ
	cards_bomb,				//ը��
	cards_gold_cow,			//��ţ
	cards_small_cow,		//��Сţ	
};

enum class poker_type
{
	poker_diamond,		//����
	poker_club,			//÷��
	poker_hearts,		//����
	poker_spade,		//����
};

struct poker
{
	int m_id;
	//��ɫ
	poker_type m_poker_type;
	//�˿˵���1��2��3��4��5��6��7��8��9��10��j,q,k
	int m_poker_point;
	//��С
	int m_poker_value;
	//����
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



DRAGON_TIGER_SPACE_END
