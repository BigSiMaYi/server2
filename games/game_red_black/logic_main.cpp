#include "stdafx.h"
#include "logic_main.h"
#include "logic_room.h"

#include "logic_other.h"
#include "logic_poker_mgr.h"
#include "logic_player.h"
#include "logic_robot.h"
#include "game_engine.h"
#include "i_game_ehandler.h"

#include "RedBlack_BaseInfo.h"
#include "RedBlack_RoomCFG.h"
#include "RedBlack_Cardodds.h"
#include "proc_red_black_logic.h"
#include "time_helper.h"
#include "logic_def.h"

#include "logic2world_msg_type.pb.h"

DRAGON_RED_BLACK_USING

logic_main::logic_main(logic_room* room)
{
	m_room = room;
	for (int i = 0; i < OTHERCOUNT; i++)
	{
		auto other = new logic_other(this);
		m_others.push_back(other);
	}
	m_poker_mgr = new logic_poker_mgr(global_random::instance().rand_int(0, 100000000));

	m_inc_id = 1;
	m_cutround_tag = 0;
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
	
	m_change_bet = false;

	auto sendmsg = PACKET_CREATE(packetl2c_bc_scene_prepare_into, e_mst_l2c_bc_scene_prepare_into);
	sendmsg->set_count_down(get_cd_time());
	//!!这里消息也要删除掉庄家信息
	m_room->broadcast_msg_to_client(sendmsg);
	m_room->init_robot_bet();
}

void logic_main::start_game()
{
	check_player_status(false);
	set_game_state(logic_main::game_state::game_state_bet);

	auto sendmsg = PACKET_CREATE(packetl2c_bc_scene_bet_into, e_mst_l2c_bc_scene_bet_into);
	sendmsg->set_count_down(get_cd_time());
	m_room->broadcast_msg_to_client(sendmsg);

	//复盘日志;
	m_cows_log.reset();
	m_cows_log = std::make_shared<logic2logsvr::RedBlackGameLog>();

	m_cows_log->set_begintime(time_helper::instance().get_cur_time());
	auto room_id = get_room()->get_room_id();
	auto child_id = get_room()->get_child_id();

	std::string now = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
	std::string id = std::to_string(room_id) + "_" + std::to_string(child_id) + "_" + now;
	m_cows_log->set_gameroundindex(id);

	auto gameinfo = m_cows_log->mutable_ginfo();
	gameinfo->set_gameid(game_engine::instance().get_gameid());
	gameinfo->set_roomid(get_room()->get_room_id());
	gameinfo->set_tableid(get_room()->get_child_id());
}

int logic_main::bet_gold(LPlayerPtr& player, uint32_t index, GOLD_TYPE gold)
{
	if (m_game_state != game_state::game_state_bet)
		return msg_type_def::e_msg_result_def::e_rmt_error_game_state;

	if (index >= m_others.size())
		return msg_type_def::e_msg_result_def::e_rmt_unknow;

	if (gold < 100)
		return msg_type_def::e_msg_result_def::e_rmt_unknow;

	//下注金额限制
	bool ret = false;

	//自己金钱是否足够
	GOLD_TYPE total_gold = get_player_betgold(player);
	ret = player->check_bet_gold(gold, total_gold);
	if (!ret)
		return msg_type_def::e_msg_result_def::e_rmt_other_betgold_is_full;

	if (index > 2 || index < 0) 
		return msg_type_def::e_msg_result_def::e_rmt_unknow;

	if (m_others[index]->get_total_bet_gold() + gold > get_room()->get_data()->mBetAreaGoldCond[index])
	{
		return msg_type_def::e_msg_result_def::e_rmt_banker_betgold_is_full;
	}

	m_others[index]->bet_gold(player->get_pid(), gold);
	add_player_betgold(player, gold);
	m_change_bet = true;

	return msg_type_def::e_msg_result_def::e_rmt_success;
}

