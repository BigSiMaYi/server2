#include "stdafx.h"
#include "logic_robot.h"
#include "logic_player.h"
#include "Cows_BaseInfo.h"
#include "logic_room.h"
#include "logic_main.h"
#include "logic_banker.h"
#include "enable_random.h"

#include "Cows_RoomCFG.h"
#include "Cows_RobotCFG.h"

#include "proc_cows_logic.h"

COWS_SPACE_USING

logic_robot::logic_robot(void)
{
	m_life_time = 100;
	m_roomcfg = nullptr;
	m_max_bet_gold = 0;
	m_interval = 1;
	m_base_bet = 100;

	reset_apply_cd();
}

logic_robot::~logic_robot(void)
{

}

//////////////////////////////////////////////////////////////////////////
void logic_robot::heartbeat( double elapsed )
{
	m_life_time -= elapsed;

	m_interval -= elapsed;

	auto game_main = m_player->get_room()->get_game_main();
	if (m_interval < 0)
	{
		if (game_main->get_game_state() == logic_main::game_state::game_state_bet)
		{
			bet();
		}
		m_interval = 1;
	}

	//上庄申请
	m_apply_cd -= elapsed;
	if (m_apply_cd <= 0)
	{
		static int32_t apply_gold = Cows_RobotCFG::GetSingleton()->GetData("RobotBankerGold")->mValue;
		int size = game_main->get_robot_banker_size();
		if ( (game_main->get_banker()->is_player_banker() == false && m_player->get_gold() >= apply_gold) || size < 10)
		{
			auto player = m_player->get_room()->get_player(m_player->get_pid());
			m_player->get_room()->get_game_main()->apply_banker(player);

			//上庄次数
			int32_t min_banker_count = Cows_RobotCFG::GetSingleton()->GetData("RobotBankerMinCount")->mValue;
			int32_t max_banker_count = Cows_RobotCFG::GetSingleton()->GetData("RobotBankerMaxCount")->mValue;
			m_banker_count = global_random::instance().rand_int(min_banker_count, max_banker_count);
		}
		reset_apply_cd();
	}
}

void logic_robot::init(logic_player* player)
{
	m_player = player;

	int32_t min_life_time = Cows_RobotCFG::GetSingleton()->GetData("RobotMinLifeTime")->mValue;
	int32_t max_life_time = Cows_RobotCFG::GetSingleton()->GetData("RobotMaxLifeTime")->mValue;
	m_life_time = global_random::instance().rand_int(min_life_time, max_life_time);
}

bool logic_robot::need_exit()
{
	auto game_main = m_player->get_room()->get_game_main();
	if (game_main->get_banker()->get_player_id() == m_player->get_pid())
	{
		return false;
	}
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

void logic_robot::reset_apply_cd()
{
	int32_t apply_cd = Cows_RobotCFG::GetSingleton()->GetData("RobotBankerCD")->mValue;
	m_apply_cd = global_random::instance().rand_int(apply_cd*0.8f, apply_cd*1.25f);
}

void logic_robot::pre_leave_banker()
{
	auto game_main = m_player->get_room()->get_game_main();
	int32_t banker_count = game_main->get_banker()->get_banker_count() + global_random::instance().rand_int(2, 5);

	if (banker_count < m_banker_count)
	{
		m_banker_count = banker_count;
	}
}

void logic_robot::set_max_bet_gold(GOLD_TYPE bet_gold)
{
	static int betRate = Cows_BaseInfo::GetSingleton()->GetData("BetRate")->mValue;

	m_max_bet_gold = bet_gold;
	if (m_max_bet_gold >= m_player->get_gold()/betRate)
	{
		m_max_bet_gold = m_player->get_gold()/betRate;
	}

	//检测上庄次数
	auto game_main = m_player->get_room()->get_game_main();
	if (game_main->get_banker()->get_player_id() == m_player->get_pid())
	{
		if (game_main->get_banker()->get_banker_count() >= m_banker_count)
		{
			auto player = m_player->get_room()->get_player(m_player->get_pid());
			int32_t cost_ticket = 0;
			game_main->ask_leave_banker(player, false, cost_ticket);
		}
	}
	auto& chipList = m_player->get_room()->get_data()->mChipList;
	if (chipList.size() > 0)
	{
		m_base_bet = chipList[0];
	}
}

void logic_robot::bet()
{
	auto game_main = m_player->get_room()->get_game_main();

	GOLD_TYPE bet_gold = 0;
	if (m_max_bet_gold >= 200*m_base_bet)
	{
		bet_gold = m_base_bet*global_random::instance().rand_int(50, 150);
	}
	else if (m_max_bet_gold >= 20*m_base_bet)
	{
		bet_gold = m_base_bet*global_random::instance().rand_int(5, 15);
	}
	else if (m_max_bet_gold >= 5*m_base_bet)
	{
		bet_gold = m_base_bet*global_random::instance().rand_int(2, 5);
	}
	else if (m_max_bet_gold >= m_base_bet)
	{
		bet_gold = m_base_bet;
	}
	if (bet_gold != 0)
	{
		int32_t bet_index = global_random::instance().rand_int(0, 3);
		auto player = m_player->get_room()->get_player(m_player->get_pid());
		int ret = game_main->bet_gold(player, bet_index, bet_gold);
		if (ret)
		{
			m_max_bet_gold -= bet_gold;

			auto sendmsg = PACKET_CREATE(packetl2c_bet_info_result, e_mst_l2c_bet_info_result);
			sendmsg->set_result((msg_type_def::e_msg_result_def)ret);
			sendmsg->set_bet_index(bet_index+1);
			sendmsg->set_bet_gold(bet_gold);
			sendmsg->set_player_id(m_player->get_pid());

			//玩家下注不广播;
			//m_player->send_msg_to_client(sendmsg);
			//m_player->get_room()->broadcast_msg_to_client(sendmsg);
		}
	}
}