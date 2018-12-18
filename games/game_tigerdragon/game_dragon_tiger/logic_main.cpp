#include "stdafx.h"
#include "logic_main.h"
#include "logic_room.h"


#include "logic_other.h"
#include "logic_poker_mgr.h"
#include "logic_cards.h"
#include "logic_player.h"
#include "logic_robot.h"

#include "DragonTiger_BaseInfo.h"
#include "DragonTiger_RoomCFG.h"
#include "DragonTiger_Cardodds.h"
#include "proc_dragon_tiger_logic.h"
#include "time_helper.h"
#include "logic_def.h"

DRAGON_TIGER_SPACE_USING

logic_main::logic_main(logic_room* room)
{
	m_room = room;
	for (int i = 0; i < OTHERCOUNT; i++)
	{
		auto other = new logic_other(this);
		m_others.push_back(other);
	}
	m_poker_mgr = new logic_poker_mgr(global_random::instance().rand_int(0, 100000000));

	m_log_time = 0;
	m_inc_id = 1;

	set_game_state(game_state::game_state_unknown);
	pre_start_game();
}

logic_main::~logic_main(void)
{
	for (unsigned int i = 0; i < m_others.size(); i++)
	{
		SAFE_DELETE(m_others[i]);
	}
	SAFE_DELETE(m_poker_mgr);
}

void logic_main::heartbeat(double elapsed)
{
	if (m_duration < 0)
		return;

	m_duration -= elapsed;
	switch (m_game_state)
	{
	case game_state::game_state_unknown:
		break;
	case game_state::game_state_prepare:
		{
			if (m_duration < 0)
			{
				start_game();
			}
		}
		break;
	case game_state::game_state_bet:
		m_sync_eleased += elapsed;
		if (m_duration < 0)
		{
			deal_cards();
		}
		else if (m_sync_eleased >= m_sync_interval)
		{
			sync_bet_gold();
			m_sync_eleased = 0.0f;
		}
		break;
	case game_state::game_state_deal:
		{
			if (m_duration < 0)
			{
				computer_result();
			}
		}
		break;
	case game_state::game_state_result:
		if (m_duration < 0)
		{
			pre_start_game();
		}
		break;
	default:
		break;
	}
}


void logic_main::stop_game()
{
	set_game_state(game_state::game_state_unknown);
}

void logic_main::pre_start_game()
{
	m_inc_id++;
	set_game_state(game_state::game_state_prepare);

	clear_last_data();

	for (int i = 0; i < OTHERCOUNT; i++)
	{
		m_others[i]->start_game();
	}
	m_room->init_robot_bet();
	m_change_bet = false;

	auto sendmsg = PACKET_CREATE(packetl2c_bc_scene_prepare_into, e_mst_l2c_bc_scene_prepare_into);
	sendmsg->set_count_down(get_cd_time());
	//!!这里消息也要删除掉庄家信息
	m_room->broadcast_msg_to_client(sendmsg);
}

void logic_main::start_game()
{
	set_game_state(logic_main::game_state::game_state_bet);

	auto sendmsg = PACKET_CREATE(packetl2c_bc_scene_bet_into, e_mst_l2c_bc_scene_bet_into);
	sendmsg->set_count_down(get_cd_time());
	m_room->broadcast_msg_to_client(sendmsg);
}

int logic_main::bet_gold(LPlayerPtr& player, uint32_t index, GOLD_TYPE gold)
{
	if (m_game_state != game_state::game_state_bet)
		return msg_type_def::e_msg_result_def::e_rmt_error_game_state;


	if (index >= m_others.size())
		return msg_type_def::e_msg_result_def::e_rmt_unknow;

	if (gold < 0)
		return msg_type_def::e_msg_result_def::e_rmt_unknow;

	//下注金额限制
	bool ret = false;

	//自己金钱是否足够
	GOLD_TYPE total_gold = get_player_betgold(player);
	ret = player->check_bet_gold(gold, total_gold);
	if (!ret)
		return msg_type_def::e_msg_result_def::e_rmt_other_betgold_is_full;

	if (index > 4 || index < 0) 
		return msg_type_def::e_msg_result_def::e_rmt_unknow;

	m_others[index]->bet_gold(player->get_pid(), gold);
	add_player_betgold(player, gold);
	m_change_bet = true;

	return msg_type_def::e_msg_result_def::e_rmt_success;
}

void logic_main::clear_bet_gold(LPlayerPtr& player)
{
	GOLD_TYPE total_gold = get_player_betgold(player);
	clear_player_betgold(player);

	for (int i = 0; i < 4; i++)
	{
		m_others[i]->clear_bet_gold(player->get_pid());
	}
	m_change_bet = true;
}

GOLD_TYPE logic_main::get_player_betgold(LPlayerPtr& player)
{
	auto it = m_bet_players.find(player->get_pid());
	if (it == m_bet_players.end())
	{
		return 0;
	}
	return it->second;
}