void logic_main::clear_bet_gold(LPlayerPtr& player)
{
	GOLD_TYPE total_gold = get_player_betgold(player);
	clear_player_betgold(player);

	for (int i = 0; i < 3; i++)
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
	m_poker_mgr->shuffle_total_pokes();
	m_poker_mgr->rand_shuffle();

	//取出压注最多的那个面板;
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

	m_room->clear_player_balane_info();
	m_red_pokers.clear();
	m_black_pokers.clear();
	m_win_area.clear();

	m_poker_mgr->get_red_pokers(m_red_pokers);
	m_poker_mgr->get_black_pokers(m_black_pokers);

	exe_cut_round();

	m_red_type = cards_golden_type::cards_golden_unknown;
	m_black_type = cards_golden_type::cards_golden_unknown;
	win_camp win_camp_ = win_unknow;
	int duizi_value = 0;
	cards_golden_type win_golden_type = m_poker_mgr->compare_red_black_pokes(m_red_pokers, m_black_pokers, m_red_type, m_black_type, win_camp_, duizi_value);
	if (win_camp_ == win_unknow) return;
	m_card_type = (int)win_golden_type;
	//测试poke 每张牌、牌型、赢方;
	SLOG_CRITICAL<<"red pokes  a="<<m_red_pokers[0].m_poker_point<< " b= "<<m_red_pokers[1].m_poker_point <<" c="<<m_red_pokers[2].m_poker_point << " type =" <<(int)m_red_type;
	SLOG_CRITICAL<<"black pokes   a="<<m_black_pokers[0].m_poker_point << " b= "<<m_black_pokers[1].m_poker_point<< " c="<<m_black_pokers[2].m_poker_point << " type =" <<(int)m_black_type;
	SLOG_CRITICAL<<"winner camp = " << (int)win_camp_;

	//清除所有一局全部标记;
	m_room->clear_player_balane_info();

	float temp_tax = get_room_commission_rate();

	int red_index = win_red - 1;
	int black_index = win_black - 1;
	int luck_index = win_luck - 1;
	if (win_camp_ == win_red)//红方赢;
	{
		int poke_red_win = RedBlack_Cardodds::GetSingleton()->GetData("poke_red_win")->mValue;
		int poke_black_lose = RedBlack_Cardodds::GetSingleton()->GetData("poke_black_lose")->mValue;
		m_others[red_index]->win_red_black( m_room, temp_tax, poke_red_win);
		m_others[black_index]->lose_red_black(m_room, 0, poke_black_lose);
	}
	else //黑方赢;
	{
		int poke_black_win = RedBlack_Cardodds::GetSingleton()->GetData("poke_black_win")->mValue;
		int poke_red_lose = RedBlack_Cardodds::GetSingleton()->GetData("poke_red_lose")->mValue;
		m_others[black_index]->win_red_black( m_room, temp_tax, poke_black_win);
		m_others[red_index]->lose_red_black(m_room, 0, poke_red_lose);
	}
	m_win_area.push_back(win_camp_);
	
	switch (win_golden_type)
	{
	case  cards_golden_type::cards_golden_duizi:
		{
			if (duizi_value >= 9 || duizi_value == 1)
			{
				int poke_duizi = RedBlack_Cardodds::GetSingleton()->GetData("poke_duizi")->mValue;
				m_others[luck_index]->win_red_black( m_room, temp_tax, poke_duizi);
				m_win_area.push_back(win_luck);
			}
			else
			{
				m_others[luck_index]->lose_red_black(m_room, 0, 1);//豹子，金花面板输钱1:1输
			}
		}
		break;
	case  cards_golden_type::cards_golden_shunzi:
		{
			int poke_shunzi = RedBlack_Cardodds::GetSingleton()->GetData("poke_shunzi")->mValue;
			m_others[luck_index]->win_red_black( m_room, temp_tax, poke_shunzi);
			m_win_area.push_back(win_luck);
		}
		break;
	case  cards_golden_type::cards_golden_jinhua:
		{
			int poke_jinhua = RedBlack_Cardodds::GetSingleton()->GetData("poke_jinhua")->mValue;
			m_others[luck_index]->win_red_black( m_room, temp_tax, poke_jinhua);
			m_win_area.push_back(win_luck);
		}
		break;
	case  cards_golden_type::cards_golden_shunjin:
		{
			int poke_shunjin = RedBlack_Cardodds::GetSingleton()->GetData("poke_shunjin")->mValue;
			m_others[luck_index]->win_red_black( m_room, temp_tax, poke_shunjin);
			m_win_area.push_back(win_luck);
		}
		break;
	case  cards_golden_type::cards_golden_baozi:
		{
			int poke_baozi = RedBlack_Cardodds::GetSingleton()->GetData("poke_baozi")->mValue;
			m_others[luck_index]->win_red_black( m_room, temp_tax, poke_baozi);
			m_win_area.push_back(win_luck);
		}
		break;
	default:
		{
			m_others[luck_index]->lose_red_black(m_room, 0, 1);//豹子，金花面板输钱1:1输
		}
		break;
	}

	cards_trend cardstrend; //牌路;
	cardstrend.m_win_area = win_camp_;
	cardstrend.m_card_type = (int)win_golden_type;
	for (int i  = 0; i < m_others.size(); ++i)
	{
		cardstrend.m_is_win[i] = m_others[i]->is_win();
	}

	
	if (m_history_infos.size() > 300)
	{
		m_history_infos.pop_back();
	}
	m_history_infos.push_front(cardstrend);

	m_room->record_room_win_lose_info(cardstrend);

	m_room->sync_player_gold();
	sync_bet_gold();
	set_game_state(logic_main::game_state::game_state_deal);

	auto sendmsg = PACKET_CREATE(packetl2c_bc_scene_deal_into, e_mst_l2c_bc_scene_deal_into);
	sendmsg->set_count_down(get_cd_time());
	auto cards_info1 = sendmsg->add_cards_infos();
	fill_cards_info(cards_info1, m_red_pokers, (int)m_red_type);
	auto cards_info2 = sendmsg->add_cards_infos();
	fill_cards_info(cards_info2, m_black_pokers, (int)m_black_type);
	//!!这里要修改消息协议把两张牌的信息发给客户端;
	//目前客户端没有对接，没有修改这条消息;
	
	m_room->broadcast_msg_to_client(sendmsg);

	//复盘日志: 区域牌、下注信息;
	auto redcards = m_cows_log->add_cardsinfo();
	auto red_all_bet = m_others[red_index]->get_total_bet_gold();
	fill_cards_info2(redcards, m_red_pokers, (int)m_red_type, red_index, red_all_bet);

	auto blackcards = m_cows_log->add_cardsinfo();
	auto black_all_bet = m_others[black_index]->get_total_bet_gold();
	fill_cards_info2(blackcards, m_black_pokers, (int)m_black_type, black_index, black_all_bet);

	auto luckcards = m_cows_log->add_cardsinfo();
	std::vector<poker> temp;
	auto luck_all_bet = m_others[luck_index]->get_total_bet_gold();
	fill_cards_info2(luckcards, temp, 0, luck_index, luck_all_bet);
	//玩家信息;
	bool has_player = false;
	for (auto& p : m_bet_players)
	{
		auto pl = m_room->get_player(p.first);
		if (pl)
		{
			if (!pl->is_robot())
			{
				auto pinfo = m_cows_log->add_pinfo();
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

				has_player = true;
			}
		}
	}
	m_cows_log->set_endtime(time_helper::instance().get_cur_time());
	SLOG_CRITICAL << m_cows_log->DebugString();
	if (has_player)
	{
		std::string strLog = m_cows_log->SerializeAsString();
		game_engine::instance().get_handler()->sendLogInfo(strLog, game_engine::instance().get_gameid());
	}
}

 void logic_main::computer_result()
 {
 	set_game_state(logic_main::game_state::game_state_result);
 
 	//sync_bet_gold();

 	auto& players = m_room->get_players();
 	for (auto it = players.begin(); it != players.end(); it++)
 	{
 		auto sendmsg = PACKET_CREATE(packetl2c_bc_scene_result_into, e_mst_l2c_bc_scene_result_into);
 		sendmsg->set_count_down(get_cd_time());
		
 		auto result_info = sendmsg->mutable_result_info();

		fill_result_info(it->second, result_info);

 		it->second->send_msg_to_client(sendmsg);
 	}
 }

