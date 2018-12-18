#include "stdafx.h"
#include "logic_room.h"
#include "Cows_RoomCFG.h"
#include "game_db.h"
#include "logic_player.h"
#include "logic_main.h"
#include "proc_cows_logic.h"

#include "i_game_ehandler.h"
#include "enable_random.h"
#include "Cows_RobotCFG.h"
#include "logic_banker.h"
#include "logic_player.h"
#include "logic_robot.h"
#include "logic2logsvr_msg_type.pb.h"

//@add by Big O 2017/01/08;
//@在线统计增加;
#include "game_db_log.h"

COWS_SPACE_USING

logic_room::logic_room(const Cows_RoomCFGData* cfg, logic_lobby* _lobby, int child_id)
{
	m_cut_round = 20;
	m_child_id = child_id;
	m_server_stauts = 0;
	m_kill_points_switch = false;
	init_game_object();
	m_lobby = _lobby;
	m_cfg = cfg;
	m_nMaxPlayerCnt = cfg->mMaxPlayerCnt;

	int val = (cfg->mRoomID << 4) | child_id;

	RoomID->set_value(val);

	if(!load_room())
		create_room();

	MaxEarnRate->set_value((double)m_cfg->mMaxEarnRate / 100);
	ExpectEarnRate->set_value((double)m_cfg->mExpectEarnRate / 100);
	MinEarnRate->set_value((double)m_cfg->mMinEarnRate / 100);

	m_main = new logic_main(this);
	m_main->init_history(History, HistoryLogTime->get_value());
	//m_main->kill_points(m_cfg->mKillPointsSwtch);
	m_checksave = 0.0;
	m_today_win_gold = 0;
	m_today_lose_gold = 0;
	m_today_bet_gold = 0;
	m_avg_bet_gold = 0;

	//机器人
	m_robot_elapsed = global_random::instance().rand_double(5, 15);
	m_banker_robot_interval = 120;
}


logic_room::~logic_room(void)
{
	SAFE_DELETE(m_main);
}

const Cows_RoomCFGData* logic_room::get_roomcfg()
{
	return m_cfg;
}

uint32_t logic_room::get_id()
{
	return m_cfg->mRoomID;
}

void logic_room::heartbeat( double elapsed )
{
	if(m_players.size() == 0) 
		return;

	m_main->heartbeat(elapsed);
	robot_heartbeat(elapsed);
	m_checksave += elapsed;
	if (m_checksave > 30)
	{
		reflush_rate();		//调整预期盈利率
		reflush_history();
		store_game_object();
		m_checksave=0.0;
	}
}

void logic_room::robot_heartbeat(double elapsed)
{
	static int isOpen = Cows_RobotCFG::GetSingleton()->GetData("IsOpen")->mValue;
	if (isOpen == 0)
		return;

	static int minCount = Cows_RobotCFG::GetSingleton()->GetData("RobotMinCount")->mValue;
	static int maxCount = Cows_RobotCFG::GetSingleton()->GetData("RobotMaxCount")->mValue;
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
				game_engine::instance().release_robot(it->first);
			}
		}

		if (m_players.size() < 10 && m_robot_players.size() < robotCount)
		{
			request_robot();
		}

		m_robot_elapsed = global_random::instance().rand_double(5, 15);
	}
	int robot_banker_size = m_main->get_robot_banker_size();

	m_banker_robot_interval -= elapsed;
	if (robot_banker_size < 3 && m_banker_robot_interval < 0)
	{
		this->request_robot(4);
		m_banker_robot_interval = 20;
	}
}

void logic_room::request_robot(int level)
{
	static int minVIP = Cows_RobotCFG::GetSingleton()->GetData("RobotMinVip")->mValue;
	static int maxVIP = Cows_RobotCFG::GetSingleton()->GetData("RobotMaxVip")->mValue;
	int32_t vip_level = 0;
	if (level > 0)
	{
		vip_level = level;
	}
	else
	{
		vip_level = global_random::instance().rand_int(minVIP, maxVIP);
	}

	auto str = boost::format( "RobotGold%1%")%vip_level;
	GOLD_TYPE base_gold = Cows_RobotCFG::GetSingleton()->GetData(str.str().c_str())->mValue;;

	float robotGoldMinRate = (float)Cows_RobotCFG::GetSingleton()->GetData("RobotGoldMinRate")->mValue;
	float robotGoldMaxRate = (float)Cows_RobotCFG::GetSingleton()->GetData("RobotGoldMaxRate")->mValue;

	game_engine::instance().request_robot(m_cfg->mRoomID, global_random::instance().rand_int(base_gold*robotGoldMinRate, base_gold*robotGoldMaxRate), vip_level);
}

