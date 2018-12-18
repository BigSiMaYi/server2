#include "stdafx.h"
#include "logic_poker_mgr.h"
#include "cards_def.h"
#include "game_db.h"

DRAGON_RED_BLACK_USING

logic_poker_mgr::logic_poker_mgr(int seed)
	:m_random(seed)
{
	m_pokers.reserve(POKERCOUNT);
	
	//1付牌随机
	for (int i = 0; i < POKERCOUNT; i++)
	{
		poker_type type = (poker_type)(i/POKERTYPECOUNT);
		int value = i%POKERTYPECOUNT + 1;
		m_pokers.push_back(poker(type, value));
	}

	for (int i = 0; i < 51; i++)
	{
		rand_swap_poker(i);
	}

}

logic_poker_mgr::~logic_poker_mgr(void)
{
}

void logic_poker_mgr::rand_swap_poker(int i)
{
	if (i < POKERCOUNT -1)
	{
		int rand_value = m_random.rand_int(i+1, POKERCOUNT - 1);
		std::swap(m_pokers[i], m_pokers[rand_value]);
	}
}

void logic_poker_mgr::swap_poker(int i, int j)
{
	if (i < POKERCOUNT-1 && j < POKERCOUNT-1 )
	{
		std::swap(m_pokers[i], m_pokers[j]);
	}
}


const poker& logic_poker_mgr::get_poker(int index)
{
	return m_pokers[index];
}

void logic_poker_mgr::rand_shuffle()
{
	for (int i = 0; i < 6; i++)
	{
		rand_swap_poker(i);
	}
}

void logic_poker_mgr::shuffle_total_pokes()
{
	m_pokers.clear();

	for (int i = 0; i < POKERCOUNT; i++)
	{
		poker_type type = (poker_type)(i / POKERTYPECOUNT);
		int value = i%POKERTYPECOUNT + 1;
		m_pokers.push_back(poker(type, value));
	}

	for (int j = 0; j < 8; j++)
	{
		for (int i = 0; i < 51; i++)
		{
			rand_swap_poker(i);
		}
	}
}

void  logic_poker_mgr::get_red_pokers(std::vector<poker> &vec_red_pokes)
{
	for (int i = 0; i < 3; i++)
	{
		vec_red_pokes.push_back(m_pokers[i]);
	}
	return;
}

