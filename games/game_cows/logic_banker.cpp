#include "stdafx.h"
#include "logic_banker.h"
#include "logic_cards.h"
#include "logic_player.h"
#include "logic_room.h"

#include "Cows_BaseInfo.h"
#include "proc_cows_logic.h"
#include "game_db_log.h"
#include "time_helper.h"

COWS_SPACE_USING

logic_banker::logic_banker(void)
{
	m_cards = new logic_cards();
	m_apply_leave = false;
	m_banker_count = 0;
	m_offline_count = 0;
	m_total_win_gold = 0;
	m_last_win_gold = 0;
	m_max_bet_gold = 0;
	m_cur_bet_gold = 0;
	m_system_gold = 0;

	m_min_banker_count = Cows_BaseInfo::GetSingleton()->GetData("MinBankerCount")->mValue;
	m_max_banker_count = Cows_BaseInfo::GetSingleton()->GetData("MaxBankerCount")->mValue;
	m_leave_banker_cost = Cows_BaseInfo::GetSingleton()->GetData("CostLeaveBanker")->mValue/100.0f;

	m_start_time = 0;
	m_start_gold = 0;
	//系统庄家名字，金钱
	resetSystemBanker();
}

logic_banker::~logic_banker(void)
{
	SAFE_DELETE(m_cards);
}

void logic_banker::check_player_state()
{
	//断线检测
	if (m_player != nullptr)
	{
		if (m_player->is_offline())
		{
			m_offline_count++;
		}
		else
		{
			m_offline_count = 0;
		}
		if (m_offline_count >= 2)
		{
			if (m_banker_count < m_min_banker_count)
			{
				force_leave();
			}
			apply_leave();
		}
		if (!m_player->is_robot())
		{
			//玩家连庄限制;
			if (m_banker_count == m_max_banker_count - 1)
			{
				apply_leave();
			}
		}
	}
}

void logic_banker::start_game()
{
	m_banker_count ++;

	m_cards->clear_cards();

	static int betRate = Cows_BaseInfo::GetSingleton()->GetData("BankerBetRate")->mValue;

	m_max_bet_gold = get_banker_gold()/betRate;
	m_cur_bet_gold = 0;

	m_last_win_gold = 0;
}

GOLD_TYPE logic_banker::get_max_bet_gold()
{
	return m_max_bet_gold;
}

GOLD_TYPE logic_banker::get_banker_gold()
{
	if (m_player == nullptr)
	{
		return m_system_gold;
	}
	else
	{
		return m_player->get_gold();
	}
}

bool logic_banker::check_bet_gold(GOLD_TYPE gold)
{
	if (m_cur_bet_gold + gold > m_max_bet_gold)
	{
		return false;
	}
	return true;
}

void logic_banker::bet_gold(GOLD_TYPE gold)
{
	m_cur_bet_gold += gold;
}

void logic_banker::clear_bet_gold(GOLD_TYPE gold)
{
	m_cur_bet_gold -= gold;
}

GOLD_TYPE logic_banker::get_cur_bet_gold()
{
	return m_cur_bet_gold;
}

logic_cards* logic_banker::get_cards()
{
	return m_cards;
}

void logic_banker::set_cards(logic_cards* cards)
{
	m_cards = cards;
}

void logic_banker::win_gold(GOLD_TYPE gold, GOLD_TYPE tax)
{
	m_last_win_gold = (gold - tax);
	if (m_player == nullptr)
	{
		m_system_gold += (gold - tax);
	}
	else if (m_player->is_robot())
	{
		m_sys_lose_gold = 0;
		if (gold < 0)
		{
			if (-gold > m_player->get_gold())
			{
				gold = -m_player->get_gold();
			}
		}
		m_player->change_gold(gold);

		//增加税收;
		m_player->set_tax(tax);

		m_total_win_gold += (gold - tax);
	}
	else
	{
		m_sys_lose_gold = 0;
		if (gold < 0)
		{
			//如果爆庄，金币最小为0
			if (-gold > m_player->get_gold())
			{
				GOLD_TYPE systemGold = m_player->get_gold() + gold;
				m_sys_lose_gold = -systemGold;
				if (m_player->get_room() != nullptr)
				{
					m_player->get_room()->log_banker_lost_gold(m_sys_lose_gold);
				}
				gold = -m_player->get_gold();
			}
		}
		else if (gold > 0)
		{
			m_player->quest_change(banker_win_count, 1);
		}
		m_player->change_gold(gold);

		//增加税收;
		m_player->set_tax(tax);

		m_total_win_gold += (gold - tax);
	}
}

