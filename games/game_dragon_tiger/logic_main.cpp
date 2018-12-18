#include "stdafx.h"
#include "logic_main.h"
#include "logic_room.h"


#include "logic_other.h"
#include "logic_poker_mgr.h"
#include "logic_cards.h"
#include "logic_player.h"
#include "logic_robot.h"
#include "game_engine.h"
#include "DragonTiger_BaseInfo.h"
#include "DragonTiger_RoomCFG.h"
#include "DragonTiger_Cardodds.h"
#include "proc_dragon_tiger_logic.h"
#include "time_helper.h"
#include "logic_def.h"
#include "i_game_engine.h"
#include "i_game_ehandler.h"

#include "logic2world_msg_type.pb.h"


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
	m_cutround_tag = 0;
	m_winner_index = -1;

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
int logic_main::get_cut_round_tag()
{
	return m_cutround_tag;
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
			if (m_service_status == 100)
			{
				server_stop();
				set_game_state(game_state::game_state_unknown);
				return;
			}

			if (m_duration < 0)
			{
				m_room->update_playerlist();
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
				SLOG_CRITICAL << "result print!";
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
	check_player_status(false);

	set_game_state(logic_main::game_state::game_state_bet);

	int value = global_random::instance().rand_int(3000, 38000);

	m_room->set_equal_panel_gold(value);

	m_room->clear_player_balane_info();

	auto sendmsg = PACKET_CREATE(packetl2c_bc_scene_bet_into, e_mst_l2c_bc_scene_bet_into);
	sendmsg->set_count_down(get_cd_time());
	m_room->broadcast_msg_to_client(sendmsg);

	//复盘日志;
	m_dragon_tiger_log.reset();
	m_dragon_tiger_log = std::make_shared<logic2logsvr::DragonTigerGameLog>();

	if (!get_room()) return;

	m_dragon_tiger_log->set_begintime(time_helper::instance().get_cur_time());
	auto room_id = get_room()->get_room_id();
	auto child_id = get_room()->get_child_id();

	std::string now = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
	std::string id = std::to_string(room_id) + "_" + std::to_string(child_id) + "_" + now;
	m_dragon_tiger_log->set_gameroundindex(id);

	auto gameinfo = m_dragon_tiger_log->mutable_ginfo();
	gameinfo->set_gameid(game_engine::instance().get_gameid());
	gameinfo->set_roomid(get_room()->get_room_id());
	gameinfo->set_tableid(get_room()->get_child_id());

}

int logic_main::bet_gold(LPlayerPtr& player, uint32_t index, GOLD_TYPE gold)
{
	if (m_game_state != game_state::game_state_bet)
		return msg_type_def::e_msg_result_def::e_rmt_error_game_state;

	if (!player->get_room() || !player->get_room()->get_data())
		return msg_type_def::e_msg_result_def::e_rmt_unknow;

	if (player->get_gold() < player->get_room()->get_data()->mBetGoldCondition )
		return msg_type_def::e_msg_result_def::e_rmt_topup_not_enough;

	bool gold_full = false;
	if (index == 1)//龙
	{
		if (m_others[1]->get_total_bet_gold() > player->get_room()->get_data()->mBetAreaGoldCond[0]) gold_full = true;
	}
	else if(index == 2)//和牌
	{
		if (m_others[2]->get_total_bet_gold() > player->get_room()->get_data()->mBetAreaGoldCond[1]) gold_full = true;
	}
	else
	{
		if (m_others[3]->get_total_bet_gold() > player->get_room()->get_data()->mBetAreaGoldCond[2]) gold_full = true;
	}
	if (gold_full)
		return msg_type_def::e_msg_result_def::e_rmt_banker_betgold_is_full;

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

	if (index > 3 || index < 1) 
		return msg_type_def::e_msg_result_def::e_rmt_unknow;

	m_others[index]->bet_gold(player->get_pid(), gold);
	add_player_betgold(player, gold);

	player->set_balance_bet(1);
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

// void logic_main::update_star_gold()
// {
// 	for (auto it = m_bet_players.begin(); it != m_bet_players.end(); it++)
// 	{
// 		auto& player = m_room->get_player(it->first);
// 		if (player != nullptr)
// 		{
// 			//static double AwardGetRate = DragonTiger_BaseInfo::GetSingleton()->GetData("AwardGetRate")->mValue/10000.0f;
// 			//player->add_star_lottery_info(it->second*AwardGetRate, 0);
// 			player->quest_change(game_count, 1);
// 		}
// 	}
// }

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
		for (unsigned int i = 1; i < m_others.size(); i++)
		{
			sendmsg->add_bet_golds(m_others[i]->get_total_bet_gold());
			//SLOG_CRITICAL << "bet total gold i =" << i << "    total_gold" << m_others[i]->get_total_bet_gold();
		}
		m_room->broadcast_msg_to_client(sendmsg);
	}
}