int logic_main::get_inc_id()
{
	return m_inc_id;
}

bool logic_main::can_leave_room(uint32_t player_id)
{
	auto it = m_bet_players.find(player_id);
	if (it != m_bet_players.end())
	{
		return false;
	}
	return true;
}

void logic_main::leave_room(LPlayerPtr& player)
{
	//清空下注
	clear_bet_gold(player);
}

double logic_main::get_room_commission_rate()
{
	double rate_val = 0;
	i_game_ehandler* ehander = game_engine::instance().get_handler();
	if (ehander)
	{
		if (m_room)
		{
			int roomid = m_room->get_id();
			int rate = ehander->GetRoomCommissionRate(roomid);
			rate_val = (double)rate / 100;
		}
	}
	return rate_val;
}

void logic_main::fill_result_info(LPlayerPtr& player, red_black_protocols::msg_result_info* result_info)
{
	int nSize = m_bet_players.size();
	result_info->mutable_others_win_info()->Reserve(nSize);
	auto wininfo = result_info->mutable_win_info();
	for (int i = 0; i < m_win_area.size(); ++i)
	{
		wininfo->add_win_area(m_win_area[i]);
	}

	result_info->set_self_win_gold(player->get_win_lose_final_gold());
	result_info->set_self_gold(player->get_gold());
	result_info->set_self_is_bet(player->get_balance_bet());
	
	wininfo->set_card_type(m_card_type);

	for (auto& it2 : m_bet_players)
	{
		auto player = m_room->get_player(it2.first);
		if (player)
		{
			auto other_win_info = result_info->add_others_win_info();
			other_win_info->set_player_id(it2.first);
			other_win_info->set_player_name(player->get_nickname());
			other_win_info->set_player_gold(player->get_gold());
			other_win_info->set_player_region(player->get_region());
			other_win_info->set_player_win_gold(player->get_win_lose_final_gold());
		}
	}
}

