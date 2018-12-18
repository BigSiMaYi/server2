#include "stdafx.h"
#include "logic_robot.h"
#include "logic_player.h"
#include "DragonTiger_BaseInfo.h"
#include "logic_room.h"
#include "logic_main.h"
#include "enable_random.h"
#include <net/packet_manager.h>
#include "dragon_tiger_logic.pb.h"

#include "DragonTiger_RoomCFG.h"
#include "DragonTiger_RobotCFG.h"
#include "DragonTiger_RoomStockCFG.h"
#include <random>

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
static double rand_float(double min, double max)
{
	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_real_distribution<> d(min, max);
	auto v = d(gen);
	return v;
}

int rate_choose(std::vector<float>& probs)
{
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
			//先随机是不是机器人下注，随机到下注再去bet
			int not_bet = m_player->get_room()->m_slmdata->mRobotCannotBet;
			int value = global_random::instance().rand_int(1, 100);
			if (value > not_bet)
			{
				bet();
			}

		}

		m_interval = global_random::instance().rand_int( m_player->get_room()->m_slmdata->mRobotMinBetTime, m_player->get_room()->m_slmdata->mRobotMaxBetTime);

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
	m_max_bet_gold = bet_gold;

	auto& chipList = m_player->get_room()->get_data()->mChipList;
	if (chipList.size() > 0)
	{
		m_base_bet = chipList[0];
	}
}

void logic_robot::bet()
{
	auto game_main = m_player->get_room()->get_game_main();

	if (m_max_bet_gold == 0 || m_player->get_robot_bet()) return;
	
	GOLD_TYPE bet_gold = 0;
	if (m_max_bet_gold >= 2000*m_base_bet)
	{
		bet_gold = m_base_bet*global_random::instance().rand_int(1500, 2000);
	}
	else if (m_max_bet_gold >= 1000*m_base_bet)
	{
		bet_gold = m_base_bet*global_random::instance().rand_int(500, 1000);
	}
	else if (m_max_bet_gold >= 500 * m_base_bet)
	{
		bet_gold = m_base_bet*global_random::instance().rand_int(100, 500);
	}
	else if (m_max_bet_gold >= 100*m_base_bet)
	{
		bet_gold = m_base_bet*global_random::instance().rand_int(50, 100);
	}
	else if (m_max_bet_gold >= 50*m_base_bet)
	{
		bet_gold = m_base_bet*global_random::instance().rand_int(10, 50);
	}
	else if (m_max_bet_gold >= 20 * m_base_bet)
	{
		bet_gold = m_base_bet*global_random::instance().rand_int(10, 15);
	}
	else if (m_max_bet_gold >= 10*m_base_bet)
	{
		bet_gold = m_base_bet * global_random::instance().rand_int(1, 10);
		if (bet_gold > 500 && bet_gold < 900)
		{
			return;
		}
	}
	if ( ((bet_gold / 100 > 100 || bet_gold / 100 > 10) && 1000 > bet_gold / 100)||(bet_gold / 100 > 1000 && 10000 > bet_gold/100))
	{
		int32_t decrese_gold = (bet_gold / 100) % 10;
		if (decrese_gold > 3)
		{
			bet_gold = bet_gold - decrese_gold * 100;
		}
	}
	if (bet_gold > 500 && bet_gold < 900)
	{
		return;
	}

	if (bet_gold != 0)
	{
		//SLOG_ERROR << "client_bet_gold " << bet_gold/100;
		//客户端用的是1龙 2和 3虎，  服务器对应该桌面 和0，1龙， 2虎
		int32_t bet_index = calc_bet_index();
		auto player = m_player->get_room()->get_player(m_player->get_pid());
		int gold_panle = game_main->get_room()->get_equal_panel_gold();
		if (bet_gold > 100 && bet_gold < 6000)
		{
			if (gold_panle > bet_gold)
			{
				bet_index = 2;
				game_main->get_room()->set_equal_panel_gold(game_main->get_room()->get_equal_panel_gold() - bet_gold);
			}
		}

		int ret = game_main->bet_gold(player, bet_index, bet_gold);
		if (ret == msg_type_def::e_msg_result_def::e_rmt_success)
		{
			SLOG_ERROR << "player_gold:" << m_player->get_gold() / 100 << "   max_bet_gold: " << m_max_bet_gold / 100 << "   bet_gold:" << bet_gold/100 << "  bet area: " << bet_index <<" equal_panle:" << gold_panle/100;
			m_max_bet_gold -= bet_gold;
			player->set_robot_bet(1);
			auto sendmsg = PACKET_CREATE(dragon_tiger_protocols::packetl2c_bet_info_result, dragon_tiger_protocols::e_mst_l2c_bet_info_result);
			if (!msg_type_def::e_msg_result_def_IsValid(ret))
			{
				SLOG_ERROR << "bet_gold return ret value error:" << ret;
				ret = msg_type_def::e_msg_result_def::e_rmt_unknow;
			}
			sendmsg->set_result((msg_type_def::e_msg_result_def)ret);
			sendmsg->set_bet_index(bet_index);
			sendmsg->set_bet_gold(bet_gold);
			sendmsg->set_player_id(m_player->get_pid());

			//玩家下注不广播;
			m_player->get_room()->broadcast_msg_to_client(sendmsg);

		}

	}

}