uint32_t logic_banker::get_player_id()
{
	if (m_player == nullptr)
	{
		return 0;
	}
	return m_player->get_pid();
}

LPlayerPtr& logic_banker::get_player()
{
	return m_player;
}
//名字
std::string logic_banker::get_banker_name()
{
	if (m_player == nullptr)
	{
		return m_system_name;
	}
	else
	{
		return m_player->get_nickname();
	}
}

void logic_banker::resetSystemBanker()
{
	m_system_name = "系统小庄";
	m_system_gold = 1000000000000;
}

void logic_banker::set_player_banker(LPlayerPtr& player)
{
	//下庄扣钱
	if (m_player != nullptr)
	{
		GOLD_TYPE cost_gold = m_total_win_gold * m_leave_banker_cost;
		if (cost_gold < 0)
		{
			cost_gold = 0;
		}
		m_player->change_gold(-cost_gold, 0);
		if (m_player->get_room() != nullptr)
		{
			m_player->get_room()->log_banker_win_gold(cost_gold);
		}
		time_t end_time = time_helper::instance().get_cur_time();
		db_log::instance().playerBanker(m_player.get(), m_start_time, end_time, m_banker_count, m_start_gold, m_player->get_gold(), m_total_win_gold, cost_gold, m_sys_lose_gold);
	}

	m_player = player;
	m_banker_count = 0;
	m_offline_count = 0;
	m_last_win_gold = 0;
	m_total_win_gold = 0;

	if (m_player != nullptr)
	{
		m_start_time = time_helper::instance().get_cur_time();
		m_start_gold = m_player->get_gold();
	}

	m_apply_leave = false;
}

bool logic_banker::is_banker(uint32_t player_id)
{
	if (m_player == nullptr)
		return false;

	return m_player->get_pid() == player_id;
}

bool logic_banker::is_player_banker()
{
	return m_player != nullptr;
}

bool logic_banker::is_banker_protect()
{
	//上庄保护期
	if (m_banker_count < m_min_banker_count)
	{
		return true;
	}
	return false;
}

bool logic_banker::is_system_banker()
{
	if (m_player == nullptr)
	{
		return true;
	}
	else if (m_player->is_robot())
	{
		return true;
	}
	return false;
}

int logic_banker::get_banker_count()
{
	return m_banker_count;
}

int logic_banker::get_min_banker_count()
{
	return m_min_banker_count;
}

GOLD_TYPE logic_banker::get_total_win_gold()
{
	return m_total_win_gold;
}

GOLD_TYPE logic_banker::get_last_win_gold()
{
	return m_last_win_gold;
}

void logic_banker::fill_player_info(cows_protocols::msg_player_info* player_info)
{
	if (m_player == nullptr)
	{
		player_info->set_player_id(0);
		//player_info->set_player_name(m_system_name); //去掉昵称;
		player_info->set_head_frame(0);
		player_info->set_head_custom("");
		player_info->set_player_gold(m_system_gold);
		player_info->set_player_sex(1);
		player_info->set_vip_level(0);
		player_info->set_player_region("");
	}
	else
	{
		player_info->set_player_id(m_player->get_pid());
		//player_info->set_player_name(m_player->get_nickname());//去掉昵称;
		player_info->set_head_frame(m_player->get_photo_frame());
		player_info->set_head_custom(m_player->get_icon_custom());
		player_info->set_player_gold(m_player->get_gold());
		player_info->set_player_sex(m_player->get_sex());
		player_info->set_vip_level(m_player->get_viplvl());
		player_info->set_player_region(m_player->GetUserRegion());
	}
}


int logic_banker::force_leave()
{
	int cost_ticket = Cows_BaseInfo::GetSingleton()->GetData("ForceLeaveCost")->mValue;
	if (m_player->change_ticket(-cost_ticket, 34))
	{
		apply_leave();
		return cost_ticket;
	}
	return -1;
}

void logic_banker::apply_leave()
{
	m_apply_leave = true;
}

bool logic_banker::is_apply_leave()
{
	return m_apply_leave;
}

const std::string logic_banker::getBankerRegion()
{
	if (m_player)
	{
		SLOG_CRITICAL<<"logic_banker::getBankerRegion :"<<m_player->get_pid() <<"region="<<m_player->GetUserRegion();
		return m_player->GetUserRegion();
	}
	else
	{
		SLOG_CRITICAL<<"logic_banker::getBankerRegion : m_player is nullptr";
		return "中国";
	}	
}