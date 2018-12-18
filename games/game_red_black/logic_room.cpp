#include "stdafx.h"
#include "logic_room.h"
#include "RedBlack_RoomCFG.h"
#include "game_db.h"
#include "logic_player.h"
#include "logic_main.h"

#include "game_engine.h"
#include "i_game_ehandler.h"
#include "enable_random.h"
#include "RedBlack_RobotCFG.h"
#include "logic_player.h"
#include "logic_robot.h"
#include "time_helper.h"
#include "game_engine.h"

#include "proc_red_black_logic.h"

#include "RedBlack_RoomStockCFG.h"
//@add by Big O 2017/01/08;
//@在线统计增加;
#include "game_db_log.h"

DRAGON_RED_BLACK_USING

logic_room::logic_room(const RedBlack_RoomCFGData* cfg, logic_lobby* _lobby, uint16_t room_id, uint16_t child_id)
{
	init_game_object();
	m_lobby = _lobby;
	m_cfg = cfg;
	m_room_id = room_id;
	m_child_id = child_id;
	RoomID->set_value(cfg->mRoomID); 


	m_room_cfg = RedBlack_RoomStockCFG::GetSingleton()->GetData(m_cfg->mRoomID);

	if(!load_room())
		create_room();

	m_main = new logic_main(this);

	m_checksave = 0.0;
	m_today_win_gold = 0;
	m_today_lose_gold = 0;
	m_today_bet_gold = 0;
	m_avg_bet_gold = 0;

	//机器人
	m_robot_elapsed = global_random::instance().rand_double(5, 15);

	m_player_count = 0;

	m_list_balance_win_lose_info.clear();
	m_sync_palyer_times = 0;
}


logic_room::~logic_room(void)
{
	SAFE_DELETE(m_main);
}

uint32_t logic_room::get_id()
{
	return m_cfg->mRoomID;
}

void logic_room::heartbeat( double elapsed )
{
	//if(m_players.size() == 0) 
	//	return;

	m_main->heartbeat(elapsed);
	robot_heartbeat(elapsed);
	m_checksave += elapsed;
	if (m_checksave > 30)
	{
		store_game_object();
		m_checksave=0.0;
	}
	if ((m_sync_palyer_times -= elapsed) <= 0)
	{
		m_sync_palyer_times = 120;
		game_engine::instance().sync_room_players(get_id(), m_players.size());
	}

}

void logic_room::robot_heartbeat(double elapsed)
{
	if (!m_room_cfg)  return;
	static int isOpen = m_room_cfg->mIsOpen;
	if (isOpen == 0)
		return;

	static int minCount = m_room_cfg->mRobotMinCount;
	static int maxCount = m_room_cfg->mRobotMaxCount;
	static int robotCount = global_random::instance().rand_int(minCount, maxCount);

	//机器人AI
	for (auto it = m_robot_players.begin(); it != m_robot_players.end(); ++it)
	{
		it->second->get_robot()->heartbeat(elapsed);
	}

	//机器人进入管理
	m_robot_elapsed -= elapsed;
	if (m_robot_elapsed <= 0)
	{
		for (auto it = m_robot_players.begin(); it != m_robot_players.end(); ++it)
		{
			if (it->second->get_robot()->need_exit())
			{
				if (it->second->can_leave_room())
				{
					game_engine::instance().release_robot(it->first);
				}
			}
		}

		if (m_robot_players.size() < robotCount)
		{
			request_robot();
		}

		m_robot_elapsed = global_random::instance().rand_double(5, 15);
	}
}