void logic_main::fill_cards_info(red_black_protocols::msg_cards_info* cards_info, std::vector<poker>& poke, int card_type)
{
	cards_info->set_cards_type(card_type);
	for (auto& p : poke)
	{
		cards_info->add_pokers(p.m_id);
	}
}

void logic_main::fill_cards_info2(logic2logsvr::BetAreaCardsInfo* cards_info, std::vector<poker>& poke, int card_type, int pos, GOLD_TYPE bet)
{
	cards_info->set_card_type(card_type);
	cards_info->set_pos(pos);
	cards_info->set_sumbet(bet);

	for (auto& p : poke)
	{
		cards_info->add_cards(p.m_id);
	}
}

void logic_main::fill_scene_info(red_black_protocols::msg_scene_info* scene_info)
{
	auto cards_info1 = scene_info->add_cards_infos();
	fill_cards_info(cards_info1, m_red_pokers, (int)m_red_type);
	auto cards_info2 = scene_info->add_cards_infos();
	fill_cards_info(cards_info2, m_black_pokers, (int)m_black_type);
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
			static float pre_time = RedBlack_BaseInfo::GetSingleton()->GetData("PreTime")->mValue;
			m_duration = pre_time;
		}
		break;
	case game_state::game_state_bet:
		{
			static float bet_time = RedBlack_BaseInfo::GetSingleton()->GetData("BetTime")->mValue;
			m_duration = bet_time;
			m_sync_eleased = 0.0f;
			m_sync_interval = 1.0f;
		}
		break;
	case game_state::game_state_deal:
		{
			static float deal_time = RedBlack_BaseInfo::GetSingleton()->GetData("DealTime")->mValue;
			m_duration = deal_time;
		}
		break;
	case game_state::game_state_result:
		{
			static float result_time = RedBlack_BaseInfo::GetSingleton()->GetData("ResultTime")->mValue;
			m_duration = result_time;
		}
		break;
	default:
		break;
	}
}

void logic_main::exe_cut_round()
{
	m_cutround_tag = 0;
	if (cut_round_check(m_cfgCutRound))
	{
		GOLD_TYPE black_bet_gold = m_others[0]->get_player_bet_gold_panel();
		GOLD_TYPE red_bet_gold = m_others[2]->get_player_bet_gold_panel();

		win_camp win_type = win_unknow;
		int duizi_val = 0;
		cards_golden_type win_golden_type = m_poker_mgr->compare_red_black_pokes(m_red_pokers, m_black_pokers, m_red_type, m_black_type, win_type, duizi_val);
		if (win_type == win_unknow) return;

		if (m_cfgCutRound > 0) //m_cfgCutRound 杀分;
		{
			m_cutround_tag = 1;
			if (win_type == win_red)
			{
				if (red_bet_gold > black_bet_gold)
				{
					std::swap(m_red_pokers, m_black_pokers);
				}
			}
			else
			{
				if (black_bet_gold > red_bet_gold)
				{
					std::swap(m_red_pokers, m_black_pokers);
				}
			}
		}
		else// if (m_cfgCutRound < 0)
		{
			m_cutround_tag = 2;
			if (win_type == win_red)
			{
				if (red_bet_gold < black_bet_gold)
				{
					std::swap(m_black_pokers, m_red_pokers);
				}
			}
			else
			{
				if (black_bet_gold < red_bet_gold)
				{
					std::swap(m_black_pokers, m_red_pokers);
				}
			}
		}
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

bool logic_main::cut_round_check(int cut_round)
{
	if (cut_round != 0)
	{
		auto round = abs(cut_round);
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
	auto sendmsg = PACKET_CREATE(red_black_protocols::packetl2c_clean_out, red_black_protocols::e_mst_l2c_clean_out);
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

int logic_main::get_cut_round_tag()
{
	return m_cutround_tag;
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
					auto sendmsg = PACKET_CREATE(red_black_protocols::packetl2c_clean_out, red_black_protocols::e_mst_l2c_clean_out);
					if (sendmsg)
					{
						if (status != 0)
						{
							if (!red_black_protocols::e_msg_cleanout_def_IsValid(status))
							{
								status = e_cleanout_gm_kick;
							}
							sendmsg->set_reason((red_black_protocols::e_msg_cleanout_def)status);
							player->send_msg_to_client(sendmsg);
							pl0->reqPlayer_leaveGame();
						}
					}
				}
			}
		}
	}
}