int logic_robot::calc_bet_index()
{
	//游戏少于3局
	auto room = m_player->get_room();
	auto& history_infos = room->get_cards_trend();
 	if (history_infos.size() < 3)
	{
		int value = global_random::instance().rand_int(1, 2);
		if (value == 2)
		{
			value = 3;//龙虎各百分之五十
		}
		return value;
	}

	//游戏大于3局统计前3局
	int index = 3;
	int dragon = 50;
	int tiger = 50;
	int flag_record = 0;
	std::list<cards_trend>::iterator itr = history_infos.begin();
	for (; itr != history_infos.end(); itr++)
	{
		flag_record++;
		if ((*itr).m_is_win[0] == 1)
		{
			dragon = dragon - 2;
			tiger = tiger + 2;
		}
		if ((*itr).m_is_win[2] == 1)
		{
			dragon = dragon + 2;
			tiger = tiger - 2;
		}
		if (flag_record >= 3) break;
		
	}

	int value_dr = global_random::instance().rand_int(1, 100);
	if (value_dr < dragon)
	{
		return 1;
	}
	return 3;

// 	auto room = m_player->get_room();
// 	auto& history_infos = room->get_cards_trend();
// 	if (history_infos.size() < 3)
// 	{
// 		double r = global_random::instance().rand_double(0.05, 0.08);
// 		std::vector<float> rates;
// 
// 		rates.push_back((1.0 - r) / 2);
// 		rates.push_back(r);
// 		rates.push_back((1.0 - r) / 2);
// 
// 		int32_t bet_index = rate_choose(rates);
// 		return bet_index;
// 	
// 	}
// 	std::map<int, int> rate;
// 	int index = 3;
// 	for (auto& item : history_infos)
// 	{
// 		int size = sizeof(item.m_is_win) / sizeof(item.m_is_win[0]);
// 		for (int i = 0; i < size; ++i)
// 		{
// 			if (i != 1)
// 			{
// 				if (item.m_is_win[i])
// 				{
// 					rate[i] -= 2;
// 				}
// 				else
// 				{
// 					rate[i] += 2;
// 				}
// 			}
// 		}
// 		if (--index < 0)
// 		{
// 			break;
// 		}
// 	}
// 
// 	int sum = 0;
// 	for (auto& m : rate)
// 	{
// 		m.second += 20;
// 		sum += m.second;
// 	}
// 
// 	double r = global_random::instance().rand_double(0.05, 0.08);
// 	double luck_val = (sum / (1.0 - r)) * r;
// 	rate[1] = luck_val;
// 	sum += luck_val;
// 	std::vector<float> rates;
// 	for (auto& m : rate)
// 	{
// 		float rate = (float)m.second / sum;
// 		rates.push_back(rate);
// 	}
// 	int i = rate_choose(rates);
// 
// 	return i;

}