void logic_room::request_robot()
{
	//static int minVIP = RedBlack_RobotCFG::GetSingleton()->GetData("RobotMinVip")->mValue;
	//static int maxVIP = RedBlack_RobotCFG::GetSingleton()->GetData("RobotMaxVip")->mValue;
	//return;

	if (!m_room_cfg || m_server_stauts == 100) return;

	static int minVIP = m_room_cfg->mRobotMinVip;
	static int maxVIP = m_room_cfg->mRobotMaxVip;

	int32_t vip_level = global_random::instance().rand_int(minVIP, maxVIP);	

	auto str = boost::format( "RobotGold%1%")%vip_level;
	std::string name = str.str().c_str();
	GOLD_TYPE base_gold = RedBlack_RobotCFG::GetSingleton()->GetData(name)->mValue;

	game_engine::instance().request_robot(RoomID->get_value(), global_random::instance().rand_int(base_gold*0.8, base_gold*1.25), vip_level);
}

uint16_t logic_room::get_cur_cout()
{
	return m_players.size();
}

LPlayerPtr& logic_room::get_player(uint32_t pid)
{
	auto it = m_players.find(pid);
	if (it != m_players.end())
	{
		return it->second;
	}
	return logic_player::EmptyPtr;
}

LPLAYER_MAP& logic_room::get_players()
{
	return m_players;
}

void logic_room::update_playerlist()
{
	std::vector<std::pair<uint32_t, LPlayerPtr>> player_list = get_player_list();

	std::pair<uint32_t, LPlayerPtr> rich = get_rich_player();//取大富豪;
	if (rich.second == nullptr)
	{
		return;
	}
	std::pair<uint32_t, LPlayerPtr> best_win = get_best_player(rich.first);//取神算子;
	if (best_win.second == nullptr)
	{
		return;
	}
	//清除离开的玩家;
	auto it = m_player_list.begin();
	while (it != m_player_list.end())
	{
		auto pid = it->first;
		if (m_players.find(pid) == m_players.end() || pid == rich.first || pid == best_win.first || it->second == nullptr)
		{
			it = m_player_list.erase(it);
			continue;
		}
		else
		{
			++it;
		}
	}

	int rd = global_random::instance().rand_int(1, 2);
	for (int i = 0; i < rd; ++i)
	{
		if (!m_player_list.empty())
		{
			m_player_list.pop_back();
		}
	}
	//记录已经存在的;
	std::set<uint32_t> pids;
	pids.insert(best_win.first);
	pids.insert(rich.first);
	for (auto& it : m_player_list)
	{
		pids.insert(it.first);
	}
	//添加新的;
	std::random_shuffle(player_list.begin(), player_list.end());
	for (auto& it : player_list)
	{
		if (m_player_list.size() < 4)
		{
			if (it.second->get_gold() > 50 * 100 && pids.find(it.first) == pids.end())
			{
				m_player_list.push_back(it);
			}
		}
	}
	m_player_list.push_front(best_win);
	m_player_list.push_front(rich);
}

std::list<std::pair<uint32_t, LPlayerPtr>>& logic_room::get_show_players(uint32_t pid)
{
	return m_player_list;
}

bool logic_room::has_seat(uint16_t& tableid)
{
	return true;
}

int logic_room::enter_room(LPlayerPtr player)
{
	//已经停服务;
	if (m_server_stauts == 100)
	{
		return 84;
	}

	auto it = m_players.find(player->get_pid());
	if (it != m_players.end())
	{
		SLOG_ERROR << "logic_room::enter_room exists player id:"<<player->get_pid();
		return 2;
	}
	if (player->get_room() != nullptr)
	{
		SLOG_CRITICAL << "logic_room::enter_room exists room id:"<<player->get_pid();
		return 2;
	}
	m_players.insert(std::make_pair(player->get_pid(), player));
	m_pids.push_back(player->get_pid());
	player->enter_room(this);

	if (player->is_robot())
	{
		player->create_robot();
		m_robot_players.insert(std::make_pair(player->get_pid(), player));
	}

	if (m_main->get_game_state() == logic_main::game_state::game_state_unknown)
	{
		m_main->pre_start_game();
	}

	SLOG_CRITICAL << "+++++ enter_table, playerid: " << player->get_pid() << ", player counter: " << m_players.size() << ", pid counter: " << m_pids.size();
	//@进入房间调用;
	if (!player->is_robot()) {
		//@进入房间同步状态;
		auto geh = game_engine::instance().get_handler();
		if (geh)
		{
			geh->sync_userGameStatus(player->get_pid(),
				game_engine::instance().get_gameid(),
				get_room_id(),
				0,
				0,
				0
			);
		}

		db_log::instance().joingame(player->get_pid(), get_room_id(), m_child_id);
	}
	return 1;
}