void logic_main::add_player_betgold(LPlayerPtr& player, GOLD_TYPE gold)
{
	auto it = m_bet_players.find(player->get_pid());
	if (it != m_bet_players.end())
	{
		it->second += gold;
	}
	else
	{
		m_bet_players.insert(std::make_pair(player->get_pid(), gold));
	}
}

void logic_main::update_star_gold()
{
	for (auto it = m_bet_players.begin(); it != m_bet_players.end(); it++)
	{
		auto& player = m_room->get_player(it->first);
		if (player != nullptr)
		{
			static double AwardGetRate = DragonTiger_BaseInfo::GetSingleton()->GetData("AwardGetRate")->mValue/10000.0f;
			player->add_star_lottery_info(it->second*AwardGetRate, 0);
			player->quest_change(game_count, 1);
		}
	}
}

void logic_main::clear_player_betgold(LPlayerPtr& player)
{
	m_bet_players.erase(player->get_pid());
}

void logic_main::clear_last_data()
{
	m_bet_players.clear();
}

void logic_main::sync_bet_gold()
{
	if (m_change_bet)
	{
		m_change_bet = false;
		auto sendmsg = PACKET_CREATE(packetl2c_bc_sync_scene_bet_into, e_mst_l2c_bc_sync_scene_bet_into);
		sendmsg->mutable_bet_golds()->Reserve(m_others.size());
		for (unsigned int i = 0; i < m_others.size(); i++)
		{
			sendmsg->add_bet_golds(m_others[i]->get_total_bet_gold());
		}
		m_room->broadcast_msg_to_client(sendmsg);
	}
}

void logic_main::deal_cards()
{
	m_poker_mgr->rand_shuffle();

	//获取目前库存对应的水位表
	int current_water = m_room->get_current_warter();
	SLOG_CRITICAL<<"current water ="<<current_water << "   value"<< m_room->get_total_stock_value();
	//取出压注最多的那个面板
	GOLD_TYPE temp_gold = 0;
	int record_index = -1;
	for (int i = 0; i < OTHERCOUNT; i++)
	{
		if (temp_gold < m_others[i]->get_player_bet_gold_panel() )
		{
			record_index = i;
			temp_gold = m_others[i]->get_player_bet_gold_panel();
		}
	}
	SLOG_CRITICAL<<"max players_bet_gold_panel  index="<<record_index;

	//预分析和交换pokes
	m_poker_mgr->pre_analysis_swap_pokes(current_water, record_index);

	m_room->clear_player_balane_info();

	//对比前两张牌 计算出赔钱
	const poker& poker_dragon = m_poker_mgr->get_poker(0);
	const poker& poker_tiger = m_poker_mgr->get_poker(1);
	DragonTiger_Cardodds::GetSingleton()->GetData("win_queal")->mValue;
	float temp_tax = m_room->get_winner_tax(m_room->RoomID->get_value());
	if (poker_dragon.m_poker_point == poker_tiger.m_poker_point)
	{
		//和牌赢 其它都输
		m_room->record_room_win_lose_info(equal_panel);
		float win_eqeal = DragonTiger_Cardodds::GetSingleton()->GetData("win_queal")->mValue;
		float dragon = DragonTiger_Cardodds::GetSingleton()->GetData("dragon")->mValue;
		float tiger = DragonTiger_Cardodds::GetSingleton()->GetData("tiger")->mValue;
		m_others[0]->win_dragon_tiger( m_room, temp_tax, win_eqeal);
 		m_others[1]->lose_dragon_tiger(m_room, 0, dragon);
 		m_others[2]->lose_dragon_tiger(m_room, 0, tiger);
	}
	else if (poker_dragon.m_poker_point > poker_tiger.m_poker_point )
	{
		//龙方赢 其它都输
		m_room->record_room_win_lose_info(dragon_panel);
		float win_dragon = DragonTiger_Cardodds::GetSingleton()->GetData("win_dragon")->mValue;
		float lose_tiger = DragonTiger_Cardodds::GetSingleton()->GetData("lose_tiger")->mValue;
 		m_others[0]->lose_dragon_tiger(m_room, 0, 1);
 		m_others[1]->win_dragon_tiger(m_room, temp_tax, win_dragon);
 		m_others[2]->lose_dragon_tiger(m_room, 0, lose_tiger);
	}
	else
	{
		//虎方赢 其它都输
		m_room->record_room_win_lose_info(tiger_panel);
		float win_tiger = DragonTiger_Cardodds::GetSingleton()->GetData("win_tiger")->mValue;
		float lose_dragon = DragonTiger_Cardodds::GetSingleton()->GetData("lose_dragon")->mValue;
 		m_others[0]->lose_dragon_tiger(m_room, 0, 1);
 		m_others[1]->lose_dragon_tiger(m_room, 0, lose_dragon);
 		m_others[2]->win_dragon_tiger(m_room, temp_tax, win_tiger);

	}
	
	m_room->sync_player_gold();
	
	set_game_state(logic_main::game_state::game_state_deal);

	auto sendmsg = PACKET_CREATE(packetl2c_bc_scene_deal_into, e_mst_l2c_bc_scene_deal_into);
	sendmsg->set_count_down(get_cd_time());
	//!!这里要修改消息协议把两张牌的信息发给客户端
	//目前客户端没有对接，没有修改这条消息

	m_room->broadcast_msg_to_client(sendmsg);
}

 void logic_main::computer_result()
 {
 	set_game_state(logic_main::game_state::game_state_result);
 
 	sync_bet_gold();
 	update_star_gold();
 
 	auto& players = m_room->get_players();
 	for (auto it = players.begin(); it != players.end(); it++)
 	{
 		auto sendmsg = PACKET_CREATE(packetl2c_bc_scene_result_into, e_mst_l2c_bc_scene_result_into);
 		sendmsg->set_count_down(get_cd_time());
 
 		auto result_info = sendmsg->mutable_result_info();
 		//消息协议需要修改
 		//fill_result_info(it->second, result_info);
 
 		it->second->send_msg_to_client(sendmsg);
 	}
 }