void  logic_poker_mgr::get_black_pokers(std::vector<poker> &vec_black_pokes)
{
	for (int i = 3; i < 6; i++)
	{
		vec_black_pokes.push_back(m_pokers[i]);
	}
	return;
}

 cards_golden_type  logic_poker_mgr::compare_red_black_pokes(std::vector<poker> &vec_red_pokes, std::vector<poker> &vec_black_pokes,
																								      cards_golden_type &red_type, cards_golden_type &black_type, win_camp &win_camp_, int &duizi_value)
{
	std::vector<poker> vec_red_poke_point;
	std::vector<poker> vec_black_poke_point;
	vec_red_poke_point.clear();
	vec_black_poke_point.clear();

	red_type = get_red_black_pokes_type(vec_red_pokes, vec_red_poke_point);
	black_type = get_red_black_pokes_type(vec_black_pokes, vec_black_poke_point);

	if (red_type == cards_golden_type::cards_golden_unknown || black_type == cards_golden_type::cards_golden_unknown) return cards_golden_type::cards_golden_unknown;
	
	if ( red_type > black_type )
	{
		if (red_type == cards_golden_type::cards_golden_duizi)
		{
			duizi_value = vec_red_poke_point[0].m_poker_point;
		}
		win_camp_ = win_red;
		return red_type;
	}
	
	if (red_type < black_type)
	{
		if (black_type == cards_golden_type::cards_golden_duizi)
		{
			duizi_value = vec_black_poke_point[0].m_poker_point;
		}
		win_camp_ = win_black;
		return black_type;
	}
	

	if (red_type == black_type)
	{
		switch (red_type)
		{
		case cards_golden_type::cards_golden_baozi:
		case cards_golden_type::cards_golden_shunjin:
		case cards_golden_type::cards_golden_shunzi:
			{ // 比较最大的一张牌
				int point_red  = vec_red_poke_point[0].m_poker_point;
				if (point_red == 1)
					point_red += 14;//给A一个大值
				int point_black  = vec_black_poke_point[0].m_poker_point;
				if (point_black == 1)
					point_black += 14;
				if (point_red > point_black )
				{
					win_camp_ = win_red;
					return red_type;
				}
				else if (point_red == point_black)
				{
					//按照黑、红、梅、方顺序
					if (vec_red_poke_point[0].m_poker_type > vec_black_poke_point[0].m_poker_type)
					{
						win_camp_ = win_red;
						return red_type;
					}
					else
					{
						win_camp_ = win_black;
						return black_type;
					}
				}
				else
				{
					win_camp_ = win_black;
					return black_type;
				}
			}
			break;
		case cards_golden_type::cards_golden_jinhua:
		case cards_golden_type::cards_golden_danzhang:
			{ 
				bool three_pokes_point_equal  = false;
				int point_red  = vec_red_poke_point[0].m_poker_point;
				if (point_red == 1)
					point_red += 14;//给A一个大值
				int point_black  = vec_black_poke_point[0].m_poker_point;
				if (point_black == 1)
					point_black += 14;
				if(point_red > point_black)
				{
					win_camp_ = win_red;
					return red_type;
				}
				else if(point_red == point_black)
				{
					//第一张相等看第二张牌
					if (vec_red_poke_point[1].m_poker_point > vec_black_poke_point[1].m_poker_point)
					{
						win_camp_ = win_red;
						return red_type;
					}
					else if (vec_red_poke_point[1].m_poker_point == vec_black_poke_point[1].m_poker_point)
					{
						//第二张相等看第三张牌
						if (vec_red_poke_point[2].m_poker_point > vec_black_poke_point[2].m_poker_point)
						{
							win_camp_ = win_red;
							return red_type;
						}
						else if (vec_red_poke_point[2].m_poker_point == vec_black_poke_point[2].m_poker_point)
						{
							three_pokes_point_equal = true;
						}
						else
						{
							win_camp_ = win_black;
							return black_type;
						}
					}
					else
					{
						win_camp_ = win_black;
						return black_type;
					}
				}
				else
				{
					win_camp_ = win_black;
					return black_type;
				}
				if (three_pokes_point_equal)//如果3张牌都相等 比如红桃3 黑桃5 方块8  方块3 红桃4 梅花8
				{
					//比最大牌的花色 	按照黑、红、梅、方顺序
					if (vec_red_poke_point[0].m_poker_type > vec_black_poke_point[0].m_poker_type)
					{
						win_camp_ = win_red;
						return red_type;
					}
					else
					{
						win_camp_ = win_black;
						return black_type;
					}
				}
			}
			break;
		case cards_golden_type::cards_golden_duizi:
			{
				//前两张牌是对子
				bool three_pokes_point_equal  = false;
				int point_red  = vec_red_poke_point[0].m_poker_point;
				if (point_red == 1)
					point_red += 14;//给A一个大值
				int point_black  = vec_black_poke_point[0].m_poker_point;
				if (point_black == 1)
					point_black += 14;
				if(point_red > point_black)
				{
					duizi_value = vec_red_poke_point[0].m_poker_point;
					win_camp_ = win_red;
					return red_type;
				}
				else if(point_red == point_black)
				{
					duizi_value = vec_red_poke_point[0].m_poker_point;

					int point_red_3  = vec_red_poke_point[0].m_poker_point;
					if (point_red_3 == 1)
						point_red_3 += 14;//给A一个大值
					int point_black_3  = vec_black_poke_point[0].m_poker_point;
					if (point_black_3 == 1)
						point_black_3 += 14;

					if (point_red_3 > point_black_3)
					{
						win_camp_ = win_red;
						return red_type;
					}
					else if (point_red_3 == point_black_3)
					{
						three_pokes_point_equal = true;
					}
					else
					{
						win_camp_ = win_black;
						return black_type;
					}
				}
				else
				{
					duizi_value  = vec_black_poke_point[0].m_poker_point;
					win_camp_ = win_black;
					return black_type;
				}
				if (three_pokes_point_equal) //三张牌要是相等,看对子中的黑桃在哪家
				{
					duizi_value = vec_red_poke_point[0].m_poker_point;
					if (vec_red_poke_point[0].m_poker_type == poker_type::poker_heitao ||
						vec_red_poke_point[1].m_poker_type == poker_type::poker_heitao)
					{
						win_camp_ = win_red;
						return red_type;
					}
					else
					{
						win_camp_ = win_black;
						return black_type;
					}
				}
			}
			break;
		default:
			break;
		}
	}

	return cards_golden_type::cards_golden_unknown;
}
bool sort_poker_greater(poker& a, poker& b)
{
	return a.m_poker_point > b.m_poker_point;
}
 cards_golden_type  logic_poker_mgr::get_red_black_pokes_type(std::vector<poker> &vec_pokes, std::vector<poker> &vec_poke_point)
{
	if (vec_pokes.size() < 3) return cards_golden_type::cards_golden_unknown;
	
	std::vector<poker> record_pokes = vec_pokes;
	std::sort(record_pokes.begin(), record_pokes.end(), sort_poker_greater);

	int poke_point[14];
	poker_type poke_point_type[14];
	memset(poke_point,0,sizeof(poke_point));
	memset(poke_point_type,0,sizeof(poke_point_type));
	
	int index = 0 ;
	for (int i = 0; i < vec_pokes.size(); i++)
	{
		index = vec_pokes[i].m_poker_point;
		++poke_point[index];

		poke_point_type[index] = vec_pokes[i].m_poker_type;
	}

	// 特殊的顺子 [A 2 3]
	if (1 == poke_point[1] && 1 == poke_point[2] && 1 == poke_point[3])
	{
		vec_poke_point.push_back(record_pokes[0]);
		vec_poke_point.push_back(record_pokes[1]);
		vec_poke_point.push_back(record_pokes[2]);
		if(poke_point_type[1]==poke_point_type[2] && poke_point_type[2]==poke_point_type[3])
		{ // 同花顺
			return cards_golden_type::cards_golden_shunjin;
		}
		else
		{ // 顺子
			return cards_golden_type::cards_golden_shunzi;
		}
	}

	int card_flag_record = 0;
	bool exist_duizi = false;
	cards_golden_type golden_type = cards_golden_type::cards_golden_unknown;
	// 牌型计算
	for (int i = 1; i < 14; i++)
	{
		if( 0 == poke_point[i] ) continue;
		switch( poke_point[i] )
		{
		case 3: // 豹子
			{
				vec_poke_point.push_back(record_pokes[0]);
				vec_poke_point.push_back(record_pokes[1]);
				vec_poke_point.push_back(record_pokes[2]);
				return cards_golden_type::cards_golden_baozi;
			}
		case 2: // 对子
			{
				card_flag_record += 2;
				exist_duizi = true;
				if (card_flag_record == 3)
				{
					// ！！！先有单张再有对子
					for (int j = 0; j < vec_pokes.size(); j++)
					{
						if (i == vec_pokes[j].m_poker_point)
						{
							vec_poke_point.push_back(vec_pokes[j]);
						}
					}
					for (int diff = 0; diff < vec_pokes.size(); diff++)
					{
						if (i != vec_pokes[diff].m_poker_point)
						{
							vec_poke_point.push_back(vec_pokes[diff]);
						}
					}
					return cards_golden_type::cards_golden_duizi;
				}
				break;
			}
		case 1:
			{ 
				card_flag_record += 1;
				if (exist_duizi && card_flag_record == 3)// !!先有对子后有单张
				{
					for (int diff = 0; diff < vec_pokes.size(); diff++)
					{
						if (i != vec_pokes[diff].m_poker_point)
						{
							vec_poke_point.push_back(vec_pokes[diff]);
						}
					}
					for (int j = 0; j < vec_pokes.size(); j++)
					{
						if (i == vec_pokes[j].m_poker_point)
						{
							vec_poke_point.push_back(vec_pokes[j]);
						}
					}
					return cards_golden_type::cards_golden_duizi;
				}
				if (card_flag_record > 1)
				{
					if (record_pokes[2].m_poker_point == 1) //最小为A时
					{
						vec_poke_point.push_back(record_pokes[2]);
						vec_poke_point.push_back(record_pokes[0]);
						vec_poke_point.push_back(record_pokes[1]);
					}
					else
					{
						vec_poke_point.push_back(record_pokes[0]);
						vec_poke_point.push_back(record_pokes[1]);
						vec_poke_point.push_back(record_pokes[2]);
					}
					// 遍历时 已经有一张单牌
					if (vec_pokes[0].m_poker_type == vec_pokes[1].m_poker_type 
						 && vec_pokes[1].m_poker_type == vec_pokes[2].m_poker_type)
					{
						golden_type = cards_golden_type::cards_golden_jinhua;
						//三张牌差值都为1 普通的顺子 差值为11表示 QKA
						int point_diff = record_pokes[1].m_poker_point - record_pokes[2].m_poker_point;
						if (record_pokes[0].m_poker_point - record_pokes[1].m_poker_point == 1 && (point_diff == 1 || point_diff == 11) )
						{
							golden_type = cards_golden_type::cards_golden_shunjin;
						}

						return golden_type;
					}
					else
					{
						golden_type = cards_golden_type::cards_golden_danzhang;	
						int point_diff = record_pokes[1].m_poker_point - record_pokes[2].m_poker_point;
						if (record_pokes[0].m_poker_point - record_pokes[1].m_poker_point == 1 && (point_diff == 1 || point_diff == 11) )
						{
							golden_type = cards_golden_type::cards_golden_shunzi;
						}
						return golden_type;
					}
				}
			}//case 1
		 default:
			 break;
		}
	}//end for (int i = 1; i < 14; i++)
	return cards_golden_type::cards_golden_unknown;
}