//@返回当前房间号;
uint16_t inline logic_room::get_room_id()
{
	return m_room_id;
}

int logic_room::get_child_id()
{
	return m_child_id;
}

void logic_room::leave_room(uint32_t pid)
{
	auto it = m_players.find(pid);
	if (it != m_players.end())
	{
		if (m_main != nullptr)
		{
			m_main->leave_room(it->second);
		}

		//@离开房间调用;
		auto playerid = pid;
		if (!it->second->is_robot()) {

			//@退出房间;
			auto geh = game_engine::instance().get_handler();
			if (geh)
			{
				geh->sync_userGameStatus(pid,
					game_engine::instance().get_gameid(),
					get_room_id(),
					0,
					0,
					0
				);
			}

			db_log::instance().leavegame(playerid);
		}
		m_players.erase(it);
	}
	for (unsigned int i = 0; i < m_pids.size(); i++)
	{
		if (m_pids[i] == pid)
		{
			m_pids[i] = m_pids[m_pids.size() - 1];
			m_pids.pop_back();
			break;
		}
	}
	
	m_robot_players.erase(pid);

	SLOG_CRITICAL << "----- leave_table, playerid: " << pid << ", player counter: " << m_players.size() << ", pid counter: " << m_pids.size();
// 	//所有玩家都离开了
// 	if (m_players.size() == 0)
// 	{
// 		if (m_main != nullptr)
// 		{
// 			m_main->stop_game();
// 		}
// 	}

}

void logic_room::init_robot_bet()
{
	for (auto& it : m_robot_players)
	{
		auto rate = global_random::instance().rand_int(m_room_cfg->mRobotRobotMinBet, m_room_cfg->mRobotRobotMaxBet) / 100.0;
		it.second->get_robot()->set_max_bet_gold(rate);
		it.second->set_robot_bet(0);
	}
}

double logic_room::get_rate()
{
	double rate_val = 0.0;
	i_game_ehandler* ehander = game_engine::instance().get_handler();
	if (ehander)
	{
		int roomid = this->get_id();
		int rate = ehander->GetRoomCommissionRate(roomid);
		rate_val = (double)rate / 100;
	}
	return rate_val;
}

const RedBlack_RoomCFGData* logic_room::get_data() const
{
	return m_cfg;
}

void logic_room::release()
{
	store_game_object();
}

int logic_room::broadcast_msg_to_client(std::vector<uint32_t>& pids, uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg)
{
	return game_engine::instance().get_handler()->broadcast_msg_to_client(pids, packet_id, msg);
}

std::vector<std::pair<uint32_t, LPlayerPtr>> logic_room::get_player_list()
{
	std::vector<std::pair<uint32_t, LPlayerPtr>> player_list;
	for (auto& it : m_players)
	{
		if (it.second != nullptr)
		{
			player_list.push_back(it);
		}
	}
	return std::move(player_list);
}

std::pair<uint32_t, LPlayerPtr> logic_room::get_rich_player()
{
	std::pair<uint32_t, LPlayerPtr> rich_player;
	for (auto& it : m_players)
	{
		if (rich_player.second == nullptr || rich_player.second->get_gold() < it.second->get_gold())
		{
			rich_player = it;
		}
	}
	return std::move(rich_player);
}

std::pair<uint32_t, LPlayerPtr> logic_room::get_best_player(uint32_t pid)
{
	std::pair<uint32_t, LPlayerPtr> rich_player;
	for (auto& it : m_players)
	{
		if (rich_player.second == nullptr || rich_player.second->get_winner_count() < it.second->get_winner_count())
		{
			if (it.second->get_pid() != pid)
			{
				rich_player = it;
			}
		}
	}
	return std::move(rich_player);
}

