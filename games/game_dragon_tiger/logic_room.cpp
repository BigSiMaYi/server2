#include "stdafx.h"
#include "logic_room.h"
#include "DragonTiger_RoomCFG.h"
#include "game_db.h"
#include "logic_player.h"
#include "logic_main.h"

#include "game_engine.h"
#include "i_game_ehandler.h"
#include "enable_random.h"
#include "DragonTiger_RobotCFG.h"
#include "logic_player.h"
#include "logic_robot.h"
#include "time_helper.h"
#include "game_engine.h"
#include "dragon_tiger_def.pb.h"
#include <net/packet_manager.h>
#include "DragonTiger_RoomStockCFG.h"
#include "game_db_log.h"
#include "logic2world_msg_type.pb.h"


DRAGON_TIGER_SPACE_USING

logic_room::logic_room(const DragonTiger_RoomCFGData* cfg, logic_lobby* _lobby, int child_id)
{
	init_game_object();
	m_lobby = _lobby;
	m_cfg = cfg;
	m_child_id = child_id;
	RoomID->set_value(cfg->mRoomID); 


	m_slmdata = DragonTiger_RoomStockCFG::GetSingleton()->GetData(m_cfg->mRoomID);

	if(!load_room())
		create_room();

	m_main = new logic_main(this);
	//m_main->init_history(History, HistoryLogTime->get_value());

	m_checksave = 0.0;
	m_today_win_gold = 0;
	m_today_lose_gold = 0;
	m_today_bet_gold = 0;
	m_avg_bet_gold = 0;

	//机器人
	m_robot_elapsed = global_random::instance().rand_double(5, 15);

	m_player_count = 0;

	m_list_balance_win_lose_info.clear();
	m_history_infos.clear();

	m_equal_panel_gold = 0;

	m_sync_palyer_times = 0;
}