// 
// void logic_main::fill_result_info(LPlayerPtr& player, dragon_tiger_protocols::msg_result_info* result_info)
// {
// 	result_info->set_banker_win_gold(m_banker->get_last_win_gold());
// 	result_info->set_self_gold(player->get_gold());
// 	
// 	result_info->mutable_other_win_golds()->Reserve(4);
// 	GOLD_TYPE total_win_gold = 0;
// 	bool self_is_bet = false;
// 	for (unsigned int i = 0; i < m_others.size(); i++)
// 	{
// 		result_info->add_other_win_golds(m_others[i]->get_total_win_gold());
// 		GOLD_TYPE win_gold = m_others[i]->get_win_gold(player->get_pid());
// 		total_win_gold += win_gold;
// 		if (!self_is_bet && win_gold != 0)
// 		{
// 			self_is_bet = true;
// 		}
// 	}
// 	result_info->set_self_is_bet(self_is_bet);
// 	result_info->set_self_win_gold(total_win_gold);
// }


int logic_main::get_inc_id()
{
	return m_inc_id;
}
bool logic_main::can_leave_room(uint32_t player_id)
{

	return true;
}

void logic_main::leave_room(LPlayerPtr& player)
{
	//清空下注
	clear_bet_gold(player);

}

logic_room* logic_main::get_room()
{
	return m_room;
}

std::vector<logic_other*>& logic_main::get_others()
{
	return m_others;
}

logic_main::game_state logic_main::get_game_state()
{
	return m_game_state;
}

float logic_main::get_duration()
{
	return m_duration;
}

int logic_main::get_cd_time()
{
	return m_duration * 1000;
}

void logic_main::set_game_state(game_state state)
{
	m_game_state = state;
	switch (m_game_state)
	{
	case game_state::game_state_unknown:
		{
			m_duration = -1.0f;
		}
		break;
	case game_state::game_state_prepare:
		{
			static float pre_time = DragonTiger_BaseInfo::GetSingleton()->GetData("PreTime")->mValue;
			m_duration = pre_time;
		}
		break;
	case game_state::game_state_bet:
		{
			static float bet_time = DragonTiger_BaseInfo::GetSingleton()->GetData("BetTime")->mValue;
			m_duration = bet_time;
			m_sync_eleased = 0.0f;
			m_sync_interval = 1.0f;
		}
		break;
	case game_state::game_state_deal:
		{
			static float deal_time = DragonTiger_BaseInfo::GetSingleton()->GetData("DealTime")->mValue;
			m_duration = deal_time;
		}
		break;
	case game_state::game_state_result:
		{
			static float result_time = DragonTiger_BaseInfo::GetSingleton()->GetData("ResultTime")->mValue;
			m_duration = result_time;
		}
		break;
	default:
		break;
	}
}

void logic_main::kill_points(int32_t cutRound, bool status)
{
	m_kill_points_switch = status;
	if (m_kill_points_switch)
	{
		m_gametimes = abs(m_cfgCutRound);
		m_cfgCutRound = cutRound;
		SLOG_CRITICAL << "###### Manager Start kill points, cut round: " << cutRound << ", status: " << status;
	}
	else
	{
		m_gametimes = 0;
		m_cfgCutRound = 0;
		SLOG_CRITICAL << "###### Manager Stop kill points, cut round: " << cutRound << ", status: " << status;
	}
}

void logic_main::service_ctrl(int32_t optype)
{
	m_service_status = optype;
}

bool logic_main::cut_round_check()
{
	if (m_cfgCutRound != 0)
	{
		auto round = abs(m_cfgCutRound);
		if (++m_gametimes >= round)
		{
			m_gametimes = 0;
			m_cuttimes = global_random::instance().rand_int(0, round - 1);
		}
	}
	if (m_gametimes == m_cuttimes)
	{
		return true;
	}
	return false;
}
