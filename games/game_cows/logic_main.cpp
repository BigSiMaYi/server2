#include "stdafx.h"
#include "logic_main.h"
#include "logic_room.h"

#include "logic_banker.h"
#include "logic_other.h"
#include "logic_poker_mgr.h"
#include "logic_cards.h"
#include "logic_player.h"
#include "logic_robot.h"

#include "Cows_BaseInfo.h"
#include "Cows_RoomCFG.h"

#include "proc_cows_logic.h"
#include "proc_cows_protocol.h"
#include "time_helper.h"
#include "logic2logsvr_msg_type.pb.h"

#include <cmath>
#include <random>

COWS_SPACE_USING

logic_main::logic_main(logic_room* room)
{
	//初始化机器人上庄和玩家上庄发牌概率
	m_service_status = 0;
	m_kill_points_switch = false;
	auto cows_base_inst = Cows_BaseInfo::GetSingleton();
	std::string robot_str = "RobotBankerProbsCards";
	std::string player_str = "PlayerBankerProbsCards";
	for (int i = 0; i < 5; ++i)
	{
		std::string id = std::to_string(i);
		auto robot_probos = cows_base_inst->GetData(robot_str+id)->mValue;
		m_robot_banker_probs.push_back((double)robot_probos /100);

		auto p_probos = cows_base_inst->GetData(player_str + id)->mValue;
		m_player_banker_probs.push_back((double)p_probos / 100);
	}

	m_room = room;
	int mode = Cows_BaseInfo::GetSingleton()->GetData("RobotStartMode")->mValue;
	if (room && mode == 1)
	{
		room->request_robot();
	}

	m_banker = new logic_banker();
	for (int i = 0; i < OTHERCOUNT; i++)
	{
		auto other = new logic_other(this);
		m_others.push_back(other);
	}
	m_poker_mgr = new logic_poker_mgr(global_random::instance().rand_int(0, 100000000));
	clear_history();
	clear_snatch_player();
	m_log_time = 0;
	m_inc_id = 1;

	set_game_state(game_state::game_state_unknown);
}

logic_main::~logic_main(void)
{
	SAFE_DELETE(m_banker);
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
		if (m_service_status == 200)
		{
			//set_game_state(game_state::game_state_prepare);
		}
		break;
	case game_state::game_state_prepare:
		if (m_service_status == 100)
		{
			set_game_state(game_state::game_state_pause);
		}
		else
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
			//检查是否有上庄的机器人或者玩家
			if (check_banker())
			{
				pre_start_game();
			}
			else
			{
				set_game_state(game_state::game_state_result);
			}
		}
		break;
	case game_state::game_state_pause:
		server_stop();
		set_game_state(game_state::game_state_unknown);
		break;
	default:
		break;
	}
}

void logic_main::test_game()
{
	static int32_t total_gold = 0;
	static int32_t total_count = 0;
	int32_t total_win_gold[4] = {0, 0, 0, 0};
	int test_count = 1;
	for (int j = 0; j < test_count; j++)
	{
		total_count++;
		m_banker->get_cards()->clear_cards();
		for (int i = 0; i < OTHERCOUNT; i++)
		{
			m_others[i]->get_cards()->clear_cards();
		}
		logic2logsvr::CowsProfitInfo profitInfo;
		int earn_type = m_room->get_earn_type(0, profitInfo);
		if (earn_type == -2)
		{
			m_poker_mgr->banker_win_shuffle();
		}
		else if(earn_type == -1)
		{
			m_poker_mgr->banker_power_shuffle();
		}
		else if (earn_type == 1)
		{
			m_poker_mgr->other_power_shuffle();
		}
		else
		{
			m_poker_mgr->rand_shuffle();
		}
		m_poker_mgr->deal(m_banker->get_cards());
		for (int i = 0; i < OTHERCOUNT; i++)
		{
			m_poker_mgr->deal(m_others[i]->get_cards());
		}
		//结果
		int32_t banker_win_gold = 0;
		int32_t total_bet_gold = 4000;
		for (int i = 0; i < OTHERCOUNT; i++)
		{
			bool result = logic_cards::compare(m_banker->get_cards(), m_others[i]->get_cards());
			int32_t win_gold = 0;
			//庄家赢
			if (result)
			{
				win_gold = 1000*m_banker->get_cards()->get_cards_rate();
				banker_win_gold += win_gold;
			}
			//闲家赢
			else
			{
				win_gold = -1000*m_others[i]->get_cards()->get_cards_rate();
				banker_win_gold += win_gold;
			}		
			m_room->log_game_single_info(i, win_gold, !result);
		}
		m_room->log_game_info(banker_win_gold, total_bet_gold);
	}
	//printf("total count = %I64d \n", total_count);
	m_room->print_bet_win();
	m_room->store_game_object(true);
}

//@
void logic_main::stop_game()
{
	set_game_state(game_state::game_state_unknown);
}

//@空闲时间;
void logic_main::pre_start_game()
{
	m_inc_id++;
	set_game_state(game_state::game_state_prepare);

	clear_last_data();
	//check_banker(); //在前面提前检查;
	m_banker->start_game();
	for (int i = 0; i < OTHERCOUNT; i++)
	{
		m_others[i]->start_game();
	}
	m_room->init_robot_bet();
	m_change_bet = false;

	auto sendmsg = PACKET_CREATE(packetl2c_bc_scene_prepare_into, e_mst_l2c_bc_scene_prepare_into);
	sendmsg->set_count_down(get_cd_time());
	auto banker_info = sendmsg->mutable_banker_info();
	auto player_info = banker_info->mutable_player_info();
	get_banker()->fill_player_info(player_info);
	banker_info->set_max_bet_gold(get_banker()->get_max_bet_gold());
	banker_info->set_can_snatch(can_snatch());
	banker_info->set_snatch_gold(get_snatch_gold());
	banker_info->set_snatch_player_id(get_snatch_player());
	m_room->broadcast_msg_to_client(sendmsg);
}