logic_room::~logic_room(void)
{
	SAFE_DELETE(m_main);
}
int logic_room::get_child_id()
{
	return m_child_id;
}
const DragonTiger_RoomCFGData* logic_room::get_roomcfg()
{
	return m_cfg;
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
		//reflush_history();
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
	if (!m_slmdata)  return;
	static int isOpen = m_slmdata->mIsOpen;
	if (isOpen == 0)
		return;

	//static int minCount = DragonTiger_RobotCFG::GetSingleton()->GetData("RobotMinCount")->mValue;
	//static int maxCount = DragonTiger_RobotCFG::GetSingleton()->GetData("RobotMaxCount")->mValue;
	static int minCount = m_slmdata->mRobotMinCount;
	static int maxCount = m_slmdata->mRobotMaxCount;
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
			if (it->second->get_robot()->need_exit() && it->second->can_leave_room())
			{
				game_engine::instance().release_robot(it->first);
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
	//static int minVIP = DragonTiger_RobotCFG::GetSingleton()->GetData("RobotMinVip")->mValue;
	//static int maxVIP = DragonTiger_RobotCFG::GetSingleton()->GetData("RobotMaxVip")->mValue;
	//return;

	if (!m_slmdata) return;

	static int minVIP = m_slmdata->mRobotMinVip;
	static int maxVIP = m_slmdata->mRobotMaxVip;

	int32_t vip_level = global_random::instance().rand_int(minVIP, maxVIP);	

	auto str = boost::format( "RobotGold%1%")%vip_level;
	GOLD_TYPE base_gold = DragonTiger_RobotCFG::GetSingleton()->GetData(str.str().c_str())->mValue;;

	game_engine::instance().request_robot(RoomID->get_value(), global_random::instance().rand_int(base_gold*0.8, base_gold*1.25), vip_level);
}

void logic_room::init_robot_bet()
{

	//没有机器人
	if (m_robot_players.size() == 0 || !m_slmdata)
	{
		return;
	}

	int32_t min_bet_gold = m_slmdata->mRobotSystemMinBet;
	int32_t max_bet_gold = m_slmdata->mRobotSystemMaxBet;

	for (auto it = m_robot_players.begin(); it != m_robot_players.end(); ++it)
	{
// 		if (!it->second->get_robot_bet())
// 		{
// 			SLOG_CRITICAL << "can not bet:" << it->second->get_pid() << "  money:"<< it->second->get_gold();
// 		}

		it->second->set_robot_bet(0);

		int bet_rate = global_random::instance().rand_int(min_bet_gold, max_bet_gold);
		GOLD_TYPE max_gold = it->second->get_gold() * (bet_rate / 100.0f);
		it->second->get_robot()->set_max_bet_gold(max_gold);
	}

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
		SLOG_ERROR << "logic_room::enter_room exists player id:"<<player->get_id();
		return 2;
	}
	if (player->get_room() != nullptr)
	{
		SLOG_CRITICAL << "logic_room::enter_room exists room id:"<<player->get_id();
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
	if(m_cfg){
		return m_cfg->mRoomID;
	}
	else
	{
		return 0;
	}
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
	
// 	//所有玩家都离开了
// 	if (m_players.size() == 0)
// 	{
// 		if (m_main != nullptr)
// 		{
// 			m_main->stop_game();
// 		}
// 	}

}

const DragonTiger_RoomCFGData* logic_room::get_data() const
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

void dragon_tiger_space::logic_room::calc_player_tax( float tax_rate )
{
	for (auto it = m_players.begin(); it != m_players.end(); ++it)
	{
		if (it->second->get_balance_bet())
		{
			if (it->second->get_win_lose_final_gold() > 0)
			{
				GOLD_TYPE win_gold = it->second->get_win_lose_final_gold() - it->second->get_win_lose_final_gold() * tax_rate;
				it->second->change_gold(win_gold, false);
			}
		}
	}
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

void logic_room::record_cards_trend(cards_trend cardstrend)
{
	m_history_infos.push_front(cardstrend);
	if (m_history_infos.size() > 300)
	{
		m_history_infos.pop_back();
	}
}

void logic_room::log_game_info(GOLD_TYPE total_win_gold, GOLD_TYPE bet_gold)
{
	if (total_win_gold > 0)
	{
		TotalWinGold->add_value(total_win_gold);
		m_today_win_gold += total_win_gold;
	}
	else
	{
		TotalLoseGold->add_value(-total_win_gold);
		m_today_lose_gold += -total_win_gold;
	}

	TotalBetGold->add_value(bet_gold);
	m_today_bet_gold += bet_gold;
}

void logic_room::log_robot_info(GOLD_TYPE robot_gold)
{
	if (robot_gold > 0)
	{
		TotalRobotWinGold->add_value(robot_gold);
	}
	else
	{
		TotalRobotLoseGold->add_value(-robot_gold);
	}
}

void logic_room::log_game_single_info(int i, GOLD_TYPE win_gold, bool win)
{
	if (i == 0)
	{
		if (win_gold>0)
		{
			WinGold1->add_value(win_gold);
		}
		else
		{
			LoseGold1->add_value(-win_gold);
		}
		WinCount1->add_value(win?1:0);
	}
	else if (i == 1)
	{
		if (win_gold>0)
		{
			WinGold2->add_value(win_gold);
		}
		else
		{
			LoseGold2->add_value(-win_gold);
		}
		WinCount2->add_value(win?1:0);
	}
	else if (i == 2)
	{
		if (win_gold>0)
		{
			WinGold3->add_value(win_gold);
		}
		else
		{
			LoseGold3->add_value(-win_gold);
		}
		WinCount3->add_value(win?1:0);
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

void logic_room::print_bet_win()
{
	printf("total win = %I64d, lose = %I64d, earnings= %I64d, pump=%f\n", TotalWinGold->get_value(), TotalLoseGold->get_value(), TotalWinGold->get_value() - TotalLoseGold->get_value(), (float)(TotalWinGold->get_value() - TotalLoseGold->get_value())/TotalWinGold->get_value());
	printf("1     win = %I64d, lose = %I64d, earnings= %I64d, pump=%f\n", WinGold1->get_value(), LoseGold1->get_value(), WinGold1->get_value() - LoseGold1->get_value(), (float)(WinGold1->get_value() - LoseGold1->get_value())/WinGold1->get_value());
	printf("2     win = %I64d, lose = %I64d, earnings= %I64d, pump=%f\n", WinGold2->get_value(), LoseGold2->get_value(), WinGold2->get_value() - LoseGold2->get_value(), (float)(WinGold2->get_value() - LoseGold2->get_value())/WinGold2->get_value());
	printf("3     win = %I64d, lose = %I64d, earnings= %I64d, pump=%f\n", WinGold3->get_value(), LoseGold3->get_value(), WinGold3->get_value() - LoseGold3->get_value(), (float)(WinGold3->get_value() - LoseGold3->get_value())/WinGold3->get_value());
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
	WinnerEarningsRate->set_value(m_slmdata->mDeduct_1);//默认值
	//输钱抽水
	LostEarningsRate->set_value(m_slmdata->mDeduct_2);//默认值

	store_game_object(true);
}

bool logic_room::load_room()
{
	mongo::BSONObj b = db_game::instance().findone(DB_DRAGON_TIGER_ROOM, BSON("room_id"<<RoomID->get_value()));
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

	auto err = db_game::instance().update(DB_DRAGON_TIGER_ROOM, BSON("room_id"<<RoomID->get_value()), BSON("$set"<<to_bson(to_all)));
	if(!err.empty())
	{
		SLOG_ERROR << "logic_room::store_game_object :" <<err;
		return false;
	}

	return true;
}

float logic_room::get_winner_tax(int room_id)
{
	auto pcfgdata = DragonTiger_RoomStockCFG::GetSingleton()->GetData(room_id);
	if (pcfgdata!=nullptr)
		return pcfgdata->mDeduct_1;
	return 1.0f;
}


void logic_room::reflush_roomstock()
{
	static auto ff = BSON("changestock"<<1);
	mongo::BSONObj b = db_game::instance().findone(DB_DRAGON_TIGER_ROOMCFG, BSON("room_id"<<RoomID->get_value()), &ff);
	if(!b.isEmpty() && b.hasField("changestock"))
	{
		double changestock = b.getField("changestock").Double();
		double oldstock = TotalStock->get_value();
		double newstock = changestock+oldstock;
		if ((changestock<0)&&(newstock<0))
		{
			changestock = 0-oldstock;
			newstock = 0;
		}
		TotalStock->set_value(newstock);

		mongo::BSONObjBuilder builder;
		builder.appendTimeT("time", time_helper::instance().get_cur_time());
		builder.append("gameid",	 game_engine::instance().get_gameid());
		builder.append("roomid",	  RoomID->get_value());
		builder.append("changestock", changestock);
		builder.append("oldstock",	  oldstock);
		builder.append("newstock",    newstock);

		db_log::instance().push_insert(game_log_pump_chang_room_stock_log, builder.obj());

		db_game::instance().remove(DB_DRAGON_TIGER_ROOMCFG, BSON("room_id"<<RoomID->get_value()));
	}
}

void dragon_tiger_space::logic_room::record_room_win_lose_info(int info)
{
	m_list_balance_win_lose_info.push_front(info);

	if (m_list_balance_win_lose_info.size() > 300)
	{
		m_list_balance_win_lose_info.pop_back();
	}
// 测试反序遍历
// 	std::list<int>::reverse_iterator rit = m_list_balance_win_lose_info.rbegin();
// 	for (; rit != m_list_balance_win_lose_info.rend(); rit++)
// 	{
// 		int  a = (*rit);
// 		int  b = 0;
// 	}
}

int dragon_tiger_space::logic_room::get_dragon_counts()
{
	std::list<int>::iterator it = m_list_balance_win_lose_info.begin();
	int temp_record = 0;
	int dragon_win_count = 0;
	for (; it != m_list_balance_win_lose_info.end(); it++)
	{
		if (temp_record > 20) break;//只统计20局的
		if ((*it) == 2) dragon_win_count++;
		temp_record++;
	}

	return dragon_win_count;
}

int dragon_tiger_space::logic_room::get_tiger_counts()
{
	std::list<int>::iterator it = m_list_balance_win_lose_info.begin();
	int temp_record = 0;
	int tiger_win_count = 0;
	for (; it != m_list_balance_win_lose_info.end(); it++)
	{
		if (temp_record > 20) break;//只统计20局的
		if ((*it) == 3) tiger_win_count++;
		temp_record++;
	}

	return tiger_win_count;
}

std::list<int> & dragon_tiger_space::logic_room::get_history_pokes_info()
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
				auto sendmsg = PACKET_CREATE(dragon_tiger_protocols::packetl2c_clean_out, dragon_tiger_protocols::e_mst_l2c_clean_out);
				if (sendmsg)
				{
					sendmsg->set_reason((dragon_tiger_protocols::e_msg_cleanout_def)7);
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
	return rich_player;
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
	return rich_player;
}

void logic_room::update_playerlist()
{
	auto player_list = get_player_list();

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
	std::set<uint32_t> pids;
	pids.insert(best_win.first);
	pids.insert(rich.first);
	for (auto& it : m_player_list)
	{
		pids.insert(it.first);
	}
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