void logic_room::sync_player_gold()
{
	for (auto it = m_players.begin(); it != m_players.end(); ++it)
	{
		it->second->sync_change_gold();

		if (it->second->get_balance_bet())
		{
			//参与赌博记录下近20局的数据
			it->second->store_balance_record();
		}
	}
}

void logic_room::clear_player_balane_info()
{
	for (auto it = m_players.begin(); it != m_players.end(); ++it)
	{
		it->second->clear_balace_info();
	}
}

int64_t logic_room::get_today_win_gold()
{
	return m_today_win_gold;
}

int64_t logic_room::get_today_lose_gold()
{
	return m_today_lose_gold;
}

int64_t logic_room::get_today_bet_gold()
{
	return m_today_bet_gold;
}

void logic_room::clear_today_gold()
{
	m_today_win_gold = 0;
	m_today_lose_gold = 0;
	m_today_bet_gold = 0;
}

//////////////////////////////////////////////////////////////////////////
void logic_room::create_room()
{

	TotalWinGold->set_value(0);
	TotalLoseGold->set_value(0);
	TotalBetGold->set_value(0);
	TotalRobotWinGold->set_value(0);
	TotalRobotLoseGold->set_value(0);

	WinGold1->set_value(0);
	LoseGold1->set_value(0);
	WinCount1->set_value(0);
	WinGold2->set_value(0);
	LoseGold2->set_value(0);
	WinCount2->set_value(0);
	WinGold3->set_value(0);
	LoseGold3->set_value(0);
	WinCount3->set_value(0);

	//qiuzq新增库存方面
	EnterCount->set_value(0);
	//总税收
	TotalTaxWinner->set_value(0);
	TotalTaxLost->set_value(0);
	//赢钱抽水
	WinnerEarningsRate->set_value(m_room_cfg->mDeduct_1);//默认值
	//输钱抽水
	LostEarningsRate->set_value(m_room_cfg->mDeduct_2);//默认值
	//初始化库存
	set_total_stock_value(m_room_cfg->mDefaultStock);

	store_game_object(true);
}

bool logic_room::load_room()
{
	mongo::BSONObj b = db_game::instance().findone(DB_RED_BLACK_ROOM, BSON("room_id"<<RoomID->get_value()));
	if(b.isEmpty())
		return false;	

	return from_bson(b);
}

void logic_room::init_game_object()
{
	RoomID = CONVERT_POINT(Tfield<int16_t>, regedit_tfield(e_got_int16, "room_id"));

	TotalWinGold = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "room_income"));
	TotalLoseGold = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "room_outcome"));
	TotalBetGold = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "TotalBetGold"));

	TotalRobotWinGold = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "TotalRobotWinGold"));
	TotalRobotLoseGold = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "TotalRobotLoseGold"));

	WinGold1 = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "WinGold1"));
	LoseGold1 = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "LoseGold1"));
	WinCount1 = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "WinCount1"));

	WinGold2 = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "WinGold2"));
	LoseGold2 = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "LoseGold2"));
	WinCount2 = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "WinCount2"));

	WinGold3 = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "WinGold3"));
	LoseGold3 = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "LoseGold3"));
	WinCount3 = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "WinCount3"));

	//库存相关增加qiuzq
	WinnerEarningsRate = CONVERT_POINT(Tfield<double>, regedit_tfield(e_got_double, "WinnerEarningsRate"));
	LostEarningsRate = CONVERT_POINT(Tfield<double>, regedit_tfield(e_got_double, "LostEarningsRate"));

	EnterCount = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "EnterCount"));
	PlayerCount = CONVERT_POINT(Tfield<int16_t>, regedit_tfield(e_got_int16, "player_count"));

	TotalStock = CONVERT_POINT(Tfield<double>, regedit_tfield(e_got_double, "TotalStock"));
	TotalTaxWinner = CONVERT_POINT(Tfield<double>, regedit_tfield(e_got_double, "TotalTaxWinner"));
	TotalTaxLost = CONVERT_POINT(Tfield<double>, regedit_tfield(e_got_double, "TotalTaxLost"));
}

