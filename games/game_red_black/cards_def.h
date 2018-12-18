#pragma once
#include "logic_def.h"

DRAGON_RED_BLACK_BEGIN

#define POKERCOUNT 52
#define POKERTYPECOUNT 13
#define CARDSPOKERCOUNT 5
#define OTHERCOUNT 3

//���ʹ�С˳��:����>˳��>��>˳��>����>����
enum class cards_golden_type
{
	
	cards_golden_unknown,  //δ֪����
	cards_golden_danzhang, //����
	cards_golden_duizi,		    //���� 
	cards_golden_shunzi,		//˳��
	cards_golden_jinhua,		//��
	cards_golden_shunjin,		//˳��
	cards_golden_baozi,		//����

};

enum win_camp
{
	win_unknow = 0, //δ֪
	win_black, //�ڷ�Ӯ
	win_luck, 
	win_red, //�췽Ӯ
};

enum class poker_type
{
	poker_fangkuai,		//����
	poker_meihua,			//÷��
	poker_hongtao,		//����
	poker_heitao,		    //����	
};

struct poker
{
	int m_id;
	//��ɫ
	poker_type m_poker_type;
	//�˿˵���1��2��3��4��5��6��7��8��9��10��j,q,k
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
