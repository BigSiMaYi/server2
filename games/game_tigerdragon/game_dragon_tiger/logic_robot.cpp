#include "stdafx.h"
#include "logic_robot.h"
#include "logic_player.h"
#include "DragonTiger_BaseInfo.h"
#include "logic_room.h"
#include "logic_main.h"
#include "enable_random.h"

#include "DragonTiger_RoomCFG.h"
#include "DragonTiger_RobotCFG.h"
#include "DragonTiger_RoomStockCFG.h"

DRAGON_TIGER_SPACE_USING

logic_robot::logic_robot(void)
{
	m_life_time = 100;
	m_roomcfg = nullptr;
	m_max_bet_gold = 0;
	m_interval = 1;
	m_base_bet = 100;

}

logic_robot::~logic_robot(void)
{

}

//////////////////////////////////////////////////////////////////////////
void logic_robot::heartbeat( double elapsed )
{

	if (!m_player->get_room() || !m_player->get_room()->m_slmdata) return;

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

}

void logic_robot::init(logic_player* player)
{
	m_player = player; 

	//int32_t min_life_time = DragonTiger_RobotCFG::GetSingleton()->GetData("RobotMinLifeTime")->mValue;
	//int32_t max_life_time = DragonTiger_RobotCFG::GetSingleton()->GetData("RobotMaxLifeTime")->mValue;
	if (m_player->get_room() && m_player->get_room()->m_slmdata)
	{
		int32_t min_life_time =  m_player->get_room()->m_slmdata->mRobotMinLifeTime;
		int32_t max_life_time =  m_player->get_room()->m_slmdata->mRobotMaxLifeTime;
		m_life_time = global_random::instance().rand_int(min_life_time, max_life_time);
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

void logic_robot::set_max_bet_gold(GOLD_TYPE bet_gold)
{
	static int betRate = DragonTiger_BaseInfo::GetSingleton()->GetData("BetRate")->mValue;

	m_max_bet_gold = bet_gold;
	if (m_max_bet_gold >= m_player->get_gold()/betRate)
	{
		m_max_bet_gold = m_player->get_gold()/betRate;
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
		int32_t bet_index = global_random::instance().rand_int(0, 2);
		auto player = m_player->get_room()->get_player(m_player->get_pid());
		int ret = game_main->bet_gold(player, bet_index, bet_gold);
		if (ret)
		{
			m_max_bet_gold -= bet_gold;
		}
	}
}