void logic_main::deal_cards()
{
	m_cutround_tag = 0;

	m_poker_mgr->shuffle_total_pokes();

	m_poker_mgr->rand_shuffle();

	//取出压注最多的那个面板
	GOLD_TYPE temp_gold = 0;
	int max_index = 1;
	GOLD_TYPE dragon_gold = m_others[1]->get_player_bet_gold_panel();
	GOLD_TYPE tiger_gold = m_others[3]->get_player_bet_gold_panel();
	if (dragon_gold < tiger_gold)
		max_index = 3;

	if (cut_round_check())
	{
		if (m_cfgCutRound > 0 ) m_cutround_tag = 1;
		if (m_cfgCutRound < 0)  m_cutround_tag = 2;
		m_poker_mgr->pre_analysis_swap_pokes(max_index, m_cfgCutRound > 0 ? true : false);
	}

	float temp_tax = 0.0f;
	i_game_ehandler* ehander = game_engine::instance().get_handler();
	if (ehander)
	{
		if (m_room)
		{
			int roomid = m_room->get_id();
			int rate = ehander->GetRoomCommissionRate(roomid);
			temp_tax = rate / 100.0f;
		}
	}

	//对比前两张牌 计算出赔钱
	m_winner_index = -1;
	const poker& poker_dragon = m_poker_mgr->get_poker(0);
	const poker& poker_tiger = m_poker_mgr->get_poker(1);
	DragonTiger_Cardodds::GetSingleton()->GetData("win_queal")->mValue;
	//float temp_tax = m_room->get_winner_tax(m_room->RoomID->get_value());
	SLOG_CRITICAL << "poker  dragon = "<< poker_dragon.m_poker_point<<"    tiger= " << poker_tiger.m_poker_point;
	if (poker_dragon.m_poker_point == poker_tiger.m_poker_point)
	{
		SLOG_CRITICAL << "equal";
		//和牌赢 其它都输
		m_room->record_room_win_lose_info(equal_panel);
		m_winner_index = 2;
		float win_eqeal = DragonTiger_Cardodds::GetSingleton()->GetData("win_queal")->mValue;
		float dragon = DragonTiger_Cardodds::GetSingleton()->GetData("dragon")->mValue;
		float tiger = DragonTiger_Cardodds::GetSingleton()->GetData("tiger")->mValue;
		m_others[2]->win_dragon_tiger( m_room, temp_tax, win_eqeal);
 		m_others[1]->lose_dragon_tiger(m_room, 0, dragon);
 		m_others[3]->lose_dragon_tiger(m_room, 0, tiger);

	}
	else if (poker_dragon.m_poker_point > poker_tiger.m_poker_point )
	{
		SLOG_CRITICAL << "dragon win";
		//龙方赢 其它都输
		m_room->record_room_win_lose_info(dragon_panel);
		m_winner_index = 1;
		float win_dragon = DragonTiger_Cardodds::GetSingleton()->GetData("win_dragon")->mValue;
		float lose_tiger = DragonTiger_Cardodds::GetSingleton()->GetData("lose_tiger")->mValue;
 		m_others[3]->lose_dragon_tiger(m_room, 0, 1);
 		m_others[1]->win_dragon_tiger(m_room, temp_tax, win_dragon);
 		m_others[2]->lose_dragon_tiger(m_room, 0, lose_tiger);

	}
	else
	{
		SLOG_CRITICAL << "tiger win";
		//虎方赢 其它都输
		m_room->record_room_win_lose_info(tiger_panel);
		m_winner_index = 3;
		float win_tiger = DragonTiger_Cardodds::GetSingleton()->GetData("win_tiger")->mValue;
		float lose_dragon = DragonTiger_Cardodds::GetSingleton()->GetData("lose_dragon")->mValue;
 		m_others[2]->lose_dragon_tiger(m_room, 0, 1);
 		m_others[1]->lose_dragon_tiger(m_room, 0, lose_dragon);
 		m_others[3]->win_dragon_tiger(m_room, temp_tax, win_tiger);

	}
	
	//m_room->calc_player_tax(temp_tax);
 	cards_trend cardstrend; //牌路;
 	cardstrend.m_win_area = m_winner_index;
 	for (int i = 1; i < 4; ++i)
 	{
 		cardstrend.m_is_win[i-1] = m_others[i]->is_win();
 	}

	m_room->record_cards_trend(cardstrend);

	m_room->sync_player_gold();
	
	sync_bet_gold();
	set_game_state(logic_main::game_state::game_state_deal);

	auto sendmsg = PACKET_CREATE(packetl2c_bc_scene_deal_into, e_mst_l2c_bc_scene_deal_into);
	sendmsg->set_count_down(get_cd_time());
	auto dragon = sendmsg->add_cards_infos();
	dragon->set_pokers(poker_dragon.m_poker_point);
	dragon->set_cards_type((int)poker_dragon.m_poker_type);

	auto tiger = sendmsg->add_cards_infos();
	tiger->set_pokers(poker_tiger.m_poker_point);
	tiger->set_cards_type((int)poker_tiger.m_poker_type);

	m_room->broadcast_msg_to_client(sendmsg);

	for (unsigned int i = 1; i < m_others.size(); i++)
	{
		SLOG_CRITICAL << "bet total gold i =" << i << "    total_gold" << m_others[i]->get_total_bet_gold() / 100;
	}

	//复盘日志: 区域牌、下注信息;
	auto dragon_cards_info = m_dragon_tiger_log->add_cardsinfo();
	auto dragon_all_bet = m_others[1]->get_total_bet_gold();
	dragon_cards_info->set_pos(1);
	dragon_cards_info->set_sumbet(dragon_all_bet);
	dragon_cards_info->set_card_type((int)poker_dragon.m_poker_type);
	dragon_cards_info->set_cards(poker_dragon.m_poker_point);

	auto tiger_cards_info = m_dragon_tiger_log->add_cardsinfo();
	auto tiger_all_bet = m_others[3]->get_total_bet_gold();
	tiger_cards_info->set_pos(3);
	tiger_cards_info->set_sumbet(tiger_all_bet);
	tiger_cards_info->set_card_type((int)poker_tiger.m_poker_type);
	tiger_cards_info->set_cards(poker_tiger.m_poker_point);

	auto luckcards = m_dragon_tiger_log->add_cardsinfo();
	std::vector<poker> temp;
	auto luck_all_bet = m_others[2]->get_total_bet_gold();
	luckcards->set_pos(3);
	luckcards->set_sumbet(luck_all_bet);
	luckcards->set_card_type((int)poker_dragon.m_poker_type);
	luckcards->set_cards(poker_tiger.m_poker_point);


	//玩家信息;
	bool has_player = false;
	for (auto& p : m_bet_players)
	{
		auto pl = m_room->get_player(p.first);
		if (pl)
		{
			if (!pl->is_robot())
			{
				has_player = true;

				auto pinfo = m_dragon_tiger_log->add_pinfo();
				pinfo->set_pid(pl->get_pid());
				pinfo->set_goldbegin(pl->get_pre_gold());
				pinfo->set_goldend(pl->get_gold());
				pinfo->set_vargold(pl->get_win_lose_final_gold());
				pinfo->set_isrobot(pl->is_robot());
				pinfo->set_commission(pl->get_tax());

				for (auto& other : m_others)
				{
					auto bet = other->get_bet_gold(pl->get_pid());
					pinfo->add_bets(bet);
				}
			}
		}
	}
	SLOG_CRITICAL << m_dragon_tiger_log->DebugString();
	if (has_player)
	{
		m_dragon_tiger_log->set_endtime(time_helper::instance().get_cur_time());
		std::string strLog = m_dragon_tiger_log->SerializeAsString();
		game_engine::instance().get_handler()->sendLogInfo(strLog, game_engine::instance().get_gameid());
	}
}

 void logic_main::computer_result()
 {
 	set_game_state(logic_main::game_state::game_state_result);
 
 	sync_bet_gold();

 	//update_star_gold();
 	auto& players = m_room->get_players();
 	for (auto it = players.begin(); it != players.end(); it++)
 	{
 		auto sendmsg = PACKET_CREATE(packetl2c_bc_scene_result_into, e_mst_l2c_bc_scene_result_into);
 		sendmsg->set_count_down(get_cd_time());
 
 		auto result_info = sendmsg->mutable_result_info();
 		//消息协议需要修改
 		fill_result_info(it->second, result_info);
 
 		it->second->send_msg_to_client(sendmsg);
 	}


	m_room->clear_player_balane_info();
 }

