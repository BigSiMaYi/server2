#include "stdafx.h"
#include "logic_other.h"
#include "logic_cards.h"
#include "logic_main.h"
#include "logic_room.h"
#include "logic_player.h"

DRAGON_TIGER_SPACE_USING

logic_other::logic_other(logic_main* main)
{
	m_main = main;
	m_cards = new logic_cards();
}

logic_other::~logic_other(void)
{
	SAFE_DELETE(m_cards);
}

//开始下一局游戏
void logic_other::start_game()
{
	m_bet_gold = 0;
	m_bet_players.clear();

	m_win_gold = 0;
	m_win_players.clear();

	m_cards->clear_cards();
}

//下注
void logic_other::bet_gold(uint32_t pid, GOLD_TYPE gold)
{
	auto it = m_bet_players.find(pid);
	if (it == m_bet_players.end())
	{
		m_bet_players.insert(std::make_pair(pid, gold));
	}
	else
	{
		it->second += gold;
	}
	m_bet_gold += gold;
}

void logic_other::clear_bet_gold(uint32_t pid)
{
	auto it = m_bet_players.find(pid);
	if (it != m_bet_players.end())
	{
		GOLD_TYPE gold = it->second;	
		m_bet_players.erase(it);
		m_bet_gold -= gold;
	}
}

GOLD_TYPE logic_other::get_total_bet_gold()
{
	return m_bet_gold;
}

GOLD_TYPE logic_other::get_bet_gold(uint32_t pid)
{
	auto it = m_bet_players.find(pid);
	if (it != m_bet_players.end())
	{
		return it->second;
	}
	return 0;
}

GOLD_TYPE logic_other::get_total_win_gold()
{
	return m_win_gold;
}

GOLD_TYPE logic_other::get_win_gold(uint32_t pid)
{
	auto it = m_win_players.find(pid);
	if (it != m_win_players.end())
	{
		return it->second;
	}
	return 0;
}

bool logic_other::is_win()
{
	return m_is_win;
}


logic_cards* logic_other::get_cards()
{
	return m_cards;
}

GOLD_TYPE dragon_tiger_space::logic_other::get_player_bet_gold_panel()
{
	GOLD_TYPE player_bet_gold = 0;
	for (auto it = m_bet_players.begin(); it != m_bet_players.end(); it++)
	{
		LPlayerPtr& player = m_main->get_room()->get_player(it->first);
		if (player != nullptr)
		{
			if (!player->is_robot())
			{
				player_bet_gold += it->second;
			}

		}
	}

	return player_bet_gold;
}

void dragon_tiger_space::logic_other::win_dragon_tiger(logic_room* room, float tax, float winner_odds )
{	
	if (!room) return;
	m_is_win = true;

	for (auto it = m_bet_players.begin(); it != m_bet_players.end(); it++)
	{
		LPlayerPtr& player = m_main->get_room()->get_player(it->first);
		if (player != nullptr)
		{
			player->set_balance_bet(1);
			GOLD_TYPE win_gold = it->second * winner_odds - it->second * winner_odds * tax;
			player->change_gold(win_gold, false);
			m_win_players[it->first] = win_gold;
		
			//赢的钱记录
			GOLD_TYPE win_gold_stat = it->second * winner_odds;
			GOLD_TYPE temp_gold_win_lose = player->get_win_lose_final_gold() + win_gold_stat;
			player->set_win_lose_final_gold(temp_gold_win_lose);

			//把押注的钱记录
			GOLD_TYPE temp_gold = player->get_balance_bet_gold() + it->second;
			player->set_balance_bet_gold(temp_gold);
		}

		// 赢的钱增加库存 这里要加上如果全是玩家的时候不去考虑库存变化的
		if (player != nullptr && !player->is_robot())
		{
			room->add_total_stock_value(-it->second * winner_odds);
			if (room->TotalTaxWinner)
				room->TotalTaxWinner->add_value(it->second * winner_odds * tax);
			room->set_stock_change(true);
		}
	}
}

void dragon_tiger_space::logic_other::lose_dragon_tiger( logic_room* room, float tax, float lose_odds )
{
	if (!room) return;

	m_is_win = false;

	for (auto it = m_bet_players.begin(); it != m_bet_players.end(); it++)
	{
		LPlayerPtr& player = m_main->get_room()->get_player(it->first);
		if (player != nullptr)
		{
			GOLD_TYPE win_gold = it->second * lose_odds - it->second * lose_odds * tax;
			if (player->change_gold(-win_gold, false))
			{
				m_win_players[it->first] = -win_gold;
			
				player->set_balance_bet(1);
				//输的钱记录
				GOLD_TYPE lose_gold = it->second * lose_odds;
				GOLD_TYPE temp_gold_win_lose = player->get_win_lose_final_gold() - lose_gold;
				player->set_win_lose_final_gold(temp_gold_win_lose);

				//把押注的钱记录
				GOLD_TYPE temp_gold = player->get_balance_bet_gold() + it->second;
				player->set_balance_bet_gold(temp_gold);
			}

			if (player != nullptr && !player->is_robot())
			{
				room->add_total_stock_value(it->second * lose_odds);
				if (room->TotalTaxWinner)
					room->TotalTaxWinner->add_value(it->second * lose_odds * tax);
				room->set_stock_change(true);
			}
		}
	}
}