void logic_room::init_robot_bet()
{
	//没有机器人
	if (m_robot_players.size() == 0)
	{
		return;
	}

	auto banker = m_main->get_banker()->get_player();
	GOLD_TYPE max_robot_bet_gold = m_main->get_banker()->get_max_bet_gold();

	//系统上庄
	if (banker == nullptr)
	{
		int32_t min_bet_gold = Cows_RobotCFG::GetSingleton()->GetData("RobotSystemMinBet")->mValue;
		int32_t max_bet_gold = Cows_RobotCFG::GetSingleton()->GetData("RobotSystemMaxBet")->mValue;

		max_robot_bet_gold = global_random::instance().rand_int(min_bet_gold, max_bet_gold);
	}
	//机器人上庄
	else if (banker->is_robot())
	{
		float min_bet_gold_percent = Cows_RobotCFG::GetSingleton()->GetData("RobotRobotMinBet")->mValue/100.0f;
		float max_bet_gold_percent = Cows_RobotCFG::GetSingleton()->GetData("RobotRobotMaxBet")->mValue/100.0f;

		max_robot_bet_gold = global_random::instance().rand_int(max_robot_bet_gold*min_bet_gold_percent, max_robot_bet_gold*max_bet_gold_percent)/m_robot_players.size();
	}
	//玩家上庄
	else
	{
		float min_bet_gold_percent = Cows_RobotCFG::GetSingleton()->GetData("RobotPlayerMinBet")->mValue/100.0f;
		float max_bet_gold_percent = Cows_RobotCFG::GetSingleton()->GetData("RobotPlayerMaxBet")->mValue/100.0f;

		max_robot_bet_gold = global_random::instance().rand_int(max_robot_bet_gold*min_bet_gold_percent, max_robot_bet_gold*max_bet_gold_percent)/m_robot_players.size();
	}

	for (auto it = m_robot_players.begin(); it != m_robot_players.end(); ++it)
	{
		it->second->get_robot()->set_max_bet_gold(max_robot_bet_gold);
	}
}

void logic_room::reflush_history()
{
	HistoryLogTime->set_value(m_main->get_history_log_time());

	const logic_main::total_history_info& total_history = m_main->get_total_history_info();

	History->clear();
	History->put(m_main->get_history_total_count());
	History->put(total_history.m_total_win[0]);
	History->put(total_history.m_total_win[1]);
	History->put(total_history.m_total_win[2]);
	History->put(total_history.m_total_win[3]);
	History->put(total_history.m_total_lose[0]);
	History->put(total_history.m_total_lose[1]);
	History->put(total_history.m_total_lose[2]);
	History->put(total_history.m_total_lose[3]);

	std::list<logic_main::history_info>& history_infos = m_main->get_history_infos();
	for (auto it = history_infos.begin(); it != history_infos.end(); it++)
	{
		History->put(it->m_is_win[0]);
		History->put(it->m_is_win[1]);
		History->put(it->m_is_win[2]);
		History->put(it->m_is_win[3]);
	}
}