//@开始下注阶段;
void logic_main::start_game()
{
	m_rebots_bet.clear();//需要清空;
	m_player_bet.clear();
	//@1.0复盘日志：记录gameinfo;
	m_cows_log.reset();
	m_cows_log = std::make_shared<logic2logsvr::CowsGameLog>();
	
	m_cows_log->set_begintime(time_helper::instance().get_cur_time());
	auto room_id = get_room()->get_room_id();
	auto child_id = get_room()->get_child_id();
	std::string id = std::to_string(room_id)+ "_" +std::to_string(child_id) + "_" + std::to_string(m_total_count);
	m_cows_log->set_gameroundindex(id);

	auto gameinfo = m_cows_log->mutable_ginfo();
	gameinfo->set_gameid(game_engine::instance().get_gameid());
	gameinfo->set_roomid(get_room()->get_room_id());
	gameinfo->set_tableid(get_room()->get_child_id());

	//检测是否存在需要踢出房间的玩家;
	check_player_status(false);
	set_game_state(logic_main::game_state::game_state_bet);

	auto sendmsg = PACKET_CREATE(packetl2c_bc_scene_bet_into, e_mst_l2c_bc_scene_bet_into);
	sendmsg->set_count_down(get_cd_time());
	m_room->broadcast_msg_to_client(sendmsg);

	//@发送庄家状态至world,直到游戏结算庄家不能取钱;
	//@用户下注或上庄则判定为游戏状态;
	//@add by Hunter 2017/08/26;
	//@同步用户状态;
	//if (!m_banker->is_robot()) 
	{
		//@进入房间同步状态;
		auto geh = game_engine::instance().get_handler();
		if (geh)
		{
			auto pid = m_banker->get_player_id();
			geh->sync_userGameStatus(m_banker->get_player_id(),
				game_engine::instance().get_gameid(),
				m_room->get_room_id(),
				0,
				0,
				2
			);
		}
	}
}

int logic_main::bet_gold(LPlayerPtr& player, uint32_t index, GOLD_TYPE gold)
{
	if (m_game_state != game_state::game_state_bet)
		return msg_type_def::e_msg_result_def::e_rmt_error_game_state;

	if (m_banker->is_banker(player->get_pid()))
		return msg_type_def::e_msg_result_def::e_rmt_unknow;

	if (index >= m_others.size())
		return msg_type_def::e_msg_result_def::e_rmt_unknow;

	//下注小于100无效;
	if (gold < 100)
		return msg_type_def::e_msg_result_def::e_rmt_unknow;

	//下注金额限制
	bool ret = m_banker->check_bet_gold(gold);
	if (!ret)
		return msg_type_def::e_msg_result_def::e_rmt_banker_betgold_is_full;

	//自己金钱是否足够
	GOLD_TYPE total_gold = get_player_betgold(player);
	ret = player->check_bet_gold(gold, total_gold);
	if (!ret)
		return msg_type_def::e_msg_result_def::e_rmt_other_betgold_is_full;

	m_banker->bet_gold(gold);
	m_others[index]->bet_gold(player->get_pid(), gold);
	add_player_betgold(player, gold);
	m_change_bet = true;

	//记录机器人每个区域的下注;
	if (player->is_robot())
	{
		m_rebots_bet[index] += gold;
	}
	else
	{
		m_player_bet[index] += gold;
	}

	//@用户下注或上庄则判定为游戏状态;
	//@add by Hunter 2017/08/26;
	//@同步用户状态;
	if (!player->is_robot()) {
		//@进入房间同步状态;
		auto geh = game_engine::instance().get_handler();
		if (geh)
		{
			auto pid = player->get_pid();
			geh->sync_userGameStatus(player->get_pid(),
				game_engine::instance().get_gameid(),
				m_room->get_room_id(),
				0,
				0,
				2
			);
		}
	}

	return msg_type_def::e_msg_result_def::e_rmt_success;
}

