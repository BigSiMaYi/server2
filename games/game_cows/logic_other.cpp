#include "stdafx.h"
#include "logic_other.h"
#include "logic_cards.h"
#include "logic_main.h"
#include "logic_room.h"
#include "logic_player.h"
#include <cmath>
COWS_SPACE_USING

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

GOLD_TYPE logic_other::get_win_gold(uint32_t pid, bool& is_bet)
{
	auto it = m_win_players.find(pid);
	if (it != m_win_players.end())
	{
		//如果下注 m_win_players 找到玩家id, 则说明玩家下注了，外面根据 返回值 == 0 判断错误;
		is_bet = true;
		return it->second;
	}
	is_bet = false;
	return 0;
}

bool logic_other::is_win()
{
	return m_is_win;
}

void logic_other::win(GOLD_TYPE& player_gold, GOLD_TYPE& robot_gold, GOLD_TYPE& bet_gold)
{
	m_win_gold = 0;
	m_is_win = true;

	int cards_rate = m_cards->get_cards_rate();
	for (auto it = m_bet_players.begin(); it != m_bet_players.end(); it++)
	{
		LPlayerPtr& player = m_main->get_room()->get_player(it->first);
		if (player != nullptr)
		{
			GOLD_TYPE win_gold = it->second * cards_rate;
			
			player->change_gold(win_gold, false);
			m_win_players[it->first] = win_gold;

			//SLOG_CRITICAL << "player id: " << it->first << ", bet gold: " << it->second << ", card rate: " << cards_rate << ", win_gold: " << win_gold;

			if (player->is_robot())
			{
				robot_gold -= win_gold;
			}
			else
			{
				player_gold -= win_gold;
				bet_gold += it->second;
			}
			player->quest_change(cards_win, 1, (int)m_cards->get_cards_type());
		}
	}
	m_win_gold = -(robot_gold + player_gold);
}

void logic_other::lose(int cards_rate, GOLD_TYPE& player_gold, GOLD_TYPE& robot_gold, GOLD_TYPE& bet_gold)
{
	m_win_gold = 0;
	m_is_win = false;

	for (auto it = m_bet_players.begin(); it != m_bet_players.end(); it++)
	{
		LPlayerPtr& player = m_main->get_room()->get_player(it->first);
		if (player != nullptr)
		{
			GOLD_TYPE win_gold = it->second * cards_rate;
			if (player->change_gold(-win_gold, false))
			{
				m_win_players[it->first] = -win_gold;

				if (player->is_robot())
				{
					robot_gold += win_gold;
				}
				else
				{
					player_gold += win_gold;
					bet_gold += it->second;
				}
			}
		}
	}
	m_win_gold = -(robot_gold + player_gold);
}

logic_cards* logic_other::get_cards()
{
	return m_cards;
}

void cows_space::logic_other::set_cards(logic_cards* cards)
{
	m_cards = cards;
}