std::vector<GOLD_TYPE> logic_main::get_robot_bet_area_gold()
{
	std::vector<GOLD_TYPE> betAraeGold;

	for (auto& it : m_others)
	{
		betAraeGold.push_back(it->get_total_bet_gold() - it->get_player_bet_gold_panel()); //只取出机器人的下注值;
	}
	return std::move(betAraeGold);
}

std::vector<float> logic_main::calc_bet_area_rate()
{
	double r = global_random::instance().rand_double(0.05, 0.08);
	std::vector<float> rates;

	if (m_history_infos.size() < 3)
	{
		rates.push_back((1.0 - r) / 2);
		rates.push_back(r);
		rates.push_back((1.0 - r) / 2);
	}
	else
	{
		std::map<int, int> rateMap;
		int index = 3;
		for (auto& item : m_history_infos)
		{
			int size = sizeof(item.m_is_win) / sizeof(item.m_is_win[0]);
			for (int i = 0; i < size; ++i)
			{
				if (i != 1)
				{
					if (item.m_is_win[i])
					{
						rateMap[i] -= 2;
					}
					else
					{
						rateMap[i] += 2;
					}
				}
			}
			if (--index <= 0)
			{
				break;
			}
		}

		int sum = 0;
		for (auto& m : rateMap)
		{
			m.second += (100 - 100.0 * r) / 2;
			sum += m.second;
		}

		float luck_val = (sum / (1.0 - r)) * r + 0.5;
		rateMap[1] = luck_val;
		sum += luck_val;

		for (auto& m : rateMap)
		{
			float rate = (float)m.second / sum;
			rates.push_back(rate);
		}
	}
	return rates;
}

//////////////////////////////////////////////////////////////////////////

//汇总每个玩家的区域下注，再计算税收;
void logic_main::calc_player_rax()
{
	double rate = m_room->get_rate();
	auto& players = m_room->get_players();

	for (auto it = players.begin(); it != players.end(); it++)
	{
		GOLD_TYPE total_win_gold = 0;
		bool self_is_bet = false;
		auto player_ptr = it->second;
		//统计玩家在所有区域的下注输赢总和;
		for (unsigned int i = 0; i < m_others.size(); i++)
		{
			bool is_bet = false;
			GOLD_TYPE win_gold = m_others[i]->get_win_gold(it->second->get_pid(), is_bet);
			total_win_gold += win_gold;
			if (is_bet)
			{
				self_is_bet = true;
			}
		}
		int64_t rax_gold = 0;
		if (total_win_gold > 0)
		{
			rax_gold = std::ceil(total_win_gold * rate);
			total_win_gold -= rax_gold;
		}

		player_ptr->set_balance_bet(self_is_bet);
		//player_ptr->set_self_win_gold(total_win_gold);

		//闲家下注税收是每个区域的输赢总和，前面change_gold 处理单个区域没有扣税;
		//player_ptr->set_tax(rax_gold);
	}
}

int logic_main::get_poker_rate(cards_golden_type win_golden_type, int poker_value)
{
	int poker_rate = 0;
	switch (win_golden_type)
	{
	case  cards_golden_type::cards_golden_duizi:
	{
		if (poker_value >= 9 || poker_value == 1)
		{
			poker_rate = RedBlack_Cardodds::GetSingleton()->GetData("poke_duizi")->mValue;
		}
		else
		{
			poker_rate = 1;
		}
		break;
	}

	case  cards_golden_type::cards_golden_shunzi:
	{
		poker_rate = RedBlack_Cardodds::GetSingleton()->GetData("poke_shunzi")->mValue;
		break;
	}

	case  cards_golden_type::cards_golden_jinhua:
	{
		poker_rate = RedBlack_Cardodds::GetSingleton()->GetData("poke_jinhua")->mValue;
		break;
	}

	case  cards_golden_type::cards_golden_shunjin:
	{
		poker_rate = RedBlack_Cardodds::GetSingleton()->GetData("poke_shunjin")->mValue;
		break;
	}

	case  cards_golden_type::cards_golden_baozi:
	{
		poker_rate = RedBlack_Cardodds::GetSingleton()->GetData("poke_baozi")->mValue;
		break;
	}

	default:
	{
		poker_rate = 1;
		break;
	}
	}//switch

	return poker_rate;
}