void logic_main::clear_bet_gold(LPlayerPtr& player)
{
	GOLD_TYPE total_gold = get_player_betgold(player);
	clear_player_betgold(player);

	m_banker->clear_bet_gold(total_gold);
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
			static double AwardGetRate = Cows_BaseInfo::GetSingleton()->GetData("AwardGetRate")->mValue/10000.0f;
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

void sort_cards(std::vector<std::pair<int, logic_cards*> >& cards_vec)
{
	for (int i = 0; i < cards_vec.size(); ++i)
	{
		for (int j = 0; j < cards_vec.size() - i - 1; ++j)
		{
			if (!logic_cards::compare(cards_vec[j].second, cards_vec[j + 1].second))
			{
				std::swap(cards_vec[j], cards_vec[j + 1]);
			}
		}
	}

	std::stringstream strstream;
	strstream << "cards order: ";
	for (auto& item : cards_vec)
	{
		strstream << item.first << " > ";
	}
	SLOG_CRITICAL << strstream.str();
}

double rand_float(double min, double max)
{
	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_real_distribution<> d(min, max);
	auto v = d(gen);
	return v;
}

int card_choose(std::vector<float>& probs)
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

void logic_main::swap_cards(std::vector<std::pair<int, logic_cards*> >& cards_vec, int swap_type)
{
	sort_cards(cards_vec);

	int banker_index = 0;
	for (auto& item : cards_vec)
	{
		if (item.first == 0)
		{
			break;
		}
		++banker_index;
	}

	int index = -1;
	if (swap_type == 1)//玩家庄家;
	{
		index = card_choose(m_player_banker_probs);
	}
	else if(swap_type == 2)//机器人庄家;
	{
		index = card_choose(m_robot_banker_probs);
	}
	if (index  != -1)
	{
		std::swap(cards_vec[banker_index].second, cards_vec[index].second);
		SLOG_CRITICAL << "swap banker cards: " << ", banker index: " << banker_index << ", swap index: " << index;
	}
	sort_cards(cards_vec);

	for (auto& item : cards_vec)
	{
		if (item.first == 0)
		{
			m_banker->set_cards(item.second);
		}
		else
		{
			m_others[item.first - 1]->set_cards(item.second);
		}
	}
}

int cows_space::logic_main::pre_game_win_rate()
{
	if (m_history_infos.empty())
	{
		return 0;
	}
	history_info& info = m_history_infos.front();
	if ( info.m_is_win[0] ==false
		&& info.m_is_win[1] == false
		&& info.m_is_win[2] == false
		&& info.m_is_win[3] == false)
	{
		return -1;
	}
	if (info.m_is_win[0] == true
		&& info.m_is_win[1] == true
		&& info.m_is_win[2] == true
		&& info.m_is_win[3] == true)
	{
		return 1;
	}
	return 0;
}

int cows_space::logic_main::cur_game_wind_rate()
{
	int win_count = 0;
	int other_size = m_others.size();
	for (unsigned int i = 0; i < other_size; i++)
	{
		bool banker_win = logic_cards::compare(m_banker->get_cards(), m_others[i]->get_cards());
		//bool other_win = m_others[i]->is_win();
		if (!banker_win)
		{
			++win_count;
		}
		else
		{
			--win_count;
		}
	}
	if (win_count == other_size)
	{
		return 1;
	}
	else if(win_count == -other_size)
	{
		return -1;
	}
	return 0;
}

void logic_main::deal_cards()
{
	int swap_type = 0;
	if (m_poker_mgr->check_gm_shuffle())
	{
	}
	else
	{
		//复盘日志;
		auto profitInfo = m_cows_log->mutable_profitinfo();
		//玩家上庄: 无论是否玩家上庄都需要判断当前系统是否输赢;
		if (m_banker->is_system_banker() == false)
		{
			SLOG_CRITICAL << "********************************player is banker***********************************";
			if (m_rebots_bet.empty())
			{
				SLOG_CRITICAL << "###### Don't kill points, player is banker, no robots bet";
				m_poker_mgr->rand_shuffle();
			}
			else
			{
				int earn_type = m_room->get_earn_type(m_banker->get_cur_bet_gold(), *profitInfo);
				if (earn_type < 0)
				{
					m_poker_mgr->other_power_shuffle();
					swap_type = 1;
				}
				else
				{
					m_poker_mgr->rand_shuffle();
				}
				if (m_kill_points_switch)
				{
					//100局杀分概率;
					int cut_round = m_room->get_cut_round();
					int val = global_random::instance().rand_int(1, 10000);
					if (val <= cut_round || earn_type == -1)
					{
						swap_type = 1;
						profitInfo->set_killpointstatus(2);
					}
					SLOG_CRITICAL << "###### Manager Start kill points, cut round: "<< cut_round <<", rand value: "<< val << ", earn_type: " << earn_type;
				}
			}
		}
		else
		{
			SLOG_CRITICAL << "********************************robot is banker***********************************";
			if (m_player_bet.empty())
			{
				SLOG_CRITICAL << "###### Don't kill points, robot is banker, no players bet";
				m_poker_mgr->rand_shuffle();
			}
			else
			{
				int earn_type = m_room->get_earn_type(m_banker->get_cur_bet_gold(), *profitInfo);
				if (earn_type == -2)
				{
					m_poker_mgr->banker_win_shuffle();
					swap_type = 2;
				}
				else if (earn_type == -1)
				{
					m_poker_mgr->banker_power_shuffle();
					swap_type = 2;
				}
				else if (earn_type == 1)
				{
					m_poker_mgr->other_power_shuffle();
				}
				else
				{
					m_poker_mgr->rand_shuffle();
				}
				if (m_kill_points_switch)
				{
					//10000局杀分概率;
					int cut_round = m_room->get_cut_round();
					int val = global_random::instance().rand_int(1, 10000);
					if (val <= cut_round || earn_type == -1)
					{
						swap_type = 2;
						profitInfo->set_killpointstatus(2);
					}
					SLOG_CRITICAL << "###### Manager Start kill points, cut round: " << cut_round << ", rand value: " << val <<", earn_type: "<< earn_type;
				}
			}
		}
	}

	m_poker_mgr->rand_shuffle();

	//发牌
	std::vector<std::pair<int, logic_cards*> > cards_vec;	
	std::vector<int> card_rand = {0, 1, 2, 3, 4};
	std::random_shuffle(card_rand.begin(), card_rand.end());

	for (int i = 0; i < OTHERCOUNT + 1; ++i)
	{
		int j = card_rand[i];
		if (j==0)
		{
			auto banker_cards = m_banker->get_cards();
			m_poker_mgr->deal(banker_cards);
			cards_vec.push_back(std::make_pair(j, banker_cards));
		}
		else
		{
			auto other_cards = m_others[j-1]->get_cards();
			m_poker_mgr->deal(other_cards);
			cards_vec.push_back(std::make_pair(j, other_cards));
		}
	}

	if (swap_type != 0)
	{
		swap_cards(cards_vec, swap_type);

		int pre_win_rate = pre_game_win_rate();
		int cur_wind_rate = cur_game_wind_rate();
		if (pre_win_rate == cur_wind_rate && cur_wind_rate != 0)
		{
			//出现连续通杀通赔 随机;
			swap_cards(cards_vec, swap_type);
		}
	}
	else
	{
		int pre_win_rate = pre_game_win_rate();
		int cur_wind_rate = cur_game_wind_rate();
		if (pre_win_rate == cur_wind_rate && cur_wind_rate != 0)
		{
			//出现连续通杀通赔 随机;
			std::random_shuffle(card_rand.begin(), card_rand.end());
			int index = 0;
			for (auto& i : card_rand)
			{
				auto cards = cards_vec[index].second;
				if (i == 0)
				{
					m_banker->set_cards(cards);
				}
				else
				{
					m_others[i-1]->set_cards(cards);
				}
				++index;
			}
		}
	}

	//同步下注信息，前端和服务端下注存在不一致
	sync_bet_gold();
	double room_rate = get_room_commission_rate();
	//结果
	GOLD_TYPE total_player_gold = 0;
	GOLD_TYPE total_robot_gold = 0;
	GOLD_TYPE total_bet_gold = 0;
	m_banker_killall = true;
	for (int i = 0; i < OTHERCOUNT; i++)
	{
		bool result = logic_cards::compare(m_banker->get_cards(), m_others[i]->get_cards());
		GOLD_TYPE player_gold = 0;
		GOLD_TYPE robot_gold = 0;
		GOLD_TYPE bet_gold = 0;
		//庄家赢
		if (result)
		{
			m_others[i]->lose(m_banker->get_cards()->get_cards_rate(), player_gold, robot_gold, bet_gold);
		}
		//闲家赢
		else
		{
			m_banker_killall = false;
			m_others[i]->win(player_gold, robot_gold, bet_gold);
		}
		total_player_gold += player_gold;
		total_robot_gold += robot_gold;
		total_bet_gold += bet_gold;

		if (m_banker->is_system_banker() == true)
		{
			m_room->log_game_single_info(i, player_gold, !result);
		}
	}
	if (m_banker->is_system_banker() == true)
	{
		m_room->log_game_info(total_player_gold, total_bet_gold);
	}
	else
	{
		//玩家上庄记录成就
		m_banker->get_player()->quest_change(cards_win, 1, (int)m_banker->get_cards()->get_cards_type());
		m_banker->get_player()->quest_change(game_count, 1);
		m_room->log_robot_info(-total_robot_gold);
	}
	auto banker_win_gold = total_player_gold + total_robot_gold;
	GOLD_TYPE rax = 0;
	if (banker_win_gold > 0)
	{
		rax = std::ceil(banker_win_gold * room_rate);
	}

	m_banker->win_gold(banker_win_gold, rax);

	//计算闲家在每个区域下注，然后计算税收;
	calc_player_rax();
	m_room->sync_player_gold();

	log_history_cards();

	set_game_state(logic_main::game_state::game_state_deal);

	auto sendmsg = PACKET_CREATE(packetl2c_bc_scene_deal_into, e_mst_l2c_bc_scene_deal_into);
	sendmsg->set_count_down(get_cd_time());

	//@2.0 记录每个区域的牌;
	auto banker_card = m_cows_log->add_cardsinfo();
	banker_card->set_pos(0);
	auto cards_info = sendmsg->add_cards_infos();
	get_banker()->get_cards()->fill_cards_info(cards_info, banker_card);
	for (unsigned int i = 0; i < OTHERCOUNT; i++)
	{
		auto other_card = m_cows_log->add_cardsinfo();
		other_card->set_pos(i+1);
		other_card->set_sumbet(m_others[i]->get_total_bet_gold());
		
		auto cards_info = sendmsg->add_cards_infos();
		m_others[i]->get_cards()->fill_cards_info(cards_info, other_card);
	}
	m_room->broadcast_msg_to_client(sendmsg);
	
	//@add by Hunter 2017/08/27;
	//@游戏发牌阶段清理游戏状态为空闲;
	auto& players = m_room->get_players();
	for (auto it = players.begin(); it != players.end(); it++) {
		//@同步游戏结束状态;
		//@用户下注或上庄则判定为游戏状态;
		//@add by Hunter 2017/08/26;
		//@同步用户状态;
		if (!it->second->is_robot()) {
			//@进入房间同步状态;
			auto geh = game_engine::instance().get_handler();
			if (geh)
			{
				auto pid = it->second->get_pid();
				geh->sync_userGameStatus(it->second->get_pid(),
					game_engine::instance().get_gameid(),
					m_room->get_room_id(),
					0,
					0,
					0
				);
			}
		}
		//@3.0记录每个下注用户的信息;
		auto pl = it->second;
		auto pid = pl->get_pid();
		bool isbanker = m_banker->is_banker(pl->get_pid());
		if (pl->get_self_is_bet() || isbanker)
		{
			auto pinfo = m_cows_log->add_pinfo();
			pinfo->set_pid(pid);
			pinfo->set_goldbegin(pl->get_pre_gold());
			pinfo->set_goldend(pl->get_gold());
			pinfo->set_vargold(pl->get_self_win_gold());
			//pinfo->set_luckvalue(pl->get_lucky());
			pinfo->set_isrobot(pl->is_robot());
			pinfo->set_commission(pl->get_tax());

			pinfo->set_isbanker(isbanker);
			if (!isbanker)
			{
				int index = 0;
				for (auto& other : m_others)
				{
					auto bet = other->get_bet_gold(pid);
					if (index == 0)
					{
						pinfo->mutable_bets()->set_bet1(bet);
					}
					if (index == 1)
					{
						pinfo->mutable_bets()->set_bet2(bet);
					}
					if (index == 2)
					{
						pinfo->mutable_bets()->set_bet3(bet);
					}
					if (index == 3)
					{
						pinfo->mutable_bets()->set_bet4(bet);
					}
					++index;
				}
			}
			//ploginfo->set_allocated_pinfo(pinfo);
		}
	}
	m_cows_log->set_endtime(time_helper::instance().get_cur_time());

	if (!m_player_bet.empty() || m_banker->is_system_banker() == false)
	{
		std::string strLog = m_cows_log->SerializeAsString();
		game_engine::instance().get_handler()->sendLogInfo(strLog, game_engine::instance().get_gameid());
	}
}

void logic_main::computer_result()
{
	set_game_state(logic_main::game_state::game_state_result);

	sync_bet_gold();
	update_star_gold(); 

	auto pBester = getBesterPlayerPtr();

	auto& players = m_room->get_players();
	for (auto it = players.begin(); it != players.end(); it++)
	{
		auto sendmsg = PACKET_CREATE(packetl2c_bc_scene_result_into, e_mst_l2c_bc_scene_result_into);
		sendmsg->set_count_down(get_cd_time());
		
		auto result_info = sendmsg->mutable_result_info();
		fill_result_info(it->second, result_info);

		if (pBester)
		{
			result_info->set_banker_kill_all(false);
			result_info->set_bester_gold(pBester->get_gold());
			result_info->set_bester_win_gold(pBester->get_self_win_gold());
			result_info->set_bester_region(pBester->GetUserRegion());
			result_info->set_bester_icon_custom(pBester->get_icon_custom());
		}
		else 
		{
			if (m_banker_killall == true)
			{
				//1-庄家通杀;
				result_info->set_banker_kill_all(1);
				result_info->set_bester_gold(0);
				result_info->set_bester_win_gold(0);
				result_info->set_bester_region("");
				result_info->set_bester_icon_custom("");
			}
			else
			{
				//2，闲家不同的区域下注，累计没有赢，庄家赢了;
				result_info->set_banker_kill_all(2);

				result_info->set_bester_gold(m_banker->get_banker_gold());
				result_info->set_bester_win_gold(m_banker->get_total_win_gold());
				result_info->set_bester_region(m_banker->getBankerRegion());
				result_info->set_bester_icon_custom(m_banker->get_player()->get_icon_custom());
			}
		}

		auto other_players = it->second->get_otherplayers();
		int nSize = other_players.size();
		result_info->mutable_others_win_info()->Reserve(nSize);
		for (auto it2 = other_players.begin(); it2 != other_players.end(); it2++)
		{
			auto other_win_info = result_info->add_others_win_info();
			other_win_info->set_player_id((*it2)->get_pid());
			//other_win_info->set_player_name((*it2)->get_nickname());//去掉昵称;
			other_win_info->set_player_gold((*it2)->get_gold());

			//player->get_otherplayers() 获取的列表不是当前下注的玩家的列表：所以出现客户端通杀或者通赔时显示的结算信息有问题; 
			auto pid = (*it2)->get_pid();
			if (!check_bet(pid))
			{
				//判断玩家是否下注，如果没下注，当前局 为 0;
				other_win_info->set_player_win_gold(0);
			}
			else
			{
				other_win_info->set_player_win_gold((*it2)->get_self_win_gold());
			}
			other_win_info->set_player_region((*it2)->GetUserRegion());
		}
		
		it->second->send_msg_to_client(sendmsg);
	}
}

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

		player_ptr->set_self_is_bet(self_is_bet);
		player_ptr->set_self_win_gold(total_win_gold);
		
		//闲家下注税收是每个区域的输赢总和，前面change_gold 处理单个区域没有扣税;
		player_ptr->set_tax(rax_gold);
	}
}