void cows_space::logic_room::kill_points(int32_t cutRound, bool status)
{
	//if (m_kill_points_switch == status)
	//{
	//	return;
	//}
	//double expect_rate = ExpectEarnRate->get_value();
	//double max_rate = MaxEarnRate->get_value();
	//double min_rate = MinEarnRate->get_value();
	m_kill_points_switch = status;
	if (m_kill_points_switch == true)
	{
		//ExpectEarnRate->set_value((double)(expect_rate * 2));
		//MaxEarnRate->set_value((double)(max_rate * 2));
		//MinEarnRate->set_value((double)(min_rate * 2));
		if (cutRound <= 0)
		{
			return;
		}
		m_cut_round =cutRound;
	}
	else
	{
		//ExpectEarnRate->set_value((double)(expect_rate / 2));
		//MaxEarnRate->set_value((double)(max_rate / 2));
		//MinEarnRate->set_value((double)(min_rate / 2));
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
				auto sendmsg = PACKET_CREATE(packetl2c_kick_player, e_mst_l2c_kick_player);
				if (sendmsg)
				{
					sendmsg->set_reason((e_msg_kick_player)7);
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

std::string cows_space::logic_room::get_name()
{
    return m_cfg->mRoomName;
}

uint16_t logic_room::get_cur_cout()
{
	static uint16_t exCnts[6]={35,28,19,8,0,0};

	uint16_t nCnt = m_players.size()*2+exCnts[m_cfg->mRoomID-1];
	
	return nCnt<200?nCnt:200;
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

LPLAYER_MAP logic_room::get_otherplayers_without_banker(LPlayerPtr& lcplayer)
{
	LPLAYER_MAP map_players=m_players;
	auto game_main = get_game_main();

	/*
	for (auto it = map_players.begin(); it != map_players.end(); ++it)
	{
		if (lcplayer==it->second)
		{
			map_players.erase(it);
			break;
		}
	}*/

	for (auto it = map_players.begin(); it != map_players.end(); ++it)
	{
		if (it->first==game_main->get_banker()->get_player_id())
		{
			map_players.erase(it);
			break;
		}
	}

	return map_players;
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
		//有上庄的玩家才开始游戏;
		if (m_main->check_banker())
		{
			m_main->pre_start_game();
		}
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
	
	//所有玩家都离开了
	if (m_players.size() == 0)
	{
		if (m_main != nullptr)
		{
			m_main->stop_game();
		}
	}
}

const Cows_RoomCFGData* logic_room::get_data() const
{
	return m_cfg;
}

int logic_room::broadcast_msg_to_client(std::vector<uint32_t>& pids, uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg)
{
	return game_engine::instance().get_handler()->broadcast_msg_to_client(pids, packet_id, msg);
}

void logic_room::sync_player_gold()
{
	for (auto it = m_players.begin(); it != m_players.end(); ++it)
	{
		it->second->sync_change_gold();
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

int logic_room::get_earn_type(GOLD_TYPE bet_gold, logic2logsvr::CowsProfitInfo& profitInfo)
{
	if (bet_gold < m_avg_bet_gold*0.9)
	{
		m_avg_bet_gold = m_avg_bet_gold * 0.9 + bet_gold * 0.1;
		//return 0; //m_avg_bet_gold 好像没用;
	}
	else
	{
		m_avg_bet_gold = m_avg_bet_gold * 0.9 + bet_gold * 0.1;
	}
	//盈利需要加上机器人的输赢;
	double win_gold = TotalWinGold->get_value() + TotalRobotWinGold->get_value();
	double lose_gold = TotalLoseGold->get_value() + TotalRobotLoseGold->get_value();
	double profit = win_gold - lose_gold;
	
	//@：复盘日志：杀分信息;
	profitInfo.set_wingold(win_gold);
	profitInfo.set_losegold(lose_gold);
	profitInfo.set_profit(profit);

	SLOG_CRITICAL << "------- win_gold: " << win_gold << ", lose_gold: " << lose_gold << ", profit: " << profit;
	if (profit <= 0)
	{
		profitInfo.set_killpointstatus(1);
		SLOG_CRITICAL <<"###### Kill points, profit: "<< profit << "  < 0";
		return -1; //-2庄家必杀有问题;
	}
	double min_rate = MinEarnRate->get_value();
	double expect_rate = ExpectEarnRate->get_value();
	double max_rate = MaxEarnRate->get_value();
	double curEarnRate = (double)(profit)/(double)win_gold;

	profitInfo.set_minrate(min_rate);
	profitInfo.set_expectrate(expect_rate);
	profitInfo.set_maxrate(max_rate);
	profitInfo.set_currate(curEarnRate);

	//不使用盈利率杀分;
	return 0;
	//小于最小盈利率;
	if (curEarnRate< min_rate)
	{
		profitInfo.set_killpointstatus(1);
		SLOG_CRITICAL << "###### Kill points, curEarnRate : " << curEarnRate << "< MinEarnRate :" << min_rate;
		return -1;
	}
	//低于最低默认盈利率;
	else if (curEarnRate < expect_rate)
	{
		int rand_value = global_random::instance().rand_100();

		double floatingRate = expect_rate - min_rate;
		int rate = ((curEarnRate - min_rate)/floatingRate) * 100.0f;

		profitInfo.set_overminraterandval(rand_value);
		profitInfo.set_overminratecalcval(rate);
		
		if (rand_value > rate)
		{
			profitInfo.set_killpointstatus(1);
			SLOG_CRITICAL << "###### Kill points, rand_value: " << rand_value <<" > rate: " << rate;
			return -1;
		}
		else
		{
			profitInfo.set_killpointstatus(0);
			SLOG_CRITICAL << "###### Don't kill points, rand_value: " << rand_value << " <=  rate: " << rate;
			return 0;
		}
	}
	//高于最高盈利率;
	if (curEarnRate > max_rate)
	{
		profitInfo.set_killpointstatus(0);
		SLOG_CRITICAL << "###### Don't kill points, curEarnRate:  " << curEarnRate << " >  MaxEarnRate: " << max_rate;
		return 1;
	}
	profitInfo.set_killpointstatus(0);
	SLOG_CRITICAL << "###### Don't kill points, curEarnRate: " << curEarnRate << " <  ExpectEarnRate: " << ExpectEarnRate->get_value();
	return 0;
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
	else if (i == 3)
	{
		if (win_gold>0)
		{
			WinGold4->add_value(win_gold);
		}
		else
		{
			LoseGold4->add_value(-win_gold);
		}
		WinCount4->add_value(win?1:0);
	}
}

void logic_room::log_banker_win_gold(GOLD_TYPE gold)
{
	BankerAddGold->add_value(gold);
}

void logic_room::log_banker_lost_gold(GOLD_TYPE gold)
{
	BankerSubGold->add_value(gold);
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
	printf("4     win = %I64d, lose = %I64d, earnings= %I64d, pump=%f\n", WinGold4->get_value(), LoseGold4->get_value(), WinGold4->get_value() - LoseGold4->get_value(), (float)(WinGold4->get_value() - LoseGold4->get_value())/WinGold4->get_value());
}

void logic_room::reflush_rate()
{
	static auto ff = BSON("ExpectEarnRate"<<1<<"MaxEarnRate"<<1<<"MinEarnRate"<<1<<"room_income"<<1);
	mongo::BSONObj b = db_game::instance().findone(DB_COWSROOM, BSON("room_id"<<RoomID->get_value()), &ff);
	if(!b.isEmpty() && b.hasField("ExpectEarnRate"))
	{
		//读room cfg 配置文件;
		//MaxEarnRate->set_value(b.getField("MaxEarnRate").Double(), false);
		//ExpectEarnRate->set_value(b.getField("ExpectEarnRate").Double(), false);
		//MinEarnRate->set_value(b.getField("MinEarnRate").Double(), false);

		if(b.hasField("room_income") == false || b.getField("room_income").Long() <0)
		{
			BankerAddGold->set_value(0);
			BankerSubGold->set_value(0);
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
			WinGold4->set_value(0);
			LoseGold4->set_value(0);
			WinCount4->set_value(0);
		}
	}	
}

//////////////////////////////////////////////////////////////////////////
void logic_room::create_room()
{
	MaxEarnRate->set_value(0.10);
	ExpectEarnRate->set_value(0.05);
	MinEarnRate->set_value(0.01);

	BankerAddGold->set_value(0);
	BankerSubGold->set_value(0);
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
	WinGold4->set_value(0);
	LoseGold4->set_value(0);
	WinCount4->set_value(0);
}

bool logic_room::load_room()
{
	mongo::BSONObj b = db_game::instance().findone(DB_COWSROOM, BSON("room_id"<<RoomID->get_value()));
	if(b.isEmpty())
		return false;	

	return from_bson(b);
}

void logic_room::init_game_object()
{
	RoomID = CONVERT_POINT(Tfield<int16_t>, regedit_tfield(e_got_int16, "room_id"));
	BankerAddGold = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "BankerAddGold"));
	BankerSubGold = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "BankerSubGold"));
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

	WinGold4 = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "WinGold4"));
	LoseGold4 = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "LoseGold4"));
	WinCount4 = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "WinCount4"));

	MaxEarnRate = CONVERT_POINT(Tfield<double>, regedit_tfield(e_got_double, "MaxEarnRate"));
	ExpectEarnRate = CONVERT_POINT(Tfield<double>, regedit_tfield(e_got_double, "ExpectEarnRate"));
	MinEarnRate = CONVERT_POINT(Tfield<double>, regedit_tfield(e_got_double, "MinEarnRate"));
	History = regedit_intlistfield("History");
	HistoryLogTime = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "HistoryLogTime"));
}

bool logic_room::store_game_object(bool to_all)
{
	if(!has_update())
		return true;

	auto err = db_game::instance().update(DB_COWSROOM, BSON("room_id"<<RoomID->get_value()), BSON("$set"<<to_bson(to_all)));
	if(!err.empty())
	{
		SLOG_ERROR << "logic_room::store_game_object :" <<err;
		return false;
	}

	return true;
}