bool logic_room::store_game_object(bool to_all)
{
	if(!has_update())
		return true;

	auto err = db_game::instance().update(DB_RED_BLACK_ROOM, BSON("room_id"<<RoomID->get_value()), BSON("$set"<<to_bson(to_all)));
	if(!err.empty())
	{
		SLOG_ERROR << "logic_room::store_game_object :" <<err;
		return false;
	}

	return true;
}

int logic_room::get_current_warter()
{
	if (!m_room_cfg || m_room_cfg->mStock.size() == 0 || m_room_cfg->mChangeBuff.size() == 0 )	
	{
		SLOG_ERROR<<"水位库存表配置错误";
		return 1;
	}
	int64_t total = get_total_stock_value();

	//小于最小库存
	if (total <= m_room_cfg->mStock[0])
	{
		return m_room_cfg->mChangeBuff[0];
	}

	//大于最大库存
	if (total >= m_room_cfg->mStock[m_room_cfg->mStock.size() - 1])
	{
		return m_room_cfg->mChangeBuff[m_room_cfg->mChangeBuff.size()-1];
	}

	int record_temp = 0;
	for (int i = 0; i < m_room_cfg->mStock.size(); i++)
	{
		record_temp = i;

		if (total > m_room_cfg->mStock[i])
		{
			continue;
		}
		break;
	}

	if (record_temp - 1 > m_room_cfg->mChangeBuff.size() || record_temp - 1  <  0  )
	{
		SLOG_ERROR<<"水位库存表配置错误临时返回水位1";
		return 1;
	}

	return m_room_cfg->mChangeBuff[record_temp - 1];

}

void  logic_room::record_room_win_lose_info(cards_trend& cardstrend)
{
	m_list_balance_win_lose_info.push_front(std::make_pair(cardstrend.m_card_type, cardstrend.m_win_area));
	
	if (m_list_balance_win_lose_info.size() > 300)
	{
		m_list_balance_win_lose_info.pop_back();
	}
}

int  logic_room::get_red_win_counts()
{
	int temp_record = 0;
	int dragon_win_count = 0;
	for (auto& it : m_list_balance_win_lose_info)
	{
		if (temp_record > 20) break;//只统计20局的
		if ((it.first) == 2) dragon_win_count++;
		temp_record++;
	}

	return dragon_win_count;
}

int  logic_room::get_black_win_counts()
{
	int temp_record = 0;
	int tiger_win_count = 0;
	for (auto& it : m_list_balance_win_lose_info)
	{
		if (temp_record > 20) break;//只统计20局的
		if ((it.first) == 3) tiger_win_count++;
		temp_record++;
	}

	return tiger_win_count;
}

std::list<std::pair<int, int> > &  logic_room::get_history_pokes_info()
{
	return m_list_balance_win_lose_info;
}

void logic_room::kill_points(int32_t cutRound, bool status)
{
	m_kill_points_switch = status;
	if (m_kill_points_switch == true)
	{
		m_cut_round = cutRound;
	}
	else
	{
	}
	m_main->kill_points(cutRound, status);
}

void logic_room::kick_player(uint32_t playerid, int bforce)
{
	auto itr = m_players.find(playerid);
	if (itr != m_players.end())
	{
		auto& player = itr->second;
		if (player)
		{
			if (bforce == 1)
			{
				player->set_kick_status(7);//e_cleanout_gm_kick
			}
			else if (bforce == 2)
			{
				auto sendmsg = PACKET_CREATE(red_black_protocols::packetl2c_clean_out, red_black_protocols::e_mst_l2c_clean_out);
				if (sendmsg)
				{
					sendmsg->set_reason((e_msg_cleanout_def)7);
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

void logic_room::service_ctrl(int32_t optype)
{
	m_server_stauts = optype;
	m_main->service_ctrl(optype);
}