LPlayerPtr logic_main::getBesterPlayerPtr()
{
	auto& players = m_room->get_players();
	LPlayerPtr pRet=nullptr;
	GOLD_TYPE bester_win_gold=0;
	for (auto it = players.begin(); it != players.end(); it++)
	{
		//在 calc_player_rax() 中已经计算了,这里直接取值比较所有玩家得分最高的;
		GOLD_TYPE total_win_gold = it->second->get_self_win_gold();
		bool self_is_bet = it->second->get_self_is_bet();

		if (total_win_gold > bester_win_gold)
		{
			bester_win_gold=total_win_gold;
			pRet=it->second;
		}
	}

	return pRet;
}

void cows_space::logic_main::server_stop()
{
	auto sendmsg = PACKET_CREATE(packetl2c_kick_player, e_mst_l2c_kick_player);
	if (sendmsg /*&& m_room*/)
	{
		sendmsg->set_reason(e_msg_kick_player::e_cleanout_servicestop);
		//m_room->broadcast_msg_to_client(sendmsg);
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

void cows_space::logic_main::check_player_status(bool server_stop)
{
	if (m_room)
	{
		auto& player_map = m_room->get_players();
		for (auto& item : player_map)
		{
			auto& player = item.second;
			if (player)
			{
				//记录当前玩家未下注的钱;
				player->record_gold();
				//清空上一局胜负值;
				player->set_self_win_gold(0);
				int status = item.second->get_kick_status();
				if (status != 0)
				{
					auto sendmsg = PACKET_CREATE(packetl2c_kick_player, e_mst_l2c_kick_player);
					if (sendmsg)
					{
						sendmsg->set_reason((e_msg_kick_player)status);
						player->send_msg_to_client(sendmsg);
					}
					auto pl = player->getIGamePlayer();
					if (pl)
					{
						pl->reqPlayer_leaveGame();
					}
				}
			}
		}
	}
}

void logic_main::fill_result_info(LPlayerPtr& player, cows_protocols::msg_result_info* result_info)
{
	result_info->set_banker_win_gold(m_banker->get_last_win_gold());
	result_info->set_self_gold(player->get_gold());
	
	result_info->mutable_other_win_golds()->Reserve(4);
	
	GOLD_TYPE total_win_gold = 0;
	bool self_is_bet = false;
	for (unsigned int i = 0; i < m_others.size(); i++)
	{
		result_info->add_other_win_golds(m_others[i]->get_total_win_gold());
		//GOLD_TYPE win_gold = m_others[i]->get_win_gold(player->get_pid());
		//total_win_gold += win_gold;
		//if (!self_is_bet && win_gold != 0)
		//{
		//	self_is_bet = true;
		//}
	}
	//result_info->set_self_is_bet(self_is_bet);
	//result_info->set_self_win_gold(total_win_gold);

	result_info->set_self_is_bet(player->get_self_is_bet());
	result_info->set_self_win_gold(player->get_self_win_gold());
}

void logic_main::init_history(GIntListFieldPtr& history, time_t log_time)
{
	clear_history();
	auto curdata = time_helper::instance().get_cur_date();

	m_log_time = time_helper::convert_from_date(curdata);
	if (m_log_time == log_time && history->get_intlist()->size() >= 9)
	{
		m_total_count = history->get(0);

		m_total_history.m_total_win[0] = history->get(1);
		m_total_history.m_total_win[1] = history->get(2);
		m_total_history.m_total_win[2] = history->get(3);
		m_total_history.m_total_win[3] = history->get(4);

		m_total_history.m_total_lose[0] = history->get(5);
		m_total_history.m_total_lose[1] = history->get(6);
		m_total_history.m_total_lose[2] = history->get(7);
		m_total_history.m_total_lose[3] = history->get(8);

		for (int i = 9; i < history->get_intlist()->size();)
		{
			if (i + 3 < history->get_intlist()->size())
			{
				history_info info;
				info.m_is_win[0] = history->get(i) == 1;
				info.m_is_win[1] = history->get(i+1) == 1;
				info.m_is_win[2] = history->get(i+2) == 1;
				info.m_is_win[3] = history->get(i+3) == 1;

				m_history_infos.push_back(info);

				i = i + 4;
			}
			else
			{
				break;
			}
		}
	}
}

void logic_main::clear_history()
{
	m_total_count = 0;
	for (unsigned int i = 0; i < m_others.size(); i++)
	{
		m_total_history.m_total_win[i] = 0;
		m_total_history.m_total_lose[i] = 0;
	}
	m_history_infos.clear();
}

void logic_main::log_history_cards()
{
	//清空昨天数据
	auto curdata = time_helper::instance().get_cur_date();
	time_t log_time = time_helper::convert_from_date(curdata);
	if (m_log_time != log_time)
	{
		clear_history();
		m_log_time = log_time;
	}

	m_total_count ++;
	history_info history;
	for (unsigned int i = 0; i < m_others.size(); i++)
	{
		history.m_is_win[i] = m_others[i]->is_win();
		if (m_others[i]->is_win())
		{
			m_total_history.m_total_win[i]++;
		}
		else
		{
			m_total_history.m_total_lose[i]++;
		}
	}

	m_history_infos.push_front(history);
	if (m_history_infos.size() > 10)
	{
		m_history_infos.pop_back();
	}
}

int logic_main::get_history_total_count()
{
	return m_total_count;
}

int64_t logic_main::get_history_log_time()
{
	return m_log_time;
}

const logic_main::total_history_info& logic_main::get_total_history_info()
{
	return m_total_history;
}

std::list<logic_main::history_info>& logic_main::get_history_infos()
{
	return m_history_infos;
}

int logic_main::apply_banker(LPlayerPtr& player)
{
	//if (m_game_state != logic_main::game_state::game_state_bet)
	//{
	//	return msg_type_def::e_msg_result_def::e_rmt_error_game_state;
	//}

	if (m_banker->is_banker(player->get_pid()))
	{
		return msg_type_def::e_msg_result_def::e_rmt_now_is_banker;
	}

	auto it = m_apply_pids.find(player->get_pid());
	if (it != m_apply_pids.end())
	{
		return msg_type_def::e_msg_result_def::e_rmt_has_in_banker_list;
	}

	if (player->get_gold() < m_room->get_data()->mBankerGold)
	{
		return msg_type_def::e_msg_result_def::e_rmt_gold_not_enough;
	}

	if (m_apply_players.size() > 20)
	{
		return msg_type_def::e_msg_result_def::e_rmt_banker_is_full;
	}

	m_apply_players.push_back(player);
	m_apply_pids.insert(player->get_pid());

	//有玩家申请上庄，同时机器人在庄上，机器准备下庄
	if (!player->is_robot() && m_banker->get_player() != nullptr && m_banker->get_player()->is_robot())
	{
		m_banker->get_player()->get_robot()->pre_leave_banker();
	}

	return  msg_type_def::e_msg_result_def::e_rmt_success;
}

int logic_main::cancel_apply_banker(LPlayerPtr& player)
{
	if (m_banker->is_banker(player->get_pid()))
	{
		return msg_type_def::e_msg_result_def::e_rmt_now_is_banker;
	}

	auto it = m_apply_pids.find(player->get_pid());
	if (it != m_apply_pids.end())
	{
		m_apply_pids.erase(player->get_pid());
		for (auto it = m_apply_players.begin(); it != m_apply_players.end(); ++it)
		{
			if ((*it).get() == player.get())
			{
				m_apply_players.erase(it);
				return  msg_type_def::e_msg_result_def::e_rmt_success;
			}
		}
	}
	
	return  msg_type_def::e_msg_result_def::e_rmt_is_not_banker;
}

void logic_main::clear_apply_banker(LPlayerPtr& player)
{
	auto it = m_apply_pids.find(player->get_pid());
	if (it != m_apply_pids.end())
	{
		m_apply_pids.erase(player->get_pid());
		for (auto it = m_apply_players.begin(); it != m_apply_players.end(); ++it)
		{
			if ((*it).get() == player.get())
			{
				m_apply_players.erase(it);
				break;
			}
		}
	}
}

int logic_main::get_inc_id()
{
	return m_inc_id;
}

bool logic_main::can_snatch()
{
	//上庄保护期
	if (m_banker->is_player_banker() && m_banker->is_banker_protect())
	{
		return false;
	}
	return true;
}

int logic_main::get_snatch_gold()
{
	return m_snatch_gold;
}

uint32_t logic_main::get_snatch_player()
{
	if (m_snatch_player == nullptr)
	{
		return 0;
	}
	return m_snatch_player->get_pid();
}

int logic_main::snatch_banker(LPlayerPtr& player, int gold)
{
	//if (m_game_state != logic_main::game_state::game_state_bet)
	//{
	//	return msg_type_def::e_msg_result_def::e_rmt_error_game_state;
	//}

	if (m_banker->is_player_banker() && m_banker->is_banker_protect())
	{
		//上庄保护期
		return msg_type_def::e_msg_result_def::e_rmt_banker_protect;
	}
	if (m_banker->is_banker(player->get_pid()))
	{
		return msg_type_def::e_msg_result_def::e_rmt_now_is_banker;
	}
	if (gold <= 0)
	{
		return msg_type_def::e_msg_result_def::e_rmt_gold_not_enough;
	}
	if (player->get_gold() < gold)
	{
		return msg_type_def::e_msg_result_def::e_rmt_gold_not_enough;
	}
	if (m_snatch_player == player)
	{
		return msg_type_def::e_msg_result_def::e_rmt_snatch_is_you;
	}
	else
	{
		if (gold > m_snatch_gold)
		{
			add_snatch_player(player, gold);
			return msg_type_def::e_msg_result_def::e_rmt_success;
		}
		else
		{
			add_snatch_player(player, gold);
			return msg_type_def::e_msg_result_def::e_rmt_snatch_is_low;
		}
	}
}

void logic_main::add_snatch_player(LPlayerPtr& player, int gold)
{
	if (player == nullptr)
	{
		auto sendmsg = PACKET_CREATE(packetl2c_bc_snatch_banker, e_mst_l2c_bc_snatch_banker);
		sendmsg->set_snatch_player_id(0);
		sendmsg->set_snatch_gold(0);
		m_room->broadcast_msg_to_client(sendmsg);
	}
	else
	{
		if (gold > m_snatch_gold)
		{
			m_snatch_player = player;
			m_snatch_gold = gold;

			auto sendmsg = PACKET_CREATE(packetl2c_bc_snatch_banker, e_mst_l2c_bc_snatch_banker);
			sendmsg->set_snatch_player_id(player->get_pid());
			sendmsg->set_snatch_gold(m_snatch_gold);
			m_room->broadcast_msg_to_client(sendmsg);
		}
		m_snatch_player_ids[player->get_pid()] = gold;
	}
}

void logic_main::remove_snatch_player(LPlayerPtr& player)
{
	m_snatch_player_ids.erase(player->get_pid());

	if (m_snatch_player == player)
	{
		m_snatch_player = nullptr;
		m_snatch_gold = 0;

		int32_t snatch_gold = 0;
		uint32_t snatch_player_id = 0;
		for (auto it = m_snatch_player_ids.begin(); it != m_snatch_player_ids.end(); it++)
		{
			if (it->second > snatch_gold)
			{
				snatch_gold = it->second;
				snatch_player_id = it->first;
			}
		}

		LPlayerPtr new_player = m_room->get_player(snatch_player_id);
		add_snatch_player(new_player, snatch_gold);
	}
}

void logic_main::clear_snatch_player()
{
	m_snatch_player = nullptr;
	m_snatch_gold = 0;
	m_snatch_player_ids.clear();
}

bool logic_main::check_banker()
{
	m_banker->check_player_state();
	//判断上庄的人条件
	if (m_banker->is_player_banker())
	{
		if (m_banker->is_apply_leave())
		{
			m_banker->set_player_banker(logic_player::EmptyPtr);
			return false;
		}
		else if (m_banker->get_banker_gold() < m_room->get_data()->mForceLeaveGold)
		{
			m_banker->set_player_banker(logic_player::EmptyPtr);
			return false;
		}
		return true;
	}
	else
	{
		if (m_banker->get_banker_gold() < m_room->get_data()->mBankerGold)
		{
			m_banker->resetSystemBanker();
			return false;
		}
	}
	//有人抢庄
	if ((m_banker->is_player_banker() && m_banker->is_banker_protect() == false) || !m_banker->is_player_banker())
	{
		if (m_snatch_player != nullptr && m_snatch_player->change_gold2(-m_snatch_gold, 33))
		{
			auto sendmsg = PACKET_CREATE(packetl2c_banker_success, e_mst_l2c_banker_success);
			sendmsg->set_gold(m_snatch_gold);
			m_snatch_player->send_msg_to_client(sendmsg);

			m_banker->set_player_banker(m_snatch_player);

			clear_apply_banker(m_snatch_player);
			clear_snatch_player();
			return true;
		}
	}
	if (!m_banker->is_player_banker())
	{
		if (m_apply_players.size() > 0)
		{
			LPlayerPtr player = m_apply_players.front();
			m_apply_players.pop_front();

			m_apply_pids.erase(player->get_pid());

			m_banker->set_player_banker(player);
			return true;
		}
	}
	return false;
}

int logic_main::ask_leave_banker(LPlayerPtr& player, bool force, int32_t& cost_ticket)
{
	//判断是否为庄家
	if (!m_banker->is_banker(player->get_pid()))
	{
		return msg_type_def::e_msg_result_def::e_rmt_is_not_banker;
	}

	if (m_banker->is_apply_leave())
	{
		return msg_type_def::e_msg_result_def::e_rmt_haven_apply_leave;
	}

	if (m_banker->get_banker_count() >= m_banker->get_min_banker_count())
	{
		m_banker->apply_leave();
		return msg_type_def::e_msg_result_def::e_rmt_success;
	}
	if (force)
	{
		cost_ticket = m_banker->force_leave();
		if (cost_ticket >= 0)
		{
			return msg_type_def::e_msg_result_def::e_rmt_success;
		}
		else
		{
			return msg_type_def::e_msg_result_def::e_rmt_ticket_not_enough;
		}
	}
	return msg_type_def::e_msg_result_def::e_rmt_banker_not_enough;
}

std::list<LPlayerPtr>& logic_main::get_apply_bankers()
{
	return m_apply_players;
}

int cows_space::logic_main::get_robot_banker_size()
{
	int size = 0;
	for (auto& item : m_apply_players)
	{
		if (item->is_robot())
		{
			++size;
		}
	}
	return size;
}

bool logic_main::can_leave_room(uint32_t player_id)
{
	if (m_banker->is_banker(player_id))
	{
		return false;
	}
	return true;
}

void logic_main::leave_room(LPlayerPtr& player)
{
	//清空下注
	clear_bet_gold(player);

	//清空上庄申请
	clear_apply_banker(player);


	remove_snatch_player(player);

	//如果
	if (m_banker->is_banker(player->get_pid()))
	{
		m_banker->set_player_banker(logic_player::EmptyPtr);
	}
}

logic_room* logic_main::get_room()
{
	return m_room;
}

logic_banker* logic_main::get_banker()
{
	return m_banker;
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

void cows_space::logic_main::kill_points(int32_t cutRound, bool status)
{
	m_kill_points_switch = status;
	if (m_kill_points_switch)
	{
		SLOG_CRITICAL << "###### Manager Start kill points, cut round: "<< cutRound<<", status: "<<status;
	}
	else
	{
		SLOG_CRITICAL << "###### Manager Stop kill points, cut round: " << cutRound << ", status: " << status;
	}
}

void cows_space::logic_main::service_ctrl(int32_t optype)
{
	m_service_status = optype;
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
			static float pre_time = Cows_BaseInfo::GetSingleton()->GetData("PreTime")->mValue;
			m_duration = pre_time;
		}
		break;
	case game_state::game_state_bet:
		{
			static float bet_time = Cows_BaseInfo::GetSingleton()->GetData("BetTime")->mValue;
			m_duration = bet_time;
			m_sync_eleased = 0.0f;
			m_sync_interval = 1.0f;
		}
		break;
	case game_state::game_state_deal:
		{
			static float deal_time = Cows_BaseInfo::GetSingleton()->GetData("DealTime")->mValue;
			m_duration = deal_time;
		}
		break;
	case game_state::game_state_result:
		{
			static float result_time = Cows_BaseInfo::GetSingleton()->GetData("ResultTime")->mValue;
			m_duration = result_time;
		}
		break;
	default:
		break;
	}
}

bool cows_space::logic_main::check_bet(int32_t pid)
{
	bool is_bet = false;
	for (auto& other : m_others)
	{
		if (other)
		{
			other->get_win_gold(pid, is_bet);
			if (is_bet)
			{
				return is_bet;
			}
		}
	}
	return is_bet;
}
