#include "stdafx.h"
#include "logic_robot.h"
#include "logic_player.h"

#include "logic_room.h"
#include "logic_main.h"
#include "enable_random.h"

#include "proc_red_black_logic.h"

#include "RedBlack_RoomCFG.h"
#include "RedBlack_RobotCFG.h"
#include "RedBlack_RoomStockCFG.h"

#include <random>

DRAGON_RED_BLACK_USING

static double rand_float(double min, double max)
{
	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_real_distribution<> d(min, max);
	auto v = d(gen);
	return v;
}

static int rate_choose(std::vector<float>& probs)
{
	if (probs.size() == 0)
	{
		return  0;
	}
	float sum = 0;
	for (auto& val : probs)
	{
		sum += val;
	}

	double rand_val = rand_float(0, sum)/*(double)global_random::instance().rand_double(0, 1)*/;

	for (int i = 0; i < probs.size(); ++i)
	{
		if (rand_val < probs[i])
		{
			return i;
		}
		else
		{
			rand_val -= probs[i];
		}
	}
	return probs.size() - 1;
}
//////////////////////////////////////////////////////////////////////////

logic_robot::logic_robot(void)
{
}

logic_robot::~logic_robot(void)
{
}

void logic_robot::init(logic_player* player)
{
	if (player == nullptr)
	{
		return;
	}
	auto room = player->get_room();
	if (room == nullptr)
	{
		return;
	}

	m_player = player;
	
	auto room_cfg = room->get_data();
	if (room_cfg)
	{
		auto& chipList = room_cfg->mChipList;
		if (chipList.size() > 0)
		{
			m_base_bet = chipList[0];
		}
	}

	auto room_stock_cfg = room->m_room_cfg;
	if (room_stock_cfg)
	{
		int32_t min_life_time = room_stock_cfg->mRobotMinLifeTime;
		int32_t max_life_time = room_stock_cfg->mRobotMaxLifeTime;
		m_life_time = global_random::instance().rand_int(min_life_time, max_life_time);
		auto rate = global_random::instance().rand_int(room_stock_cfg->mRobotRobotMinBet, room_stock_cfg->mRobotRobotMaxBet) / 100.0;
		set_max_bet_gold(rate);
	}
}

//////////////////////////////////////////////////////////////////////////
void logic_robot::heartbeat( double elapsed )
{
	auto room = m_player->get_room();
	if (!room || !room->m_room_cfg || m_player->get_robot_bet())
	{
		return;
	}

	m_life_time -= elapsed;
	m_interval -= elapsed;

	auto game_main = room->get_game_main();
	if (m_interval < 0)
	{
		if (game_main->get_game_state() == logic_main::game_state::game_state_bet)
		{
			if (game_main->get_duration() > 1)
			{
				bet();
			}
		}
		m_interval = global_random::instance().rand_int(m_player->get_room()->m_room_cfg->mRobotMinBetTime, m_player->get_room()->m_room_cfg->mRobotMaxBetTime);
	}
}

bool logic_robot::need_exit()
{
	auto game_main = m_player->get_room()->get_game_main();

	if (m_player->get_gold() < 5000)
	{
		return true;
	}
	if (m_life_time <= 0)
	{
		return true;
	}
	return false;
}

void logic_robot::set_max_bet_gold(double rate)
{
	auto gold = m_player->get_gold();

	m_max_bet_gold = gold * rate;
}

