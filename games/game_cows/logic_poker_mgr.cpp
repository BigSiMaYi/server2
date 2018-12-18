#include "stdafx.h"
#include "logic_poker_mgr.h"
#include "logic_cards.h"
#include "cards_def.h"
#include "game_db.h"

COWS_SPACE_USING

logic_poker_mgr::logic_poker_mgr(int seed)
	:m_random(seed)
{
	m_pokers.reserve(POKERCOUNT);
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
	if (i < POKERCOUNT-1)
	{
		int rand_value = m_random.rand_int(i+1, POKERCOUNT-1);
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

void logic_poker_mgr::swap_player_poker(int player_index)
{
	for (int i = 0; i < CARDSPOKERCOUNT; i++)
	{
		int rand_value = m_random.rand_int(player_index*CARDSPOKERCOUNT+i, player_index*CARDSPOKERCOUNT+4);
		swap_poker(player_index*CARDSPOKERCOUNT+i, rand_value);
	}
}

const poker& logic_poker_mgr::get_poker(int index)
{
	return m_pokers[index];
}

void logic_poker_mgr::rand_shuffle()
{
	m_start_index = 0;
	for (int i = 0; i < 25; i++)
	{
		rand_swap_poker(i);
	}
}

void logic_poker_mgr::player_rand_shuffle(int playerIndex)
{
	for (int i = 0; i < CARDSPOKERCOUNT; i++)
	{
		int rand_value = m_random.rand_int((playerIndex+1)*CARDSPOKERCOUNT, POKERCOUNT-1);
		swap_poker(playerIndex*CARDSPOKERCOUNT + i, rand_value);
	}
}

void logic_poker_mgr::banker_power_shuffle()
{
	rand_shuffle();
	//庄家必牛牌
	logic_cards cards;
	cards.clear_cards();
	cards.init_cards(m_pokers, 0);
	while (cards.get_cards_type() == cards_type::cards_cow_0)
	{
		player_rand_shuffle(0);
		cards.clear_cards();
		cards.init_cards(m_pokers, 0);
	}
}

void logic_poker_mgr::other_power_shuffle()
{
	rand_shuffle();
	//某个闲家必牛牌
	int index = global_random::instance().rand_int(1, 4);

	logic_cards player_cards;
	player_cards.clear_cards();
	player_cards.init_cards(m_pokers, index*CARDSPOKERCOUNT);

	while (player_cards.get_cards_type() == cards_type::cards_cow_0)
	{
		player_rand_shuffle(index);
		player_cards.clear_cards();
		player_cards.init_cards(m_pokers, index*CARDSPOKERCOUNT);
	}
}

void logic_poker_mgr::banker_win_shuffle()
{
	rand_shuffle();

	//庄家牌必须大于牛五
	logic_cards banker_cards;
	banker_cards.clear_cards();
	banker_cards.init_cards(m_pokers, 0);
	while (banker_cards.get_cards_type() <= cards_type::cards_cow_5)
	{
		player_rand_shuffle(0);
		banker_cards.clear_cards();
		banker_cards.init_cards(m_pokers, 0);
	}

	//其他玩家牌小于庄家牌
	logic_cards player_cards;
	for (int i = 1; i <= OTHERCOUNT; i++)
	{
		player_cards.clear_cards();
		player_cards.init_cards(m_pokers, i*CARDSPOKERCOUNT);
		while (logic_cards::compare(&banker_cards, &player_cards) == false)
		{
			player_rand_shuffle(i);
			player_cards.clear_cards();
			player_cards.init_cards(m_pokers, i*CARDSPOKERCOUNT);
		}
	}
}

bool logic_poker_mgr::check_gm_shuffle()
{
	auto ff = BSON("insert_time"<<BSON("$gt"<<mongo::Date_t(0)));
	auto shot = BSON("insert_time"<< 1);
	std::vector<mongo::BSONObj> vec;
	db_game::instance().find(vec, DB_COWSCARDS, ff, nullptr, 1, 0, &shot);

	if(vec.size() >0)
	{
		mongo::BSONObj& v = vec[0];

		int32_t banker_cards = v.getIntField("banker_cards");
		int32_t other_cards1 = v.getIntField("other_cards1");
		int32_t other_cards2 = v.getIntField("other_cards2");
		int32_t other_cards3 = v.getIntField("other_cards3");
		int32_t other_cards4 = v.getIntField("other_cards4");

		gm_shuffle(banker_cards, other_cards1, other_cards2, other_cards3, other_cards4);

		db_game::instance().remove(DB_COWSCARDS, BSON("_id"<< v.getField("_id").OID()), true);
		return true;
	}
	return false;
}

void logic_poker_mgr::gm_shuffle(int banker_cards, int other_cards1, int other_cards2, int other_cards3, int other_cards4)
{
	rand_shuffle();
	shuffle(0, banker_cards);
	shuffle(1, other_cards1);
	shuffle(2, other_cards2);
	shuffle(3, other_cards3);
	shuffle(4, other_cards4);
}

void logic_poker_mgr::shuffle(int playerIndex, int cards)
{
	if (cards == (int)cards_type::cards_small_cow)
	{
		small_suffle(playerIndex);
	}
	else if (cards == (int)cards_type::cards_gold_cow)
	{
		gold_cows_suffle(playerIndex);
	}
	else if (cards == (int)cards_type::cards_bomb)
	{
		bomb_cows_suffle(playerIndex);
	}
	else if (cards == (int)cards_type::cards_silver_cow)
	{
		silver_cows_suffle(playerIndex);
	}
	else if (cards <= (int)cards_type::cards_cow_10 && cards > (int)cards_type::cards_cow_0)
	{
		cows_N_suffle(playerIndex, cards);
	}
	else if (cards == (int)cards_type::cards_cow_0)
	{
		cows_0_suffle(playerIndex);
	}
}

void logic_poker_mgr::small_suffle(int player_index)
{
	int total_point = 10;

	std::vector<int> poker_index;
	for (int i = player_index*CARDSPOKERCOUNT; i < POKERCOUNT; i++)
	{
		int max_point = total_point - CARDSPOKERCOUNT + (int)(poker_index.size()+1);
		if (max_point > 4)
		{
			max_point = 4;
		}
		const poker& p = get_poker(i);
		if (p.m_poker_point <= max_point)
		{
			total_point -= p.m_poker_point;
			poker_index.push_back(i);
			if (poker_index.size() >= 5)
			{
				break;
			}
		}
	}

	if (poker_index.size() == 5)
	{
		for (int i = 0; i < poker_index.size(); i++)
		{
			swap_poker(player_index*CARDSPOKERCOUNT + i, poker_index[i]);
		}
	}
}

void logic_poker_mgr::gold_cows_suffle(int player_index)
{
	std::vector<int> poker_index;
	for (int i = player_index*CARDSPOKERCOUNT; i < POKERCOUNT; i++)
	{
		if (get_poker(i).m_poker_point > 10)
		{
			poker_index.push_back(i);
			if (poker_index.size() >= 5)
			{
				break;
			}
		}
	}

	if (poker_index.size() == 5)
	{
		for (int i = 0; i < poker_index.size(); i++)
		{
			swap_poker(player_index*CARDSPOKERCOUNT + i, poker_index[i]);
		}
	}
}

void logic_poker_mgr::silver_cows_suffle(int player_index)
{
	std::vector<int> poker_index;
	bool have10 = false;
	for (int i = player_index*CARDSPOKERCOUNT; i < POKERCOUNT; i++)
	{
		if (get_poker(i).m_poker_point > 10 || (have10 == false && get_poker(i).m_poker_point == 10))
		{
			poker_index.push_back(i);
			if (poker_index.size() >= 5)
			{
				break;
			}
			if (get_poker(i).m_poker_point == 10)
			{
				have10 = true;
			}
		}
	}

	if (poker_index.size() == 5)
	{
		for (int i = 0; i < poker_index.size(); i++)
		{
			swap_poker(player_index*CARDSPOKERCOUNT + i, poker_index[i]);
		}
	}
}

void logic_poker_mgr::bomb_cows_suffle(int player_index)
{
	std::vector<int> poker_count;
	poker_count.resize(POKERTYPECOUNT);

	for (int i = player_index*CARDSPOKERCOUNT; i < POKERCOUNT; i++)
	{
		poker_count[get_poker(i).m_poker_point-1]++;
	}

	std::vector<int> poker_full;
	for (int i = 0; i < poker_count.size(); i++)
	{
		if (poker_count[i] == 4)
		{
			poker_full.push_back(i);
		}
	}
	if (poker_full.size() > 0)
	{
		int point = poker_full[m_random.rand_int(0, poker_full.size() - 1)]+1;
		int poker_index = 0;
		for (int i = player_index*CARDSPOKERCOUNT; i < POKERCOUNT; i++)
		{
			if (get_poker(i).m_poker_point == point)
			{
				swap_poker(player_index*CARDSPOKERCOUNT + poker_index, i);
				poker_index++;
			}
		}
		int rand_value = m_random.rand_int(player_index*CARDSPOKERCOUNT + 4, POKERCOUNT - 1);
		swap_poker(player_index*CARDSPOKERCOUNT + 4, rand_value);

		swap_player_poker(player_index);
	}
}

void logic_poker_mgr::cows_N_suffle(int player_index, int cows_n)
{
	int max_try_count = 10;
	do 
	{
		max_try_count--;
		if (max_try_count < 0)
			break;

		auto& poker1 = get_poker(player_index*CARDSPOKERCOUNT);
		auto& poker2 = get_poker(player_index*CARDSPOKERCOUNT+1);

		int score = 10 - (poker1.m_poker_score + poker2.m_poker_score)%10;
		if (score == 0)
		{
			score = 10;
		}

		bool match = false;
		for (int i = player_index*CARDSPOKERCOUNT+2; i < POKERCOUNT; i++)
		{
			if (get_poker(i).m_poker_score == score)
			{
				swap_poker(player_index*CARDSPOKERCOUNT + 2, i);
				match = true;
				break;
			}
		}
		if (match == false)
		{
			int rand_value = m_random.rand_int(player_index*CARDSPOKERCOUNT+2, POKERCOUNT - 1);	
			swap_poker(player_index*CARDSPOKERCOUNT + 1, rand_value);
			continue;
		}
		else
		{
			break;
		}
	} 
	while (true);

	do 
	{
		max_try_count--;
		if (max_try_count < 0)
			break;

		auto& poker4 = get_poker(player_index*CARDSPOKERCOUNT+3);
		int score = 0;
		if (poker4.m_poker_score >= cows_n)
		{
			score = cows_n + 10 - poker4.m_poker_score;
		}
		else if(poker4.m_poker_score < cows_n)
		{
			score = cows_n - poker4.m_poker_score;
		}
		bool match = false;
		for (int i = player_index*CARDSPOKERCOUNT+4; i < POKERCOUNT; i++)
		{
			if (get_poker(i).m_poker_score == score)
			{
				swap_poker(player_index*CARDSPOKERCOUNT + 4, i);
				match = true;
				break;
			}
		}
		if (match == false)
		{
			int rand_value = m_random.rand_int(player_index*CARDSPOKERCOUNT+4, POKERCOUNT - 1);	
			swap_poker(player_index*CARDSPOKERCOUNT + 3, rand_value);
			continue;
		}
		else
		{
			break;
		}
	}
	while (true);

	swap_player_poker(player_index);
}

void logic_poker_mgr::cows_0_suffle(int playerIndex)
{
	logic_cards cards;
	cards.clear_cards();
	cards.init_cards(m_pokers, playerIndex*CARDSPOKERCOUNT);
	while (cards.get_cards_type() != cards_type::cards_cow_0)
	{
		player_rand_shuffle(playerIndex);
		cards.clear_cards();
		cards.init_cards(m_pokers, playerIndex*CARDSPOKERCOUNT);
	}
}

void logic_poker_mgr::deal(logic_cards* cards)
{
	cards->init_cards(m_pokers, m_start_index);
	m_start_index += CARDSPOKERCOUNT;
}