void logic_main::fill_result_info(LPlayerPtr& player, dragon_tiger_protocols::msg_result_info* result_info)
{
	result_info->set_self_win_gold(player->get_win_lose_final_gold());
	result_info->set_self_gold(player->get_gold());
	result_info->set_self_is_bet(player->get_balance_bet());

	//客户端 1.龙 2.和 3.虎
	result_info->set_win_area(m_winner_index);
	for (int i = 1; i < 4; i++)
	{
		m_others[i]->others_win_info(player, m_room, result_info);
	}
	

}


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
		if (m_gametimes >= round)
		{
			m_gametimes = 0;
			m_cuttimes = 0;
		}
		if (m_cuttimes == 0)
		{
			m_cuttimes = global_random::instance().rand_int(1, round);;
		}
		if (++m_gametimes == m_cuttimes)
		{
			return true;
		}
	}
	return false;
}
void logic_main::server_stop()
{
	auto sendmsg = PACKET_CREATE(dragon_tiger_protocols::packetl2c_clean_out, dragon_tiger_protocols::e_mst_l2c_clean_out);
	if (sendmsg)
	{
		sendmsg->set_reason(e_msg_cleanout_def::e_cleanout_servicestop);
	}
	if (m_room)
	{
		auto players = m_room->get_players();
		for (auto& p : players)
		{
			if (p.second)
			{
				auto pl = p.second->getIGamePlayer();
				if (pl)
				{
					pl->send_msg_to_client(sendmsg);
					pl->reqPlayer_leaveGame();
				}
			}
		}
	}
}
void logic_main::check_player_status(bool server_stop)
{
	if (m_room)
	{
		auto& player_map = m_room->get_players();
		for (auto& item : player_map)
		{
			auto& player = item.second;
			if (player)
			{
				//断线检测;
				auto pl0 = player->getIGamePlayer();
				if (pl0)
				{
					if (pl0->get_state() == e_player_state::e_ps_disconnect)
					{
						pl0->reqPlayer_leaveGame();
					}
				}

				//记录当前玩家未下注的钱;
				player->record_gold();

				int status = player->get_kick_status();
				int leave_status = player->get_leave_status();
				if (status != 0 || leave_status != 0)
				{
					auto sendmsg = PACKET_CREATE(dragon_tiger_protocols::packetl2c_clean_out, dragon_tiger_protocols::e_mst_l2c_clean_out);
					if (sendmsg)
					{
						if (dragon_tiger_protocols::e_msg_cleanout_def_IsValid(status))
						{
							sendmsg->set_reason((dragon_tiger_protocols::e_msg_cleanout_def)status);
							player->send_msg_to_client(sendmsg);
							pl0->reqPlayer_leaveGame();
						}
					}
				}
			}
		}
	}
}