void logic_robot::bet()
{
	auto game_main = m_player->get_room()->get_game_main();

	GOLD_TYPE betgold = 0;
	if (m_max_bet_gold >= 2000 * m_base_bet)
	{
		betgold = m_base_bet*global_random::instance().rand_int(1500, 2000);
	}
	else if (m_max_bet_gold >= 1000 * m_base_bet)
	{
		betgold = m_base_bet*global_random::instance().rand_int(500, 1000);
	}
	else if (m_max_bet_gold >= 500 * m_base_bet)
	{
		betgold = m_base_bet*global_random::instance().rand_int(100, 500);
	}
	else if (m_max_bet_gold >= 100 * m_base_bet)
	{
		betgold = m_base_bet*global_random::instance().rand_int(50, 100);
	}
	else if (m_max_bet_gold >= 50 * m_base_bet)
	{
		betgold = m_base_bet*global_random::instance().rand_int(10, 50);
	}
	else if (m_max_bet_gold >= 20 * m_base_bet)
	{
		betgold = m_base_bet*global_random::instance().rand_int(10, 15);
	}
	else if (m_max_bet_gold >= 10 * m_base_bet)
	{
		betgold = m_base_bet*global_random::instance().rand_int(1, 10);
	}

	GOLD_TYPE times = (betgold / m_base_bet)*m_base_bet;
	int rem = times % (10 * m_base_bet);
	if (rem > 5 * m_base_bet) //个位数最大金币大于 500, 最小筹码太多，减去它;
	{
		betgold -= 5 * m_base_bet;
	}

	if (betgold != 0)
	{
		int32_t bet_index = calc_bet_index(betgold);
		auto player = m_player->get_room()->get_player(m_player->get_pid());
		int ret = game_main->bet_gold(player, bet_index, betgold);
		if (ret == 1)
		{
			player->set_robot_bet(1);
			m_max_bet_gold -= betgold;
			bc_bet_info(m_player, bet_index + 1, betgold, ret);

			SLOG_CRITICAL << "player_gold:" << m_player->get_gold() / 100 << " max_bet_gold: " << m_max_bet_gold / 100 << " bet_gold:" << betgold / 100 << " bet area: " << bet_index;// << " equal_panle:" << gold_panle / 100;
		}
	}
}

void logic_robot::bc_bet_info(logic_player* player, int index, GOLD_TYPE bet_gold, int ret)
{
	auto sendmsg = PACKET_CREATE(packetl2c_bet_info_result, e_mst_l2c_bet_info_result);
	if (!msg_type_def::e_msg_result_def_IsValid(ret))
	{
		SLOG_ERROR << "bet_gold return ret value error:" << ret;
		ret = msg_type_def::e_msg_result_def::e_rmt_unknow;
	}
	sendmsg->set_result((msg_type_def::e_msg_result_def)ret);
	sendmsg->set_bet_index(index);
	sendmsg->set_bet_gold(bet_gold);
	auto pid = player->get_pid();
	sendmsg->set_player_id(pid);

	//玩家下注不广播;
	//m_player->send_msg_to_client(sendmsg);
	player->get_room()->broadcast_msg_to_client(sendmsg);
}

int logic_robot::calc_bet_index(GOLD_TYPE betgold)
{
	int32_t bet_index = 0;
	auto room = m_player->get_room();
	auto mian = room->get_game_main();
	GOLD_TYPE luck_area_bet_cond = global_random::instance().rand_int(10000, room->get_data()->mRobotLuckAreaBetCond);

	if (mian)
	{
		std::vector<float> rates = mian->calc_bet_area_rate();
		std::vector<GOLD_TYPE> chips = mian->get_robot_bet_area_gold();
		GOLD_TYPE allchips = betgold;
		for (auto& it : chips)
		{
			allchips += it;
		}
		if (allchips <= 0)
		{
			return global_random::instance().rand_int(0, 2);
		}
		int size = chips.size() > rates.size() ? rates.size() : chips.size();
		std::map<int, float> temprates; //用于统计 红黑区域下注筹码超过当前比例的差值，如果直接加到该区域，可能导致该区域下注比另一个高很多;
		for (int i = 0; i < size; ++i)
		{
			float r2 = (float)(chips[i] + betgold) / allchips; //当前下注区域的总下注 + 将要下的 / 所有区域下注;

			if (i == 1 /*&& rates[i] > r2*/)//幸运区;
			{
				if (chips[i] + betgold > luck_area_bet_cond)
				{
					continue;
				}
				else
				{
					return i;
				}
			}
				
			if (rates[i] > r2)//如果该区域比例小于计算的比例，则可以继续压该区域;
			{
				return i;
			}
			else
			{
				if (i != 1)
				{
					temprates[i] = fabs(r2 - rates[i]);
				}
			}
		}
		float i = 1.0;
		for (auto& it : temprates)
		{
			if (i > it.second)
			{
				i = it.second;
				bet_index = it.first;
			}
		}
	}
	return  bet_index;
}
