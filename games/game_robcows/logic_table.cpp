#include "stdafx.h"
#include "logic_table.h"
#include "logic_player.h"

#include "logic_room.h"
#include "game_engine.h"
#include "i_game_ehandler.h"
#include "game_db.h"
#include "logic_lobby.h"
#include <net/packet_manager.h>


#include "robcows_def.pb.h"
#include "robcows_logic.pb.h"
#include "robcows_robot.pb.h"

#include "enable_random.h"

#include "robot_mgr.h"
#include <random>
#include "time_helper.h"

ROBCOWS_SPACE_USING

static const int MAX_TALBE_PLAYER = 5;//桌子人数
static const int MAX_OP_PLAYER = 4;//观战人数

#define MAX_TIME	180



logic_table::logic_table(void)
:m_room(nullptr)
,m_players(GAME_PLAYER)//每个桌子4个人
,m_player_count(0)
,m_elapse(0.0)
,m_checksave(0)
,m_cbKillType(0)
,m_cbKillRound(0)
,m_cbGameRound(0)
,m_clean_elapsed(0)
{
	
	init_game_object();

	initGame();
	
	// 设置 Chair ID
	for (int32_t i = 0; i < GAME_PLAYER; i++)
	{
		m_VecChairMgr.push_back(i);
	}
	// Robot
	memset(&m_robotcfg,0,sizeof(m_robotcfg));

}


logic_table::~logic_table(void)
{
}

void logic_table::release()
{
	store_game_object();
	
}

void logic_table::init_table(uint16_t tid, logic_room* room)
{
	// 保存当前房间信息
	m_room = room;
	// 设置 Table ID
	TableID->set_value(tid);
	
	if(!load_table())
		create_table();

	set_status(eGameState_Free);
	//
	m_StartTime = RobCows_BaseInfo::GetSingleton()->GetData("StartTime")->mValue;
	m_BankerTime = RobCows_BaseInfo::GetSingleton()->GetData("BankerTime")->mValue;
	m_BetTime = RobCows_BaseInfo::GetSingleton()->GetData("BetTime")->mValue;
	m_OpenTime = RobCows_BaseInfo::GetSingleton()->GetData("OpenCardTime")->mValue;
	m_nDisplayTime = RobCows_BaseInfo::GetSingleton()->GetData("DisplayTime")->mValue/10.0;
	m_ResultTime = RobCows_BaseInfo::GetSingleton()->GetData("ResultTime")->mValue;
	// Robot
	const boost::unordered_map<int, RobCows_RobotCFGData>& list = RobCows_RobotCFG::GetSingleton()->GetMapData();
	for (auto it = list.begin(); it != list.end(); ++it)
	{
		if(it->second.mRoomID == m_room->get_id())
		{
			m_robotcfg = it->second;
			break;
		}
	}
	m_robot_elapsed = global_random::instance().rand_int(m_robotcfg.mRobotMinEntry, m_robotcfg.mRobotMaxEntry);
}

uint32_t logic_table::get_id()
{
	return TableID->get_value();
}

int8_t logic_table::get_status()
{

	return TableStatus->get_value();
}

void logic_table::set_status(int8_t state)
{
	TableStatus->set_value(state);
}

void logic_table::inc_dec_count(bool binc/* = true*/)
{
	if(binc)
	{
		m_player_count++;
	}
	else
	{
		m_player_count--;

		if(m_player_count == 0)
			store_game_object();
	}
}

void logic_table::heartbeat( double elapsed )
{
	if(m_player_count==0)
		return;

	// Robot
	robot_heartbeat(elapsed);

	//同步协议
	m_elapse += elapsed;
	
	m_checksave+= elapsed;
	if (m_checksave > 30)//桌子信息30s保存1次
	{
		store_game_object();
	}

	//////////////////////////////////////////////////////////////////////////
	if(m_duration>0)
	{
		m_duration -= elapsed;
	}
	int8_t nStatus = get_status();
	switch (nStatus)
	{
	case eGameState_Free:
		{
			
			int nPlayerCount = GetJoinCount();
			int nReadyCount = GetActiveUserCount();
			if(nPlayerCount>=PLAY_MIN_COUNT )
			{
				onGameNoticeSpare();
			}
			else
			{
				// @空闲清理
				CleanTableEvent();
			}

		}
		break;

	case eGameState_Spare:
		{
			// 
			if (m_duration <= 2.0)
			{
				// @延时清理
				CleanTableEvent();
			}
			
			if(m_duration<0)
			{
				int nPlayerCount = GetJoinCount();
				if(nPlayerCount>=PLAY_MIN_COUNT )
				{
					m_duration = 0.0;
					onGameNoticeRobBanker();
				}
				else
				{
					m_duration = 0;
					set_status(eGameState_Free);
				}
			}
		}
		break;

	case eGameState_Banker:
		{
			if (m_duration < 0)
			{
				m_duration = 0;
				// 默认抢庄 0
				defaultRobBanker();
			}
		}
		break;

	case eGameState_Bet:
		{
			if (m_duration < 0)
			{
				m_duration = 0;
				// 默认下注最小
				defaultBetRate();
			}
		}
		break;

	case eGameState_FaPai:
		{
// 			if(m_duration<0)
// 			{
// 				m_duration = 0.0;
// 				onEventGameStart();
// 			}
		}
		break;

	case eGameState_OpenCard:
		{
			if(m_duration < 0)
			{
				m_duration = 0;
				defaultOpenCard();
			}
		}
		break;
	case eGameState_Display:
		{
			if(m_duration < 0)
			{
				m_duration = 0;
				SLOG_ERROR << "err : eGameState_Display ";
			}
		}
		break;
	case eGameState_End:
		{
			if (m_duration<0)
			{
				m_duration = 0.0;

				set_status(eGameState_Free);
				// 桌子复位
				repositGame();
			}
		}
		break;
	default:
		break;
	}

	// @空闲状态
	if (nStatus == eGameState_Free) return;
	// @解散
	m_clean_elapsed += elapsed;
	if (m_clean_elapsed > TIME_CLEAN)
	{
		SLOG_ERROR << boost::format("[RepositGame]====>>>>");
		SLOG_ERROR << boost::format("roomid:%1%,tableid:%2%,status:%3%") % m_room->get_id() % this->get_id() % ((int32_t)this->get_status());
		//////////////////////////////
		for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
		{
			auto &user = it->second;
			auto player = user.p_playerptr;
			auto playerid = user.p_id;
			if (player == nullptr) continue;
			auto leave = user.p_leave;
			auto status = player->get_status();
			SLOG_ERROR << boost::format("[RepositGame]====>>>>playerid:%1%,leave:%2%,status:%3%") % playerid % leave % status;
		}
		SLOG_ERROR << boost::format("[RepositGame]<<<<====");
		//////////////////
		set_status(eGameState_Free);
		// 桌子复位
		repositGame();
	}
	// TEST

}

bool logic_table::is_full()
{
	return m_player_count >= GAME_PLAYER;
}

bool logic_table::is_opentable()
{
	return m_player_count >= 1;
}

uint32_t logic_table::all_robot()
{
	int32_t nCount = 0;
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if(player && player->is_robot()) 
			nCount++;
	}
	return nCount;
}

uint32_t logic_table::all_human()
{
	int32_t nCount = 0;
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if (player && !player->is_robot())
			nCount++;
	}
	return nCount;
}

uint32_t logic_table::all_extraman()
{
	return m_MapExtraPlayer.size();
}

bool logic_table::is_all_robot()
{
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if(player && !player->is_robot()) 
			return false;
	}
	return true;
}

unsigned int logic_table::get_max_table_player()
{
	return m_players.size();
}

LPlayerPtr& logic_table::get_player(int index)
{
	if(isVaildSeat(index))
		return m_players[index];

	return logic_player::EmptyPtr;

}

LPlayerPtr& logic_table::get_player_byid(uint32_t pid)
{
	for (int i = 0; i < GAME_PLAYER; i++)
	{
		if (m_players[i] != nullptr && m_players[i]->get_pid() == pid)
		{
			return m_players[i];
		}
	}
	return logic_player::EmptyPtr;
}

void logic_table::PlatformKiller(int32_t optype, int32_t cutRound)
{
	if (m_player_count > 0)
	{
		m_cbKillRound = 0;
		m_cbGameRound = 0;
	}
}

logic_room* logic_table::get_room()
{
	return m_room;
}



int32_t logic_table::get_seat_byid(uint32_t pid)
{
	int32_t seat = INVALID_CHAIR;
	auto it = m_MapTablePlayer.find(pid);
	if(it != m_MapTablePlayer.end())
	{
		seat = it->second.p_idx;
	}
	return seat;
}

uint32_t robcows_space::logic_table::get_id_byseat(int32_t seat)
{
	uint32_t uid = 0;
	for (auto user : m_MapTablePlayer)
	{
		if(user.second.p_idx == seat)
		{
			uid = user.second.p_id;
			break;
		}
	}
	return uid;
}

int logic_table::enter_table(LPlayerPtr player)
{
	if(player->get_table()!=nullptr) 
	{
		SLOG_ERROR<<"logic_table enter_table is in table";
		return 2;
	}
#if 1
	// 随机分配椅子
	if(m_VecChairMgr.size()>0)
	{
		std::random_shuffle(std::begin(m_VecChairMgr),std::end(m_VecChairMgr));
		int32_t seat = m_VecChairMgr.back();
		m_VecChairMgr.pop_back();
		// find seat -- order by 
	//	bc_enter_seat(seat, player);
		// 
		uint32_t id = player->get_pid();
		m_pids.push_back(player->get_pid());

		inc_dec_count();
		//
		TablePlayer table_player;
		memset(&table_player,0,sizeof(table_player));
		table_player.p_playerptr = player;
		table_player.p_idx = seat;
		uint32_t uid = player->get_pid();
		table_player.p_id  = uid;
		table_player.p_asset = player->get_gold();
		table_player.p_leave = false;
		table_player.p_background = false;
		table_player.p_kick = false;
		table_player.p_norob = false;
		table_player.p_nobet = false;
		table_player.p_noopen = false;
		table_player.p_noaction = 0;
		tagPlayerCtrlAttr attr;
		bool bRet = player->get_player_ctrl(attr);
		int32_t nTag = Tag_UserA;
		if(bRet)
		{
			nTag = attr.nTag;
		}
		table_player.p_tag = nTag;
		table_player.p_percent = attr.fwinPercent;
		table_player.p_leave = false;
		table_player.p_runaway = false;
		table_player.p_active = FALSE;
		if (player->is_robot())
		{
// 			std::random_device rd;
// 			std::mt19937_64 g(rd());
// 			std::uniform_int_distribution<> dis(m_robotcfg.mRobotMinRound, m_robotcfg.mRobotMaxRound);
// 			int32_t nRound = dis(g);
			int32_t nRound = global_random::instance().rand_int(m_robotcfg.mRobotMinRound, m_robotcfg.mRobotMaxRound);
			table_player.p_round = nRound;
			
		}
		else
		{	
			table_player.p_round = 0;
			
		}
		m_MapTablePlayer.insert(std::make_pair(uid,table_player));

		if(get_status() == eGameState_Banker || get_status() == eGameState_Bet || get_status() == eGameState_OpenCard )
		{
			player->set_status(eUserState_Wait);
		}
		else
		{
			player->set_status(eUserState_free);
		}

		SLOG_CRITICAL<<boost::format("logic_table::enter_table:%1%,seatid:%2%")%get_id()%seat;
		SLOG_CRITICAL<<boost::format("logic_table::enter_table:[Tag:%d,Win:%f]")%attr.nTag%attr.fwinPercent;
		bc_enter_seat(table_player);
		return 1;
	}
#ifdef UsingCling
	if (player && player->is_robot())
	{
	//	assert(0);
		player->join_table(nullptr, -1);
		return 12;
	}
	// @ cling to robot
	int32_t seat = cling_robotid();
	if(seat != INVALID_CHAIR)
	{ // 
		/* 1、一个位置真实玩家只附加一个
		*  2、游戏结束将附加玩家加入对局
		*/
		uint32_t id = player->get_pid();
		m_pids.push_back(player->get_pid());
		//
		TablePlayer table_player;
		memset(&table_player, 0, sizeof(table_player));
		table_player.p_playerptr = player;
		table_player.p_idx = seat;
		uint32_t uid = player->get_pid();
		table_player.p_id = uid;
		table_player.p_asset = player->get_gold();
		table_player.p_leave = false;
		table_player.p_background = false;
		table_player.p_kick = false;
		table_player.p_norob = false;
		table_player.p_nobet = false;
		table_player.p_noopen = false;
		table_player.p_noaction = 0;
		tagPlayerCtrlAttr attr;
		bool bRet = player->get_player_ctrl(attr);
		int32_t nTag = Tag_UserA;
		if (bRet)
		{
			nTag = attr.nTag;
		}
		table_player.p_tag = nTag;
		table_player.p_percent = attr.fwinPercent;
		table_player.p_leave = false;
		table_player.p_runaway = false;
		table_player.p_active = FALSE;
		table_player.p_round = 0;
		player->set_status(eUserState_Wait);
		m_MapExtraPlayer.insert(std::make_pair(seat, table_player));
		SLOG_CRITICAL << boost::format("logic_table::enter_table-2:%1%,seatid:%2%") % get_id() % seat;
		player->join_table(this, seat);
		return 1;
	}
#endif // UsingCling

#else
	// 顺序分配椅子
	for (int i =0;i<GAME_PLAYER;i++)
	{
		if(m_players[i] == nullptr)
		{	
			m_players[i] = player;
			// 
			uint32_t id = player->get_pid();
			m_pids.push_back(player->get_pid());

			inc_dec_count();

			// find seat -- order by 
			bc_enter_seat(i, player);
			// 
			TablePlayer table_player;
			memset(&table_player,0,sizeof(table_player));
			table_player.p_playerptr = player;
			table_player.p_idx = i;
			uint32_t uid = player->get_pid();
			table_player.p_id  = uid;
			table_player.p_asset = player->get_gold();
			table_player.p_tag = player->get_player_tag();
			m_MapTablePlayer.insert(std::make_pair(uid,table_player));

			SLOG_CRITICAL<<boost::format("logic_table::enter_table:%1%,seatid:%2%")%get_id()%i;
			return 1;
		}
	}
#endif

	player->join_table(nullptr,-1);
	return 12;
}

bool logic_table::leave_table(uint32_t pid)
{
#if 0
	for (int i =0;i<GAME_PLAYER;i++)
	{
		LPlayerPtr& player = get_player(i);
		if(player != nullptr && player->get_pid() == pid)
		{
			player->set_status(eUserState_null);
			player->clear_round();

			player = nullptr;
			//
			m_room->leave_table(pid);
			// 
			inc_dec_count(false);

			bc_leave_seat(pid);	
			
			break;
		}
	}
#else
	// 清理用户
	auto it = m_MapTablePlayer.find(pid);
	if(it != m_MapTablePlayer.end())
	{
		if(can_leave_table(pid)==false)
		{ // 游戏已经开始不能离开
			auto& tableuser = it->second;
			tableuser.p_runaway = true;
			return false;
		}
		auto user = it->second;
		int32_t seat = user.p_idx;
		m_VecChairMgr.push_back(seat);
		LPlayerPtr& player = user.p_playerptr;
		if(player)
		{
			player->set_status(eUserState_null);
			player->clear_round();
		}
		m_room->leave_table(pid);
		// 
		inc_dec_count(false);

		bc_leave_seat(pid);	

		m_MapTablePlayer.erase(it);
	}
#ifdef UsingCling
	// 清理等待
	for (auto itextra = m_MapExtraPlayer.begin(); itextra != m_MapExtraPlayer.end();)
	{
		auto table_player = itextra->second;
		if (table_player.p_id == pid)
		{
			int32_t seat = table_player.p_idx;
			//m_VecChairMgr.push_back(seat);
			m_room->leave_table(pid);
			m_MapExtraPlayer.erase(itextra++);

		}
		else
		{
			itextra++;
		}
	}
#endif // UsingCling

#endif
	// 清理 pid_vec
	for (unsigned int i = 0; i < m_pids.size(); i++)
	{
		if (m_pids[i] == pid)
		{
			m_pids[i] = m_pids[m_pids.size() - 1];
			m_pids.pop_back();
			break;
		}
	}

	return true;
}

bool logic_table::can_leave_table(uint32_t pid, bool bReq /*= false*/)
{
	auto it = m_MapTablePlayer.find(pid);
	if(it != m_MapTablePlayer.end())
	{
		auto& user = it->second;
		LPlayerPtr player = user.p_playerptr;
		// 
		if(user.p_active==TRUE) return false;
		//
		if(player && player->get_status()==eUserState_play)
		{
			return false;
		}
		else
		{
			if (bReq)
			{
				user.p_leave = true;
				SLOG_ERROR << boost::format("[Error:can_leave_table]pid:%1%") % pid;
			}
			return true;
		}
	}
	// 
	return true;
}

void logic_table::getout_table(uint32_t pid)
{
	auto it = m_MapTablePlayer.find(pid);
	if (it != m_MapTablePlayer.end())
	{
		auto& tuser = it->second;
		auto player = tuser.p_playerptr;
		tuser.p_kick = true;
	}
}

bool logic_table::change_sit(uint32_t pid, uint32_t seat_index)
{

	return true;
}

int logic_table::broadcast_msg_to_client(std::vector<uint32_t>& pids, uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg)
{
//	printf("broadcast_msg_to_client: %d:%d,,%d\n",pids.size(),pids[pids.size()-1],packet_id);

	return game_engine::instance().get_handler()->broadcast_msg_to_client(pids, packet_id, msg);

}


void logic_table::bc_enter_seat(int seat_index, LPlayerPtr& player)
{
	SLOG_CRITICAL<<boost::format("logic_table enter seat,seat:%d region=%s")%seat_index%player->GetUserRegion();
	
	player->join_table(this,seat_index);
	//  notice other
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_user_enter_seat,robcows_protocols::e_mst_l2c_user_enter_seat);
	auto playerinfo = sendmsg->mutable_playerinfo();
	playerinfo->set_seat_index(seat_index);
	playerinfo->set_player_id(player->get_pid());
	playerinfo->set_player_nickname(player->get_nickname());
	playerinfo->set_player_headframe(player->get_photo_frame());
	playerinfo->set_player_headcustom(player->get_icon_custom());
	playerinfo->set_player_gold(player->get_gold());
	playerinfo->set_player_sex(player->get_sex());
	playerinfo->set_player_viplv(player->get_viplvl());
	playerinfo->set_player_ticket(player->get_ticket());
	playerinfo->set_player_region(player->GetUserRegion());

	broadcast_msg_to_client2(sendmsg);

}


void robcows_space::logic_table::bc_enter_seat(TablePlayer& tabplayer)
{
	LPlayerPtr player = tabplayer.p_playerptr;
	if(!player) return;
	int seat_index = tabplayer.p_idx;
	player->join_table(this,seat_index);
	int32_t nTag = tabplayer.p_tag;
	double fPercent = tabplayer.p_percent;

	//  notice other
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_user_enter_seat,robcows_protocols::e_mst_l2c_user_enter_seat);
	auto playerinfo = sendmsg->mutable_playerinfo();
	playerinfo->set_seat_index(seat_index);
	playerinfo->set_player_id(player->get_pid());
	playerinfo->set_player_nickname(player->get_nickname());
	playerinfo->set_player_headframe(player->get_photo_frame());
	playerinfo->set_player_headcustom(player->get_icon_custom());
	playerinfo->set_player_gold(player->get_gold());
	playerinfo->set_player_sex(player->get_sex());
	playerinfo->set_player_viplv(player->get_viplvl());
	playerinfo->set_player_ticket(player->get_ticket());
	playerinfo->set_player_region(player->GetUserRegion());

	//auto playertag = playerinfo->mutable_playertag();
	//playertag->set_type(nTag);
	//playertag->set_percent(fPercent);

	broadcast_msg_to_client2(sendmsg);
}

void logic_table::bc_leave_seat(uint32_t player_id)
{
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_user_leave_seat, robcows_protocols::e_mst_l2c_user_leave_seat);
	sendmsg->set_playerid(player_id);
	broadcast_msg_to_client2(sendmsg);
}

//////////////////////////////////////////////////////////////////////////
void logic_table::create_table()
{
	
	

}

void logic_table::init_game_object()
{
	TableID = CONVERT_POINT(Tfield<int16_t>, regedit_tfield(e_got_int16, "table_id"));

	TableStatus = CONVERT_POINT(Tfield<int8_t>, regedit_tfield(e_got_int8,"table_status"));
}

bool logic_table::load_table()
{
	mongo::BSONObj b = db_game::instance().findone(DB_ROBCOWSTABLE, BSON("room_id"<<m_room->get_id() << "table_id" << TableID->get_value()));
	if(b.isEmpty())
		return false;

	return from_bson(b);
	
}

bool logic_table::store_game_object(bool to_all)
{
	if(!has_update())
		return true;

	auto err = db_game::instance().update(DB_ROBCOWSTABLE, BSON("room_id"<<m_room->get_id() << "table_id" << TableID->get_value()), BSON("$set"<<to_bson(to_all)));
	if(!err.empty())
	{
		SLOG_ERROR << "logic_table::store_game_object :" <<err;
		return false;
	}

	m_checksave = 0;

	return true;
}

void logic_table::req_scene(uint32_t playerid)
{
	// 重置 Home键
	auto player_it = m_MapTablePlayer.find(playerid);
	if (player_it != m_MapTablePlayer.end())
	{
		auto &user = player_it->second;
		user.p_background = false;
		auto player = user.p_playerptr;
		if (player) player->set_serverflag(0);
	}

}

boost::shared_ptr<robcows_protocols::packetl2c_scene_info_free> logic_table::get_scene_info_msg_free()
{
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_scene_info_free,robcows_protocols::e_mst_l2c_scene_info_free);
	auto freeinfo = sendmsg->mutable_freeinfo();
	int8_t nGameState = get_status();
	int32_t nSpareTime = 0;
	if(nGameState==eGameState_Spare)
	{
		nSpareTime = m_duration;
	}
	freeinfo->set_status(nGameState);
	freeinfo->set_roomid(m_room->get_id());
	freeinfo->set_tableid(get_id());
	freeinfo->set_sparetime(nSpareTime);
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto seat = user.p_idx;
		auto nTag = user.p_tag;
		auto fPercent = user.p_percent;
		if(player!=nullptr)
		{
			auto playerinfo = freeinfo->add_playerinfo();
			playerinfo->set_seat_index(seat);
			playerinfo->set_player_id(player->get_pid());
			playerinfo->set_player_nickname(player->get_nickname());
			playerinfo->set_player_headframe(player->get_photo_frame());
			playerinfo->set_player_headcustom(player->get_icon_custom());
			playerinfo->set_player_gold(player->get_gold());
			playerinfo->set_player_sex(player->get_sex());
			playerinfo->set_player_viplv(player->get_viplvl());
			playerinfo->set_player_ticket(player->get_ticket());
			playerinfo->set_player_status(player->get_status());
			playerinfo->set_player_region(player->GetUserRegion());

			/*auto playertag = playerinfo->mutable_playertag();
			playertag->set_type(nTag);
			playertag->set_percent(fPercent);*/

			playerinfo->clear_robrate();
			playerinfo->clear_betrate();
		}
	}

	return sendmsg;
}


boost::shared_ptr<robcows_protocols::packetl2c_scene_info_play_bank> robcows_space::logic_table::get_scene_info_msg_play_bank(uint32_t uid)
{
#ifdef UsingCling
	auto extraidx = INVALID_CHAIR;
	TablePlayer extraplayer;
	memset(&extraplayer, 0, sizeof(extraplayer));
	for (auto it = m_MapExtraPlayer.begin(); it != m_MapExtraPlayer.end(); it++)
	{
		auto findextra = it->second;
		if (findextra.p_id == uid)
		{
			extraidx = findextra.p_idx;
			extraplayer = findextra;
			break;
		}
	}
#endif // UsingCling
	// 
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_scene_info_play_bank,robcows_protocols::e_mst_l2c_scene_info_play_bank);
	auto playbank = sendmsg->mutable_playinfo();
	int8_t nGameState = get_status();
	playbank->set_status(nGameState);
	playbank->set_roomid(m_room->get_id());
	playbank->set_tableid(get_id());
	playbank->set_sparetime(m_duration);
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if (player == nullptr) continue;
		auto seat = user.p_idx;
		auto playerid = user.p_id;
		auto robrate = user.p_bankrate;
		auto playerinfo = playbank->add_playerinfo();
#ifdef UsingCling
		if (seat == extraidx)
		{
			player = extraplayer.p_playerptr;
			robrate = 0;
		}
#endif // UsingCling
		playerinfo->set_seat_index(seat);
		playerinfo->set_player_id(player->get_pid());
		playerinfo->set_player_nickname(player->get_nickname());
		playerinfo->set_player_headframe(player->get_photo_frame());
		playerinfo->set_player_headcustom(player->get_icon_custom());
		playerinfo->set_player_gold(player->get_gold());
		playerinfo->set_player_sex(player->get_sex());
		playerinfo->set_player_viplv(player->get_viplvl());
		playerinfo->set_player_ticket(player->get_ticket());
		playerinfo->set_player_status(player->get_status());
		playerinfo->set_player_region(player->GetUserRegion());
		playerinfo->set_robrate(robrate);
		playerinfo->clear_betrate();
		playerinfo->clear_opencard();

		playerinfo->clear_cardinfo();
// 		if (playerid == uid)
// 		{
// 			auto cardinfo = playerinfo->mutable_cardinfo();
// 			GameCard card;
// 			memset(&card, 0, sizeof(card));
// 			auto fresult = m_MapPlayerCard.find(uid);
// 			if (fresult != m_MapPlayerCard.end())
// 			{
// 				card = fresult->second;
// 			}
// 			int32_t nCount = MAX_CARDS_HAND - 1;
// 			cardinfo->set_pokercount(nCount);
// 			cardinfo->mutable_pokerdata()->Reserve(nCount);
// 			for (int j = 0; j < nCount; j++)
// 			{
// 				cardinfo->add_pokerdata(card.nCard[j]);
// 			}
// 			cardinfo->clear_pokertype();
// 		}
// 		else
// 		{
// 			playerinfo->clear_cardinfo();
// 		}
	}
	return sendmsg;
}

boost::shared_ptr<robcows_protocols::packetl2c_scene_info_play_bet> robcows_space::logic_table::get_scene_info_msg_play_bet(uint32_t uid)
{
#ifdef UsingCling
	auto extraidx = INVALID_CHAIR;
	TablePlayer extraplayer;
	memset(&extraplayer, 0, sizeof(extraplayer));
	for (auto it = m_MapExtraPlayer.begin(); it != m_MapExtraPlayer.end(); it++)
	{
		auto findextra = it->second;
		if (findextra.p_id == uid)
		{
			extraidx = findextra.p_idx;
			extraplayer = findextra;
			break;
		}
	}
#endif // UsingCling
	// 
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_scene_info_play_bet,robcows_protocols::e_mst_l2c_scene_info_play_bet);
	auto playbet = sendmsg->mutable_playinfo();
	int8_t nGameState = get_status();
	playbet->set_status(nGameState);
	playbet->set_roomid(m_room->get_id());
	playbet->set_tableid(get_id());
	playbet->set_bankerid(m_nBankerID);
	playbet->set_robrate(m_nBankerRate);
	playbet->set_sparetime(m_duration);
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if (player == nullptr) continue;
		auto seat = user.p_idx;
		auto playerid = user.p_id;
		auto betrate = user.p_betrate;
		auto playerinfo = playbet->add_playerinfo();
#ifdef UsingCling
		if (seat == extraidx)
		{
			player = extraplayer.p_playerptr;
			betrate = 0;
		}
#endif // UsingCling
		playerinfo->set_seat_index(seat);
		playerinfo->set_player_id(player->get_pid());
		playerinfo->set_player_nickname(player->get_nickname());
		playerinfo->set_player_headframe(player->get_photo_frame());
		playerinfo->set_player_headcustom(player->get_icon_custom());
		playerinfo->set_player_gold(player->get_gold());
		playerinfo->set_player_sex(player->get_sex());
		playerinfo->set_player_viplv(player->get_viplvl());
		playerinfo->set_player_ticket(player->get_ticket());
		playerinfo->set_player_status(player->get_status());
		playerinfo->set_player_region(player->GetUserRegion());
		playerinfo->clear_robrate();
		playerinfo->set_betrate(betrate);
		playerinfo->clear_opencard();

		playerinfo->clear_cardinfo();
// 		if (playerid == uid)
// 		{
// 			auto cardinfo = playerinfo->mutable_cardinfo();
// 			GameCard card;
// 			memset(&card, 0, sizeof(card));
// 			auto fresult = m_MapPlayerCard.find(uid);
// 			if (fresult != m_MapPlayerCard.end())
// 			{
// 				card = fresult->second;
// 			}
// 			int32_t nCount = MAX_CARDS_HAND - 1;
// 			cardinfo->set_pokercount(nCount);
// 			cardinfo->mutable_pokerdata()->Reserve(nCount);
// 			for (int j = 0; j < nCount; j++)
// 			{
// 				cardinfo->add_pokerdata(card.nCard[j]);
// 			}
// 			cardinfo->clear_pokertype();
// 		}
// 		else
// 		{
// 			playerinfo->clear_cardinfo();
// 		}
	}
	return sendmsg;
}

boost::shared_ptr<robcows_protocols::packetl2c_scene_info_play_opencard> robcows_space::logic_table::get_scene_info_msg_play_opencard(uint32_t uid)
{
#ifdef UsingCling
	auto extraidx = INVALID_CHAIR;
	TablePlayer extraplayer;
	memset(&extraplayer, 0, sizeof(extraplayer));
	for (auto it = m_MapExtraPlayer.begin(); it != m_MapExtraPlayer.end(); it++)
	{
		auto findextra = it->second;
		if (findextra.p_id == uid)
		{
			extraidx = findextra.p_idx;
			extraplayer = findextra;
			break;
		}
	}
#endif // UsingCling
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_scene_info_play_opencard,robcows_protocols::e_mst_l2c_scene_info_play_opencard);
	auto playopencard = sendmsg->mutable_playinfo();
	int8_t nGameState = get_status();
	playopencard->set_status(nGameState);
	playopencard->set_roomid(m_room->get_id());
	playopencard->set_tableid(get_id());
	playopencard->set_bankerid(m_nBankerID);
	playopencard->set_robrate(m_nBankerRate);
	playopencard->set_sparetime(m_duration);
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if (player == nullptr) continue;
		auto seat = user.p_idx;
		auto playerid = user.p_id;
		auto betrate = user.p_betrate;
		auto opencard = user.p_opencard;
		auto playerinfo = playopencard->add_playerinfo();
#ifdef UsingCling
		if (seat == extraidx)
		{
			player = extraplayer.p_playerptr;
			playerinfo->set_seat_index(seat);
			playerinfo->set_player_id(player->get_pid());
			playerinfo->set_player_nickname(player->get_nickname());
			playerinfo->set_player_headframe(player->get_photo_frame());
			playerinfo->set_player_headcustom(player->get_icon_custom());
			playerinfo->set_player_gold(player->get_gold());
			playerinfo->set_player_sex(player->get_sex());
			playerinfo->set_player_viplv(player->get_viplvl());
			playerinfo->set_player_ticket(player->get_ticket());
			playerinfo->set_player_status(player->get_status());
			playerinfo->set_player_region(player->GetUserRegion());
			playerinfo->clear_robrate();
			playerinfo->set_betrate(0);
			playerinfo->clear_cardinfo();
		}
		else
		{
#endif // UsingCling
			playerinfo->set_seat_index(seat);
			playerinfo->set_player_id(player->get_pid());
			playerinfo->set_player_nickname(player->get_nickname());
			playerinfo->set_player_headframe(player->get_photo_frame());
			playerinfo->set_player_headcustom(player->get_icon_custom());
			playerinfo->set_player_gold(player->get_gold());
			playerinfo->set_player_sex(player->get_sex());
			playerinfo->set_player_viplv(player->get_viplvl());
			playerinfo->set_player_ticket(player->get_ticket());
			playerinfo->set_player_status(player->get_status());
			playerinfo->set_player_region(player->GetUserRegion());
			playerinfo->clear_robrate();
			playerinfo->set_betrate(betrate);
			playerinfo->set_opencard(opencard);
			if(playerid==uid || opencard)
			{
				auto cardinfo = playerinfo->mutable_cardinfo();
				GameCard card;
				memset(&card,0,sizeof(card));
				auto fresult = m_MapPlayerCard.find(playerid);
				if (fresult != m_MapPlayerCard.end())
				{
					card = fresult->second;
				}
				int32_t nCount = MAX_CARDS_HAND;
				cardinfo->set_pokercount(nCount);
				cardinfo->mutable_pokerdata()->Reserve(nCount);
				cardinfo->mutable_pokerdata2()->Reserve(nCount);
				for (int j=0;j<nCount;j++)
				{
					cardinfo->add_pokerdata(card.nCowsCard[j]);
					cardinfo->add_pokerdata2(card.nCowsCard[j]);
				}
				cardinfo->set_pokertype(card.nModel);
			}
			else
			{
				playerinfo->clear_cardinfo();
			}
#ifdef UsingCling
		}
#endif // UsingCling
	}

	return sendmsg;
}

boost::shared_ptr<robcows_protocols::packetl2c_scene_info_play_display> robcows_space::logic_table::get_scene_info_msg_play_display()
{
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_scene_info_play_display,robcows_protocols::e_mst_l2c_scene_info_play_display);
	auto playopencard = sendmsg->mutable_playinfo();
	int8_t nGameState = get_status();
	playopencard->set_status(nGameState);
	playopencard->set_roomid(m_room->get_id());
	playopencard->set_tableid(get_id());
	playopencard->set_bankerid(m_nBankerID);
	playopencard->set_robrate(m_nBankerRate);
	playopencard->set_sparetime(m_duration);
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto seat = user.p_idx;
		auto uid = user.p_id;
		auto betrate = user.p_betrate;
		auto opencard = user.p_opencard;
		if(player!=nullptr)
		{
			auto playerinfo = playopencard->add_playerinfo();
			playerinfo->set_seat_index(seat);
			playerinfo->set_player_id(player->get_pid());
			playerinfo->set_player_nickname(player->get_nickname());
			playerinfo->set_player_headframe(player->get_photo_frame());
			playerinfo->set_player_headcustom(player->get_icon_custom());
			playerinfo->set_player_gold(player->get_gold());
			playerinfo->set_player_sex(player->get_sex());
			playerinfo->set_player_viplv(player->get_viplvl());
			playerinfo->set_player_ticket(player->get_ticket());
			playerinfo->set_player_status(player->get_status());
			playerinfo->set_player_region(player->GetUserRegion());
			playerinfo->clear_robrate();
			playerinfo->set_betrate(betrate);
			playerinfo->clear_opencard();

			// card info
			auto cardinfo = playerinfo->mutable_cardinfo();
			GameCard card;
			memset(&card,0,sizeof(card));
			auto fresult = m_MapPlayerCard.find(uid);
			if (fresult != m_MapPlayerCard.end())
			{
				card = fresult->second;
			}

			int32_t nCount = MAX_CARDS_HAND;
			cardinfo->set_pokercount(nCount);
			cardinfo->mutable_pokerdata()->Reserve(nCount);
			for (int j=0;j<nCount;j++)
			{
				cardinfo->add_pokerdata(card.nCowsCard[j]);
			}
			cardinfo->set_pokertype(card.nModel);
		}
	}

	return sendmsg;
}


boost::shared_ptr<robcows_protocols::packetl2c_scene_info_play> robcows_space::logic_table::get_scene_info_msg_play(uint32_t uid)
{
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_scene_info_play,robcows_protocols::e_mst_l2c_scene_info_play);
	// 

	return sendmsg;
}

template<class T>
void robcows_space::logic_table::broadcast_msg_to_client2(T msg)
{
#if 0
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if (player != nullptr )
		{
#if 0
			player->send_msg_to_client(msg);
#else
			if(!player->is_robot())
			{
				player->send_msg_to_client(msg);
			}
			else
			{
				// Robot
				robot_mgr::instance().recv_packet(player->get_pid(),msg->packet_id(),msg);
			}
#endif
		}
	}
#else
	Vec_UserID userpids;
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if (player != nullptr)
		{
			auto uid = player->get_pid();
			if (!player->is_robot())
			{
				userpids.push_back(uid);
			}
			else
			{
				robot_mgr::instance().recv_packet(uid, msg->packet_id(), msg);
			}
		}
	}
	if (userpids.size() > 0)
	{
		game_engine::instance().get_handler()->broadcast_msg_to_client(userpids, msg->packet_id(), msg);
	}

#endif
}

int32_t robcows_space::logic_table::GetActiveUserCount()
{
	int32_t nActiveCount = 0;
	for(auto user : m_MapTablePlayer)
	{
		if(user.second.p_active == TRUE)
			nActiveCount++;
	}

	return nActiveCount;
}

int32_t robcows_space::logic_table::GetAllUserCount()
{
	
	return m_MapTablePlayer.size();
}


int32_t robcows_space::logic_table::GetJoinCount()
{
	int32_t nCount = 0;
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		if(player->get_gold() < m_room->get_data()->mGoldMinCondition)			
			continue;
		if(user.p_leave==true) 
			continue;

		++nCount;
	}
	return nCount;
}

bool robcows_space::logic_table::onEventUserReady(uint32_t playerid)
{
	int32_t nResult = eReadyResult_Succeed;
	//
	auto it = m_MapTablePlayer.find(playerid);
	if(it == m_MapTablePlayer.end())
	{
		SLOG_CRITICAL<<"logic_table::onEventUserReady Can't Ready:nullptr";
		nResult = eReadyResult_Faild_unknow;
	}
	else
	{
		auto& tuser = it->second;
		auto player = tuser.p_playerptr;
		if(player->is_robot())
		{
			SLOG_CRITICAL<<boost::format("logic_table::onEventUserReady Robot-id:%1%")%playerid;
		}
		if(player->get_status()>eUserState_free)
		{
			SLOG_CRITICAL<<"logic_table::onEventUserReady Can't Ready:usestate";
			nResult = eReadyResult_Faild_userstate;
		}
		else if(get_status()!=eGameState_Free && get_status()!=eGameState_Spare)
		{
			SLOG_CRITICAL<<"logic_table::onEventUserReady Can't Ready:gamestate";
			nResult = eReadyResult_Faild_gamestate;
		}
		else if(player->get_gold()< m_room->get_data()->mGoldMinCondition)
		{
			SLOG_CRITICAL<<"logic_table::onEventUserReady Can't Ready:gold";
			nResult = eReadyResult_Faild_gold;
		}
		else
		{
			// succeed
			tuser.p_active = TRUE;
			nResult = eReadyResult_Succeed;
			player->set_status(eUserState_ready);
		}
	}

	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_ask_ready_result,robcows_protocols::e_mst_l2c_ask_ready_result);
	sendmsg->set_playerid(playerid);
	sendmsg->set_result(nResult);
	// TODO Bug to fix : can't send msg
//	int ret = broadcast_msg_to_client(sendmsg);
	// TODO send msg ok
//	m_room->broadcast_msg_to_client(sendmsg);

	broadcast_msg_to_client2(sendmsg);
	

	return true;
}

void robcows_space::logic_table::initGame(bool bResetAll/*=false*/)
{
	set_status(eGameState_Free);

	// 初始化游戏参数
	m_nBankerID = 0;

	m_nCurOperatorID = 0;

	m_nGameRound = 0;

	m_BankerRate = 0;
}


void robcows_space::logic_table::repositGame()
{
	SLOG_CRITICAL<<"------------RepositGame------------";
	m_clean_elapsed = 0;

	// 游戏数据初始化
	m_nBankerID = 0;
	m_BankerRate = 0;

	m_nCurOperatorID = 0;
	m_nFirstID = 0;

	m_nGameRound = 0;

	m_MapCompareNexus.clear();

	m_VecActive.clear();
	m_VecPlaying.clear();
	m_VecRecord.clear();
	// 重置数据
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		user.p_active = FALSE;
		user.p_playing = FALSE;
		
		user.p_bankrate = -1;
		user.p_betrate = 0;
		user.p_opencard = false;

		user.p_result = 0;

		player->set_status(eUserState_free);
		if (user.p_norob == true && user.p_nobet == true && user.p_noopen == true)
		{
			user.p_noaction += 1;
		}
		else
		{
			user.p_noaction = 0;
		}
		// 清理机器人
// 		if(player->is_robot())
// 		{
// 			int32_t nRound = global_random::instance().rand_int(m_robotcfg.mRobotMinRound, m_robotcfg.mRobotMaxRound);
// 			if(player->get_gold()<m_room->get_data()->mGoldMinCondition || 
// 				player->get_round()>=nRound)
// 			{
// 				release_robot(player->get_pid());
// 			}
// 		}
	}

	// 清理逃跑用户
 	CleanRunaway();
// 
// 	CleanDisconnect();
// 	//
// 	OnDealEnterBackground();

}

bool robcows_space::logic_table::onGameNoticeSpare()
{
	SLOG_CRITICAL<<"------------onGameNoticeSpare------------";
#ifdef UsingCling
	joinnextround();
#endif // UsingCling

	set_status(eGameState_Spare);
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_notice_sparetime, robcows_protocols::e_mst_l2c_notice_sparetime);
	sendmsg->set_sparetime(m_StartTime);
	broadcast_msg_to_client2(sendmsg);
	m_duration = m_StartTime /*+ TIME_LESS2*/;
	m_clean_elapsed = 0;
	return true;
}

bool robcows_space::logic_table::onGameNoticeRobBanker()
{
	SLOG_CRITICAL << "------------onGameNoticeRobBanker------------";
	std::srand(unsigned(std::time(0)));
	m_VecActive.clear();
	m_VecPlaying.clear();
	m_VecRecord.clear();
	m_GameCardMgr.Reposition();
#ifdef UsingLog
	mGameLog.Clear();
	// gameinfo
	auto gameinfo = mGameLog.mutable_ginfo();
	gameinfo->set_gameid(game_engine::instance().get_gameid());
	gameinfo->set_roomid(m_room->get_id());
	gameinfo->set_tableid(get_id());
	mGameLog.set_begintime(time_helper::instance().get_cur_time());
#endif // UsingLog
	// 参数
	m_BankerRate = 0;
	m_base = m_room->get_data()->mBaseCondition;
	m_VecRobRate = m_room->get_data()->mRobBankerList;
	m_VecBetRate = m_room->get_data()->mBetList;
	// check play and active
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto playerid = user.p_id;
		if (player == nullptr) continue;
		auto leave = user.p_leave;
		if (leave == true) continue;
		// @ 已经请求离开的用户不加入对局
		if (player->get_status() == eUserState_null) continue;
		// add all user
		user.p_active = TRUE;
		// 
		if (user.p_active == TRUE)
		{
			// 排除金币不足的玩家
			if (player->get_gold() < m_room->get_data()->mGoldMinCondition)
			{
				user.p_active = FALSE;
				user.p_playing = FALSE;

				player->set_status(eUserState_Wait);
				player->add_wait();
			}
			else
			{
				m_VecActive.push_back(playerid);
				m_VecPlaying.push_back(playerid);

				user.p_playing = TRUE;

				user.p_bankrate = -1;
				user.p_betrate = 0;
				user.p_opencard = false;
				user.p_norob = false;
				user.p_nobet = false;
				user.p_noopen = false;
				user.p_result = 0;

				player->set_status(eUserState_play);
				player->add_round();
				player->set_wait(0);
				// card mgr
				tagPlayerCtrlAttr attr;
				bool bRet = player->get_player_ctrl(attr);
				int32_t nTag = Tag_UserB;
				if (bRet)
				{
					nTag = attr.nTag;
				}
				user.p_tag = nTag;
				user.p_percent = attr.fwinPercent;
				m_GameCardMgr.SetUserLabel(playerid, nTag);
				SLOG_CRITICAL << boost::format("onGameNoticeRobBanker:[playerid:%d,Tag:%d,Win:%f]") % playerid%attr.nTag%attr.fwinPercent;
				m_GameCardMgr.SetUserPos(playerid, user.p_idx);
#ifdef UsingLog
				// log:记录参与本局游戏
				auto plog = mGameLog.add_pinfo();
				plog->set_pid(playerid);
				plog->set_goldbegin(player->get_gold());
				plog->set_seatid(user.p_idx);
				plog->set_isrobot(player->is_robot());
#endif // UsingLog
			}
		}
		else
		{
			user.p_playing = FALSE;

			player->set_status(eUserState_Wait);
			player->add_wait();
		}
	}
	// 洗牌
//	m_MapPlayerCard.clear();
// 	m_GameCardMgr.Wash_Card3();
// 	SysRegulatoryPolicy();

//	PreSysRegulatoryPolicy();
	//
	m_nGameRound++;
	// 
	// deal msg
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_notice_robbanker, robcows_protocols::e_mst_l2c_notice_robbanker);
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{ // 参与游戏用户
		auto &user = it->second;
		auto player = user.p_playerptr;
		if (player == nullptr) continue;
		uint32_t playerid = user.p_id;
		sendmsg->add_state(user.p_active);
		sendmsg->add_stateid(user.p_id);
		// card info
		// 取牌
// 		GameCard card;
// 		memset(&card, 0, sizeof(card));
// 		auto findcard = m_MapPlayerCard.find(playerid);
// 		if (findcard != m_MapPlayerCard.end())
// 		{
// 			card = findcard->second;
// 		}
// 		else
// 		{
// 			SLOG_CRITICAL << boost::format("%1%:%2% playerid:%3%,Waiting") % __FUNCTION__%__LINE__%playerid;
// 		}
	}
	broadcast_msg_to_client2(sendmsg);

	m_duration = m_BankerTime + TIME_LESS2;
	m_VecRecord.clear();
	set_status(eGameState_Banker);
	m_clean_elapsed = 0;
	return true;
}

bool logic_table::onEventGameStart()
{
	SLOG_CRITICAL<<"------------GameStart------------";
	std::srand(unsigned(std::time(0)));
	m_VecActive.clear();
	m_VecPlaying.clear();
	m_VecRecord.clear();
	m_GameCardMgr.Reposition();
	// 参数
	m_BankerRate = 0;
	m_base = m_room->get_data()->mBaseCondition;
	m_VecRobRate = m_room->get_data()->mRobBankerList;
	m_VecBetRate = m_room->get_data()->mBetList;
	// check play and active
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto playerid = user.p_id;
		if(player==nullptr) continue;
		auto leave = user.p_leave;
		if(leave==true) continue;
		auto runaway = user.p_runaway;
		if(runaway) continue;
		// add all user
		user.p_active = TRUE;
		// 
		if(user.p_active==TRUE)
		{
			// 排除金币不足的玩家
			if(player->get_gold() < m_room->get_data()->mGoldMinCondition)
			{
				user.p_active = FALSE;
				user.p_playing = FALSE;

				player->set_status(eUserState_Wait);
				player->add_wait();
			}
			else
			{
				m_VecActive.push_back(playerid);
				m_VecPlaying.push_back(playerid);

				user.p_playing = TRUE;

				user.p_bankrate = -1;
				user.p_betrate = 0;
				user.p_opencard = false;
				user.p_norob = false;
				user.p_nobet = false;
				user.p_noopen = false;
				user.p_result = 0;

				player->set_status(eUserState_play);
				player->add_round();
				player->set_wait(0);
				// card mgr
				tagPlayerCtrlAttr attr;
				bool bRet = player->get_player_ctrl(attr);
				int32_t nTag = Tag_UserA;
				if(bRet)
				{
					nTag = attr.nTag;
				}
				user.p_tag = nTag;
				user.p_percent = attr.fwinPercent;
				m_GameCardMgr.SetUserLabel(playerid,nTag);
				SLOG_CRITICAL<<boost::format("onEventGameStart:[playerid:%d,Tag:%d,Win:%f]")%playerid%attr.nTag%attr.fwinPercent;
				m_GameCardMgr.SetUserPos(playerid, user.p_idx);

			}
		}
		else
		{
			user.p_playing = FALSE;

			player->set_status(eUserState_Wait);
			player->add_wait();
		}
	}
	// 洗牌
	m_GameCardMgr.Wash_Card3();
	m_MapPlayerCard.clear();
	SysRegulatoryPolicy();
	//
	m_nGameRound++;
	// deal msg
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{ // 参与游戏用户
		auto &user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_game_start,robcows_protocols::e_mst_l2c_game_start);
		sendmsg->set_basegold(m_base);
		// state
		for (auto it2=m_MapTablePlayer.begin();it2!=m_MapTablePlayer.end();it2++)
		{
			auto &user = it2->second;
			sendmsg->add_state(user.p_active);
			sendmsg->add_stateid(user.p_id);
		}
		// card info
		// 取牌

		uint32_t playerid = player->get_pid();
		GameCard card;
		memset(&card,0,sizeof(card));
#if 0
		m_GameCardMgr.GetUserCard(playerid,card);
		m_MapPlayerCard.insert(std::make_pair(playerid,card));
#else
		auto findcard = m_MapPlayerCard.find(playerid);
		if (findcard != m_MapPlayerCard.end())
		{
			card = findcard->second;
		}
		else
		{
			m_GameCardMgr.GetUserCard(card);
			m_MapPlayerCard.insert(std::make_pair(playerid, card));
		}
#endif
		auto cardinfo = sendmsg->mutable_cardinfo();

		if(player->is_robot())
		{
			cardinfo->clear_pokercount();
			cardinfo->clear_pokerdata();
			cardinfo->clear_pokertype();
			// Robot
			robot_mgr::instance().recv_packet(player->get_pid(),sendmsg->packet_id(),sendmsg);
		}
		else
		{
			if(user.p_active==FALSE)
			{ // wait
				cardinfo->clear_pokercount();
				cardinfo->clear_pokerdata();
				cardinfo->clear_pokertype();
			}
			else
			{
				int32_t nCount = MAX_CARDS_HAND - 1;
				cardinfo->set_pokercount(nCount);
				cardinfo->mutable_pokerdata()->Reserve(nCount);
				for (int j=0;j<nCount;j++)
				{
					cardinfo->add_pokerdata(card.nCard[j]);
				}
				cardinfo->clear_pokertype();
				
			}
			player->send_msg_to_client(sendmsg);
		}
	}

	set_status(eGameState_FaPai);
	// 计算发牌时间
	float fCardTime = RobCows_BaseInfo::GetSingleton()->GetData("CardTime")->mValue/10.0;
	float fSpaceTime = RobCows_BaseInfo::GetSingleton()->GetData("SpaceTime")->mValue;
	float fPauseTime = RobCows_BaseInfo::GetSingleton()->GetData("PauseTime")->mValue/10.0;
	
	m_duration = fCardTime * m_VecActive.size() +  fPauseTime + fSpaceTime;

	return true;
}

bool logic_table::onNoticeRobBanker()
{
	SLOG_CRITICAL << "------------onNoticeRobBanker------------";
	set_status(eGameState_Banker);
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_notice_robbanker, robcows_protocols::e_mst_l2c_notice_robbanker);
	broadcast_msg_to_client2(sendmsg);
	m_duration = m_BankerTime + TIME_LESS2;
	m_VecRecord.clear();
	return true;
}

bool robcows_space::logic_table::IsValidRate(int32_t nRate)
{
	for(auto rate : m_VecRobRate)
	{
		if(rate == nRate)
		{
			return true;
		}
	}
	return false;
}

bool robcows_space::logic_table::CanRobBanker(uint32_t playerid,int32_t nRate)
{
	if(nRate==0) return true;
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it != m_MapTablePlayer.end())
	{
		auto user =  player_it->second;
		auto player = user.p_playerptr;
		int64_t lGold = player->get_gold();

		SLOG_CRITICAL<<boost::format("%s:%d Gold:%d,base:%d,nRate:%d")%__FUNCTION__%__LINE__%lGold%m_base%nRate;
		if(lGold >= m_base*30*nRate)
		{
			return true;
		}
	}
	return false;
}

void robcows_space::logic_table::defaultRobBanker()
{
	SLOG_ERROR << boost::format("[Info defaultRobBanker before]====>>>>roomid:%1%,tableid:%2%,status:%3%") % m_room->get_id() % this->get_id() % ((int32_t)this->get_status());
	for (auto user : m_MapTablePlayer)
	{
		auto tableuser = user.second;
		if(tableuser.p_active == TRUE && tableuser.p_bankrate == -1)
		{
			auto uid = tableuser.p_id;
			auto save_it = std::find(m_VecRecord.begin(),m_VecRecord.end(),uid);
			if(save_it==m_VecRecord.end())
			{
				SLOG_CRITICAL<<boost::format("defaultRobBanker:playerid:%d")%uid;

				onGameRobBanker(uid,0,true);
			}
		}
	}
	SLOG_ERROR << boost::format("[Info defaultRobBanker after]====>>>>roomid:%1%,tableid:%2%,status:%3%") % m_room->get_id() % this->get_id() % ((int32_t)this->get_status());
}

bool robcows_space::logic_table::onGameRobBanker(uint32_t playerid,int32_t nRate,bool bTimeOver/*=false*/)
{
	SLOG_CRITICAL<<boost::format("%1%:%2% playerid:%3%,rob:%4%")%__FUNCTION__%__LINE__%playerid%nRate;
	// check player
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"onGameRobBanker player err";
		return false;
	}
	auto &user =  player_it->second;
	auto player = user.p_playerptr;
	auto &robrate = user.p_bankrate;
	if(player && player->is_robot())
	{
		SLOG_CRITICAL<<boost::format("%1%:%2% Robot-id:%3%")%__FUNCTION__%__LINE__%playerid;
	}
	if(user.p_active==FALSE)
	{
		SLOG_ERROR<<"onGameRobBanker player-wait can't rob";
		return false;
	}
	// check state
	auto active_it = std::find(m_VecActive.begin(),m_VecActive.end(),playerid);
	if(active_it==m_VecActive.end()) 
	{
		SLOG_ERROR<<"onGameRobBanker player-not-join-game";
		return false;
	}
	// check game state
	int32_t nGameState= get_status();
	if( nGameState != eGameState_Banker)
	{
		SLOG_ERROR<<boost::format("%1%:%2% game state err:%3%")%__FUNCTION__%__LINE__%nGameState;
		return false;
	}
	// 
	if(!IsValidRate(nRate))
	{
		SLOG_ERROR<<"onGameRobBanker invalid rate";
		return false;
	}
	if(robrate != -1)
	{
		SLOG_ERROR<<"onGameRobBanker repeat request 1";
		return false;
	}
	auto save_it = std::find(m_VecRecord.begin(),m_VecRecord.end(),playerid);
	if(save_it!=m_VecRecord.end())
	{
		SLOG_ERROR<<"onGameRobBanker repeat request 2";
		return false;
	}
#if 0
	// check can call banker
	if(!CanRobBanker(playerid,nRate))
	{
		SLOG_ERROR<<"onGameRobBanker player-gold can't rob";
		return false;
	}
#endif
	if(bTimeOver==true)
	{
		user.p_norob = true;
	}
	else
	{
		user.p_norob = false;
	}
#ifdef UsingLog
	auto count = mGameLog.pinfo_size();
	for (int i = 0; i < count; i++)
	{
		auto pinfo = mGameLog.mutable_pinfo(i);
		auto pid = pinfo->pid();
		if (pid == playerid)
		{
			pinfo->set_robrate(nRate);
			break;
		}
	}
#endif // UsingLog

	// set 
	m_VecRecord.push_back(playerid);
	robrate = nRate;
	// send msg
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_robbank_result, robcows_protocols::e_mst_l2c_ask_robbank_result);
	sendmsg->set_playerid(playerid);
	sendmsg->set_robrate(nRate);
	broadcast_msg_to_client2(sendmsg);
	// check 
	maybe_AllRobBanker();

	return true;
}

void robcows_space::logic_table::maybe_AllRobBanker()
{
	int nUserCount = m_VecActive.size();
	int nRecordCount = m_VecRecord.size();
	SLOG_CRITICAL<<boost::format("%1%:%2%,allcount:%3%,robcount:%4%")%__FUNCTION__%__LINE__%nUserCount%nRecordCount;
	if (nUserCount > nRecordCount) return;
#if 0
	/* 随机庄家*/
	// banker
	int32_t nMaxRate = 0;
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		if(user.p_active == FALSE) continue;
		int32_t nRate = user.p_bankrate;
		if(nRate>=0 && nRate>=nMaxRate)
		{
			nMaxRate = nRate;
		}
	}
	SLOG_CRITICAL<<boost::format("%1%:%2%,MaxRate:%3%")%__FUNCTION__%__LINE__%nMaxRate;
	// 庄倍率
	m_nBankerRate = nMaxRate;
	// 
	Vec_UserID findbanker;
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		if(user.p_active == FALSE) continue;
		uint32_t uid = user.p_id;
		int32_t nRate = user.p_bankrate;
		if(nRate==nMaxRate)
		{
			findbanker.push_back(uid);
		}
	}
	assert(findbanker.size()>0);
	
	std::srand(unsigned(std::time(0)));
	std::random_shuffle(findbanker.begin(),findbanker.end());	
	m_nBankerID = findbanker.back();
#else
	/*
	*  机器人概率事件
	*/
	Vec_UserID findbanker;
	m_nBankerID = SysAlloctBanker(findbanker);
	int32_t nMaxRate = m_nBankerRate;

#endif
	SLOG_CRITICAL<<"------------onNoticeBet------------";
	SLOG_CRITICAL<<boost::format("BankerID:%1%,BankerRob:%2%")%m_nBankerID%m_nBankerRate;
	set_status(eGameState_Bet);
	m_clean_elapsed = 0;
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_notice_bet, robcows_protocols::e_mst_l2c_notice_bet);
	sendmsg->set_bankerid(m_nBankerID);
	sendmsg->set_robrate(nMaxRate);
	for(auto fuid: findbanker)
	{
		sendmsg->add_playerid(fuid);
	}
	broadcast_msg_to_client2(sendmsg);
	m_duration = m_BetTime+TIME_LESS2;
	m_VecRecord.clear();
}

uint32_t robcows_space::logic_table::SysAlloctBanker(Vec_UserID &findbanker)
{
	// banker
	uint32_t nBanker = 0;
	int32_t nMaxRate = 0;
	findbanker.clear();
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		if(user.p_active == FALSE) continue;
		int32_t nRate = user.p_bankrate;
		if(nRate>=0 && nRate>=nMaxRate)
		{
			nMaxRate = nRate;
		}
	}
	SLOG_CRITICAL<<boost::format("SysAlloctBanker,MaxRate:%1%")%nMaxRate;
	
	// 
	Vec_UserID v_robot;
	Vec_UserID v_player;
	bool bHaveA=false,bHaveB=false,bHaveC=false;
	SLOG_CRITICAL<<boost::format("SysAlloctBanker,TablePlayer:%1%")%m_MapTablePlayer.size();
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		if(user.p_active == FALSE) continue;
		uint32_t uid = user.p_id;
		int32_t nRate = user.p_bankrate;
		int32_t nTag = user.p_tag;
		if(nRate==nMaxRate)
		{
			findbanker.push_back(uid);
		}
	}
	// 庄倍率
	if(nMaxRate==0)
	{
		nMaxRate = m_VecRobRate[1];
	}
	m_nBankerRate = nMaxRate;
	SLOG_CRITICAL<<boost::format("SysAlloctBanker,BankerRate:%1%")%nMaxRate;

	std::random_device rd;
	std::seed_seq seed2{ rd(),  rd(),  rd(), rd(),  rd(),  rd(),  rd(),  rd() };
	std::mt19937_64 g2(seed2);
	for (int i = 0; i < findbanker.size(); i++)
	{
		std::shuffle(findbanker.begin(), findbanker.end(), g2);
	}
	nBanker = findbanker.back();

	return nBanker;
}

bool robcows_space::logic_table::CanBet(uint32_t playerid,int32_t nRate)
{
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it != m_MapTablePlayer.end())
	{
		auto user =  player_it->second;
		auto player = user.p_playerptr;
		int64_t lGold = player->get_gold();
		if(lGold > m_base*m_BankerRate*nRate)
		{
			return true;
		}
	}
	return false;
}

bool robcows_space::logic_table::IsValidBet(int nRate)
{
	for(auto rate : m_VecBetRate)
	{
		if(rate == nRate)
		{
			return true;
		}
	}
	return false;
}

void robcows_space::logic_table::defaultBetRate()
{
	SLOG_ERROR << boost::format("[Info defaultBetRate before]====>>>>roomid:%1%,tableid:%2%,status:%3%") % m_room->get_id() % this->get_id() % ((int32_t)this->get_status());
	for (auto user : m_MapTablePlayer)
	{
		auto tableuser = user.second;
		auto uid = tableuser.p_id;
		// 过滤庄家
		if(tableuser.p_active == TRUE && uid != m_nBankerID && tableuser.p_betrate == 0)
		{
			auto save_it = std::find(m_VecRecord.begin(),m_VecRecord.end(),uid);
			if(save_it==m_VecRecord.end())
			{
				int32_t nRate = m_VecBetRate[0];

				SLOG_CRITICAL<<boost::format("defaultBetRate:playerid:%d,betrate:%d")%uid%nRate;

				onGameBetRate(uid,nRate,true);
			}
			else
			{
				// @解散
				SLOG_ERROR << boost::format("[Error]====>>>>roomid:%1%,tableid:%2%,status:%3%") % m_room->get_id() % this->get_id() % ((int32_t)this->get_status());
				set_status(eGameState_Free);
				// 桌子复位
				repositGame();
				
			}
		}

	}
	SLOG_ERROR << boost::format("[Info defaultBetRate after]====>>>>roomid:%1%,tableid:%2%,status:%3%") % m_room->get_id() % this->get_id() % ((int32_t)this->get_status());
}

bool robcows_space::logic_table::onGameBetRate(uint32_t playerid,int32_t nRate,bool bTimeOver/*=false*/)
{
	SLOG_CRITICAL<<boost::format("%1%:%2% playerid:%3%,gold:%4%")%__FUNCTION__%__LINE__%playerid%nRate;
	// check player
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"onGameBet player err";
		return false;
	}
	auto &user =  player_it->second;
	auto player = user.p_playerptr;
	auto &betrate = user.p_betrate;
	if(player && player->is_robot())
	{
		SLOG_CRITICAL<<boost::format("%1%:%2% Robot-id:%3%")%__FUNCTION__%__LINE__%playerid;
	}
	if(user.p_active==FALSE)
	{
		SLOG_ERROR<<"onGameBet player-wait can't bet";
		return false;
	}
	// check state
	auto active_it = std::find(m_VecActive.begin(),m_VecActive.end(),playerid);
	if(active_it==m_VecActive.end()) 
	{
		SLOG_ERROR<<"onGameBet player-not-join-game";
		return false;
	}
	// check game state
	int32_t nGameState= get_status();
	if( nGameState != eGameState_Bet)
	{
		SLOG_ERROR<<boost::format("%1%:%2% game state err:%3%")%__FUNCTION__%__LINE__%nGameState;
		return false;
	}
	// 
	if(!IsValidBet(nRate))
	{
		SLOG_ERROR<<"onGameBet invalid rate";
		return false;
	}
	auto save_it = std::find(m_VecRecord.begin(),m_VecRecord.end(),playerid);
	if(save_it!=m_VecRecord.end())
	{
		SLOG_ERROR<<"onGameBet repeat request";
		return false;
	}
#if 0
	// check can call banker
	if(!CanBet(playerid,nRate))
	{
		SLOG_ERROR<<"onGameBet can't bet ";
		return false;
	}
#endif
	if(playerid==m_nBankerID)
	{
		SLOG_ERROR<<"onGameBet banker ";
		return false;
	}
	if(bTimeOver==true)
	{
		user.p_nobet = true;
	}
	else
	{
		user.p_nobet = false;
	}
#ifdef UsingLog
	auto count = mGameLog.pinfo_size();
	for (int i = 0; i < count; i++)
	{
		auto pinfo = mGameLog.mutable_pinfo(i);
		auto pid = pinfo->pid();
		if (pid == playerid)
		{
			pinfo->set_betrate(nRate);
			break;
		}
	}
#endif // UsingLog
	// set 
	m_VecRecord.push_back(playerid);
	betrate = nRate;
	// send msg
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_bet_result, robcows_protocols::e_mst_l2c_ask_bet_result);
	sendmsg->set_playerid(playerid);
	sendmsg->set_betrate(nRate);
	broadcast_msg_to_client2(sendmsg);

	// check
	maybe_AllBetRate();

	return true;
}

void robcows_space::logic_table::maybe_AllBetRate()
{
	SLOG_ERROR << boost::format("[Info maybe_AllBetRate]====>>>>roomid:%1%,tableid:%2%") % m_room->get_id() % this->get_id();
#if 0
	int nUserCount = m_VecActive.size()-1;
	int nRecordCount = m_VecRecord.size();
	SLOG_CRITICAL<<boost::format("%1%:%2%,all:%3%,bet:%4%")%__FUNCTION__%__LINE__%nUserCount%nRecordCount;
	// 过滤庄家
	if ( nUserCount != nRecordCount) return;
#else
	bool bAllBet = true;
	for (auto user : m_MapTablePlayer)
	{
		auto tableuser = user.second;
		auto uid = tableuser.p_id;
		// 过滤庄家
		if (tableuser.p_active == TRUE && uid != m_nBankerID && tableuser.p_betrate == 0)
		{
			bAllBet = false;
			break;
		}
	}
	if (!bAllBet) return;
#endif

	// @取牌
	PreSysRegulatoryPolicy();

	// 第五张牌
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		if(user.p_active == FALSE) continue;
		uint32_t uid = user.p_id;
		
		// get card
		GameCard card;
		memset(&card,0,sizeof(card));
		auto fresult = m_MapPlayerCard.find(uid);
		if (fresult != m_MapPlayerCard.end())
		{
			card = fresult->second;
		}
		// send msg
		auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_fifth_card, robcows_protocols::e_mst_l2c_fifth_card);

		int32_t nCount = MAX_CARDS_HAND;
		auto cardinfo = sendmsg->mutable_cardinfo();
		cardinfo->set_pokercount(nCount);
		cardinfo->mutable_pokerdata()->Reserve(nCount);
		cardinfo->mutable_pokerdata2()->Reserve(nCount);
		for (int j=0;j<nCount;j++)
		{
			cardinfo->add_pokerdata(card.nCard[j]);
			cardinfo->add_pokerdata2(card.nCowsCard[j]);
		}
		cardinfo->set_pokertype(card.nModel);
		
		if (player->is_robot())
		{
			// Robot
			robot_mgr::instance().recv_packet(player->get_pid(), sendmsg->packet_id(), sendmsg);
		}
		else
		{
			player->send_msg_to_client(sendmsg);
		}
		
	}

	SLOG_CRITICAL<<"------------onNoticeOpenCard------------";
// 	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_notice_opencard, robcows_protocols::e_mst_l2c_notice_opencard);
// 	broadcast_msg_to_client2(sendmsg);

	set_status(eGameState_OpenCard);
	m_duration = m_OpenTime + TIME_LESS2;
	m_clean_elapsed = 0;
	m_VecRecord.clear();
}

bool robcows_space::logic_table::onGameOpenCard(uint32_t playerid, bool bTimeOver /*= false*/)
{
	SLOG_CRITICAL<<boost::format("%1%:%2% playerid:%3%")%__FUNCTION__%__LINE__%playerid;
	// check player
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"onGameOpenCard player err";
		return false;
	}
	auto &user =  player_it->second;
	auto player = user.p_playerptr;
	auto &bopencard = user.p_opencard;
	if(player && player->is_robot())
	{
		SLOG_CRITICAL<<boost::format("%1%:%2% Robot-id:%3%")%__FUNCTION__%__LINE__%playerid;
	}
	if(user.p_active==FALSE)
	{
		SLOG_ERROR<<"onGameOpenCard player-wait can't opencard";
		return false;
	}
	// check state
	auto active_it = std::find(m_VecActive.begin(),m_VecActive.end(),playerid);
	if(active_it==m_VecActive.end()) 
	{
		SLOG_ERROR<<"onGameBet player-not-join-game";
		return false;
	}
	// check game state
	int32_t nGameState= get_status();
	if( nGameState != eGameState_OpenCard)
	{
		SLOG_ERROR<<boost::format("%1%:%2% game state err:%3%")%__FUNCTION__%__LINE__%nGameState;
		return false;
	}
	// check open state
	if(bopencard==true)
	{
		SLOG_ERROR<<"onGameOpenCard open-card err";
		return false;
	}
	//
	if (bTimeOver == true)
	{
		user.p_noopen = true;
	}
	else
	{
		user.p_noopen = false;
	}
	// save
	m_VecRecord.push_back(playerid);
	bopencard = true;
	// get card
	GameCard card;
	memset(&card,0,sizeof(card));
	auto fresult = m_MapPlayerCard.find(playerid);
	if (fresult != m_MapPlayerCard.end())
	{
		card = fresult->second;
	}
	// send msg
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_opencard_result, robcows_protocols::e_mst_l2c_ask_opencard_result);
	sendmsg->set_playerid(playerid);
	auto cardinfo = sendmsg->mutable_cardinfo();

	int32_t nCount = MAX_CARDS_HAND;
	cardinfo->set_pokercount(nCount);
	cardinfo->mutable_pokerdata()->Reserve(nCount);
	for (int j=0;j<nCount;j++)
	{
		cardinfo->add_pokerdata(card.nCowsCard[j]);
	}
	cardinfo->set_pokertype(card.nModel);

	broadcast_msg_to_client2(sendmsg);

	// check
	maybe_AllOpenCard();

	return true;
}

void robcows_space::logic_table::defaultOpenCard()
{
	SLOG_ERROR << boost::format("[Info defaultOpenCard before]====>>>>roomid:%1%,tableid:%2%,status:%3%") % m_room->get_id() % this->get_id() % ((int32_t)this->get_status());
	for (auto user : m_MapTablePlayer)
	{
		auto tableuser = user.second;
		auto uid = tableuser.p_id;
		if (tableuser.p_active == TRUE  && tableuser.p_opencard == false)
		{
			auto save_it = std::find(m_VecRecord.begin(), m_VecRecord.end(), uid);
			if (save_it == m_VecRecord.end())
			{
				SLOG_CRITICAL << boost::format("defaultOpenCard:playerid:%d") % uid;
				onGameOpenCard(uid, true);
			}
		}
	}
	SLOG_ERROR << boost::format("[Info defaultOpenCard after]====>>>>roomid:%1%,tableid:%2%,status:%3%") % m_room->get_id() % this->get_id() % ((int32_t)this->get_status());
}

void robcows_space::logic_table::maybe_AllOpenCard()
{
	int nUserCount = m_VecActive.size();
	int nRecordCount = m_VecRecord.size();
	SLOG_CRITICAL<<boost::format("%1%:%2%,allcount:%3%,opencount:%4%")%__FUNCTION__%__LINE__%nUserCount%nRecordCount;
	if (nUserCount > nRecordCount) return;
	// 取消亮牌过程
	// onGameDisplayCard();
	onGameResult();
}


bool robcows_space::logic_table::onGameDisplayCard()
{
	/* 展示牌
	*  闲家从小到大 、庄家最后
	*/
	SLOG_CRITICAL<<"------------onGameDisplayCard------------";
	set_status(eGameState_Display);
	m_duration = m_nDisplayTime*m_VecActive.size() + TIME_LESS2;

	uint32_t playerid = m_nBankerID;
	GameCard bankcard;
	memset(&bankcard,0,sizeof(bankcard));
	std::map<uint32_t,GameCard> tmpPlayerCard = m_MapPlayerCard;
	auto findit = tmpPlayerCard.find(playerid);
	if(findit!=tmpPlayerCard.end())
	{
		bankcard = findit->second;
		tmpPlayerCard.erase(findit);
	}
	typedef std::pair<uint32_t,GameCard> MYPAIR;
	std::vector<MYPAIR> vec_id_card(tmpPlayerCard.begin(),tmpPlayerCard.end());
	std::sort(vec_id_card.begin(),vec_id_card.end(),[](const MYPAIR& lhs, const MYPAIR& rhs) {
		return lhs.second.nIndex < rhs.second.nIndex;
// #if 0
// 		if(lhs.second.nModel > rhs.second.nModel)
// 		{
// 			return false;
// 		}
// 		else if(lhs.second.nModel==rhs.second.nModel)
// 		{
// 			if(lhs.second.nCardIdx[0] > rhs.second.nCardIdx[0])
// 			{
// 				return false;
// 			}
// 			else if (lhs.second.nCardIdx[0] == rhs.second.nCardIdx[0])
// 			{ 
// 				if(lhs.second.nHua > rhs.second.nHua)
// 					return false;
// 			}
// 		}
// 		return true;
// #else
// 		return lhs.second < rhs.second;
// #endif
	});
	// 庄家放在最后
	MYPAIR banker = std::make_pair(m_nBankerID,bankcard);
	vec_id_card.push_back(banker);

	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_showcard, robcows_protocols::e_mst_l2c_show_card);
	
	for (auto pair_data : vec_id_card)
	{
		auto cardinfo = sendmsg->add_cardinfo();
		uint32_t uid = pair_data.first;
		cardinfo->set_playerid(uid);
		GameCard card = pair_data.second;
		int32_t nCount = MAX_CARDS_HAND;
		cardinfo->set_pokercount(nCount);
		cardinfo->mutable_pokerdata()->Reserve(nCount);
		for (int j=0;j<nCount;j++)
		{
			cardinfo->add_pokerdata(card.nCowsCard[j]);
		}
		cardinfo->set_pokertype(card.nModel);
		SLOG_CRITICAL<<boost::format("[CowCard]uid:%d,data:%02x,%02x,%02x,%02x,%02x") \
			%uid	\
			%card.nCowsCard[0]\
			%card.nCowsCard[1]\
			%card.nCowsCard[2]\
			%card.nCowsCard[3]\
			%card.nCowsCard[4];
	}
	broadcast_msg_to_client2(sendmsg);


	// 
	return true;
}

bool robcows_space::logic_table::onGameResult()
{
	/* 结算信息
	* 1、庄家  入
	* 2、庄家  出
	*/
	SLOG_CRITICAL<<"------------onGameResult------------";
	set_status(eGameState_End);
	//m_duration = m_ResultTime + TIME_LESS35;
	m_duration = m_VecActive.size() * 0.5 + m_ResultTime;
	m_clean_elapsed = 0;

	uint32_t nBanker = m_nBankerID;
	int32_t nBankerRate = m_nBankerRate;
	// Get card
	GameCard bcard;
	memset(&bcard,0,sizeof(bcard));
	auto fresult = m_MapPlayerCard.find(nBanker);
	if (fresult != m_MapPlayerCard.end())
	{
		bcard = fresult->second;
	}
	int32_t nCowModel = bcard.nModel;
	int32_t nCowIndex = bcard.nIndex;
	int32_t nCowRate = m_GameCardMgr.GetCardRate(bcard)  ;
	SLOG_CRITICAL<<boost::format("[Expend]banker:%1%,model:%2%,Index:%3%,RobRate:%4%,CowRate:%5%") \
		%nBanker%nCowModel%nCowIndex%nBankerRate%nCowRate;
	// 计算玩家
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		if(user.p_active == FALSE) continue;
		uint32_t uid = user.p_id;
		// 过滤庄
		if(uid==nBanker) continue;
		GameCard card;
		memset(&card,0,sizeof(card));
		auto fresult = m_MapPlayerCard.find(uid);
		if (fresult != m_MapPlayerCard.end())
		{
			card = fresult->second;
		}
		int32_t nModel = card.nModel;
		int32_t nIndex = card.nIndex;
		int32_t nBetRate = user.p_betrate;
		
		int32_t nRate = m_GameCardMgr.GetCardRate(card);
		SLOG_CRITICAL<<boost::format("[Normal-Expend]uid:%1%,model:%2%,Index:%3%,BetRate:%4%,CowRate:%5%") \
			%uid%nModel%nIndex%nBetRate%nRate;
		int64_t nExpend = 0;
		//if(nIndex < nCowIndex)
		if(card < bcard)
		{ // 闲家输
			nExpend = m_base * nBankerRate * nBetRate * nCowRate;
			user.p_result -= nExpend;
		}
		else
		{ // 闲家赢
			nExpend = m_base * nBankerRate * nBetRate * nRate;
			user.p_result += nExpend;
		}
		SLOG_CRITICAL<<boost::format("[Normal-Expend]result:%1%")%user.p_result;
	}

	/*
	* 玩家单局输（或者赢）的金币总量不能超过自身携带的金币总额
	*/
	// 计算闲家真实输赢
	int64_t nSumIncome = 0;
	int64_t nSumExpend = 0;
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		if(user.p_active == FALSE) continue;
		uint32_t uid = user.p_id;
		// 过滤庄
		if(uid==nBanker) continue;
		int64_t nExpend = 0;
		if(user.p_result <= 0)
		{
			nExpend = std::min(player->get_gold(),std::abs(user.p_result));
			user.p_result = -nExpend;
			nSumIncome += nExpend;
		}
		else
		{
			nExpend = std::min(player->get_gold(),std::abs(user.p_result));
			user.p_result = nExpend;
			nSumExpend += nExpend;
		}
		SLOG_CRITICAL<<boost::format("[Reality-Expend]uid:%1%,result:%2%")%uid%user.p_result;
	}
	//
	SLOG_CRITICAL<<boost::format("[Reality-Expend]income:%d,expend:%d")%nSumIncome%nSumExpend;
	// 计算庄家真实输赢
	auto banker_it = m_MapTablePlayer.find(nBanker);
	if(banker_it == m_MapTablePlayer.end()) 
	{
		assert(0);
	}
	auto &userb = banker_it->second;
	auto player = userb.p_playerptr;
	int64_t nBankerGold = player->get_gold();
	// 计算庄家真实赢
	bool bNeedShareLost = false;
	if ( nBankerGold <= nSumIncome)
	{
		bNeedShareLost = true;
	}
	else
	{
		bNeedShareLost = false;
	}
	int64_t nRealIncome = std::min(nBankerGold , nSumIncome);
	SLOG_CRITICAL << boost::format("[Account-Banker]uid:%1%,Income:%2%") % nBanker%nRealIncome;
	bool bNeedShareWin = false;
	userb.p_result += nRealIncome;
	int64_t nProperty = player->get_gold() + nRealIncome;
	if (nProperty < nSumExpend)
	{ // 需要其他玩家分钱
		bNeedShareWin = true;
	}
	else
	{ // 钱足够支付
		bNeedShareWin = false;
	}
	userb.p_result -= std::min(nProperty, nSumExpend);
	SLOG_CRITICAL<<boost::format("[Account-Banker]uid:%1%,result:%2%")%nBanker%userb.p_result;
	// 计算闲家真实赢
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		if(user.p_active == FALSE) continue;
		uint32_t uid = user.p_id;
		// 过滤庄
		if(uid==nBanker) continue;
		int64_t nExpend = 0;
		int64_t & result = user.p_result;
		if(bNeedShareWin)
		{
			if((result>0) && (nSumExpend!=0))
			{
				nExpend = (result*1.0 / nSumExpend)*nProperty;
				result = nExpend;
				
				SLOG_CRITICAL<<boost::format("[Account-ShareWin]uid:%1%,result:%2%")%uid%result;
			}
		}

		if (bNeedShareLost)
		{
			if ((result<=0)  && (nSumIncome!=0))
			{
				nExpend = (result*1.0 / nSumIncome)*nBankerGold;
				result = nExpend;
				
				SLOG_CRITICAL << boost::format("[Account-ShareLost]uid:%1%,result:%2%") % uid%result;
			}
		}
		SLOG_CRITICAL<<boost::format("[Account-Player]uid:%1%,result:%2%")%uid%result;
	}
	
	// send msg
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_game_result, robcows_protocols::e_mst_l2c_game_result);
	sendmsg->set_bankerid(nBanker);
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		if(user.p_active == FALSE) continue;

		uint32_t uid = user.p_id;
		int64_t nExpend = user.p_result;
		// tax
		int32_t nTax = 0;
		int32_t nRate = 0;
		auto p_gamehandler = game_engine::instance().get_handler();
		if(p_gamehandler && m_room)
		{
			nRate = p_gamehandler->GetRoomCommissionRate(m_room->get_id());
		}
		if(nExpend > 0 && nRate > 0)
		{
			nTax = std::ceil(nExpend * nRate / 100.0);
		}
		nExpend -= nTax;
		auto goldinfo = sendmsg->add_goldinfo();
		goldinfo->set_playerid(uid);
		goldinfo->set_expend(nExpend);

 		player->write_property(nExpend,nTax,m_cbKillType);
 		SLOG_CRITICAL<<boost::format("[Save DB] %1%,result:%2%,tax:%3%")%uid%nExpend%nTax;
#ifdef UsingLog
		int64_t selfgold = player->get_gold() /*+ nExpend*/;
		auto count = mGameLog.pinfo_size();
		for (int i = 0; i < count; i++)
		{
			auto pinfo = mGameLog.mutable_pinfo(i);
			auto pid = pinfo->pid();
			if (pid == uid)
			{
				pinfo->set_goldend(selfgold);
				pinfo->set_commission(nTax);
				pinfo->set_vargold(nExpend);
				break;
			}
		}
#endif // UsingLog
		// @新增牌型广播
		GameCard card;
		memset(&card, 0, sizeof(card));
		auto fresult = m_MapPlayerCard.find(uid);
		if (fresult != m_MapPlayerCard.end())
		{
			card = fresult->second;
		}
		player->Broadcast2RobCows(nExpend, card.nModel);
	}

	broadcast_msg_to_client2(sendmsg);

#ifdef UsingLog
	// check is all robot
	bool bAllRobot = true;
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto user = it->second;
		LPlayerPtr& player = user.p_playerptr;
		if (player == nullptr) continue;
		if (user.p_active == TRUE && player->is_robot() == false)
		{
			bAllRobot = false;
			break;
		}
	}
	if (bAllRobot) return true;
	mGameLog.set_killtype(m_cbKillType);
	mGameLog.set_endtime(time_helper::instance().get_cur_time());
	std::string strLog;
	mGameLog.SerializeToString(&strLog);
	game_engine::instance().get_handler()->sendLogInfo(strLog, GAME_ID);
	SLOG_CRITICAL << mGameLog.DebugString();
#endif // UsingLog
	return true;
}

void robcows_space::logic_table::Switch2Robot(int nRandom/*=100*/)
{
	for (auto user : m_MapTablePlayer)
	{
		auto player = user.second.p_playerptr;
		auto active = user.second.p_active;
		auto uid = user.second.p_id;
		if(player && active==TRUE && player->is_robot())
		{
			robot_switch(uid,nRandom);
		}
	}
}


void robcows_space::logic_table::SysSwitchCard()
{
	SLOG_CRITICAL<<boost::format("------------SysSwitchCard------------");
	uint32_t nBanker = m_nBankerID;
	int32_t nLabel = GetPlayerLabel(nBanker);
	switch (nLabel)
	{
	case Label_UserC:
		{
			double fRandom = global_random::instance().rand_double(1,100);
			// 放大：赢
			Switch2Robot(fRandom);
		}
		break;
	case Label_UserB:
		{
			Switch2Robot(55);
		}
		break;
	case Label_UserA:
		{
			Switch2Robot(30);
		}
		break;
	case Label_Robot:
		{
			int32_t nCountA = 0;
			int32_t nCountB = 0;
			for (auto user : m_MapTablePlayer)
			{
				auto player = user.second.p_playerptr;
				auto active = user.second.p_active;
				auto uid = user.second.p_id;
				if(player && active==TRUE)
				{
					int32_t nLabel = GetPlayerLabel(uid);
					if(nLabel==Label_UserA)
						nCountA++;
					else if(nLabel==Label_UserB)
						nCountB++;
				}
			}
			if(nCountA-nCountB>0)
			{
				robot_switch(nBanker,35);
			}
			else
			{
				robot_switch(nBanker,50);
			}
		}
		break;
	case Label_Supper:
		break;
	default:
		break;
	}
}

void robcows_space::logic_table::SysSwitchCardEx()
{
	SLOG_CRITICAL<<boost::format("------------SysSwitchCardEx------------");
	uint32_t nBanker = m_nBankerID;
	auto player_banker = m_MapTablePlayer.find(nBanker);
	if(player_banker==m_MapTablePlayer.end()) return;

	int32_t nGamerCount = GetJoinCount();
	int32_t nRobotCount = robot_counter();
	SLOG_CRITICAL<<boost::format("GameCounter:%d,RobotCount:%d")%nGamerCount%nRobotCount;
	for (auto user : m_MapTablePlayer)
	{
		auto player = user.second.p_playerptr;
		auto active = user.second.p_active;
		auto uid = user.second.p_id;
		if(!player) continue;
		if(active==FALSE) continue;
		if(uid==nBanker) continue;

		reverse_resultEx(nBanker,uid);
	}

}

uint32_t robcows_space::logic_table::GetNextPlayerByID(uint32_t playerid)
{
	SLOG_CRITICAL<<"GetNextPlayerByID playerid:"<<playerid;
	uint32_t nextid = 0;
	int32_t seat = get_seat_byid(playerid);
	int32_t nextseat = INVALID_CHAIR;
	int32_t fseat = seat;
	do 
	{
		if(nextseat == seat) break;
		nextseat = (fseat-1+GAME_PLAYER)%GAME_PLAYER;
		auto itfind = std::find(m_VecChairMgr.begin(),m_VecChairMgr.end(),nextseat);
		if(itfind==m_VecChairMgr.end())
		{
			uint32_t findid = get_id_byseat(nextseat);
			auto it2 = std::find(m_VecPlaying.begin(),m_VecPlaying.end(),findid);
			if(it2!=m_VecPlaying.end()) break;
		}
		fseat = nextseat;
	} while (TRUE);

	nextid = get_id_byseat(nextseat);
	SLOG_CRITICAL<<"GetNextPlayerByID nextid:"<<nextid;
	return nextid;
}


void robcows_space::logic_table::CleanOutPlayer()
{
	for(auto user : m_MapTablePlayer)
	{
		auto player = user.second.p_playerptr;
		uint32_t playerid = user.second.p_id;
		int32_t noaction = user.second.p_noaction;
		
		if(player && !player->is_robot())
		{// e_msg_cleanout_def
			int64_t lGold = player->get_gold();
			// wait
			int32_t nReason = 0;
			if(player->get_wait()>=3)
			{
				nReason = 3;
			}
			else if(noaction >= 2)
			{
				nReason = 4;
			}
			else if(lGold<m_room->get_data()->mGoldMinCondition)
			{
				nReason = 5;
			}

			if(nReason > 0)
			{
				auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_clean_out, robcows_protocols::e_mst_l2c_clean_out);
				sendmsg->set_reason((robcows_protocols::e_msg_cleanout_def)nReason);
				sendmsg->set_sync_gold(lGold);
				player->send_msg_to_client(sendmsg);

				player->reqPlayer_leaveGame();
			}
			SLOG_WARNING << boost::format("CleanOutPlayer:%1%,noaction:%2%")%playerid%noaction;
		}
	}
}

void robcows_space::logic_table::CleanNoAction(uint32_t playerid)
{
	auto it = m_MapTablePlayer.find(playerid);
	if(it!=m_MapTablePlayer.end())
	{
		auto & user = it->second;
		user.p_noaction = 0;
	}
}
void robcows_space::logic_table::onGameCleanOut(uint32_t playerid,int32_t nReason)
{
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_clean_out, robcows_protocols::e_mst_l2c_clean_out);
	sendmsg->set_reason((robcows_protocols::e_msg_cleanout_def)nReason);
	broadcast_msg_to_client2(sendmsg);
}

bool robcows_space::logic_table::checkGameStart()
{
	int nReadyCount = GetActiveUserCount();
	if(nReadyCount>=PLAY_MIN_COUNT)
	{
		return true;
	}
	return false;
}

bool robcows_space::logic_table::checkStartSpare()
{
	int nPlayerCount = GetAllUserCount();
	int nReadyCount = GetActiveUserCount();

	if(nReadyCount>=PLAY_MIN_COUNT && nPlayerCount>nReadyCount)
	{
		return true;
	}
	return false;
}

bool robcows_space::logic_table::checkOutofRange()
{


	return true;
}

bool robcows_space::logic_table::checkOutofBound(uint32_t playerid)
{
	return true;
}

uint32_t robcows_space::logic_table::CompareWinner(uint32_t reqid,uint32_t resid)
{
	SLOG_CRITICAL<<boost::format("%1%:%2%")%__FUNCTION__%__LINE__;
	// 比牌之前
	reverse_result(reqid,resid);
	//
	GameCard reqcard,rescard;
	memset(&reqcard,0,sizeof(reqcard));
	memset(&rescard,0,sizeof(rescard));
	auto fresult = m_MapPlayerCard.find(reqid);
	if (fresult != m_MapPlayerCard.end())
	{
		reqcard = fresult->second;
	}
	auto fresult2 = m_MapPlayerCard.find(resid);
	if (fresult2 != m_MapPlayerCard.end())
	{
		rescard = fresult2->second;
	}

	if(reqcard.nIndex > rescard.nIndex)
		return reqid;
	else
		return resid;
}

uint32_t robcows_space::logic_table::CompareWinner()
{
	SLOG_CRITICAL<<boost::format("%1%:%2%")%__FUNCTION__%__LINE__;
	// 比牌之前
	reverse_result();

	int32_t nMaxIndx = 0;
	uint32_t nMaxPlayerid = 0;
	for (int i = 0; i < m_VecPlaying.size(); i++)
	{
		uint32_t uid = m_VecPlaying[i];
		auto it = m_MapPlayerCard.find(uid);
		if(it==m_MapPlayerCard.end())
		{
			assert(0);
		}
		int32_t nCardIndx = it->second.nIndex;
		if(nCardIndx > nMaxIndx)
		{
			nMaxIndx = nCardIndx;
			nMaxPlayerid = uid;
		}
	}
	return nMaxPlayerid;
}


bool robcows_space::logic_table::isNewRound()
{
	SLOG_CRITICAL<<boost::format("isNewRound cur:%1%,round:%2%")%m_nCurOperatorID%m_nGameRound;
	if(m_nFirstID==m_nCurOperatorID)
	{
		m_nGameRound++;
		return true;
	}
	return false;
}

bool robcows_space::logic_table::isVaildGold(int64_t lGold)
{
	for (std::vector<int>::iterator it=m_VecAddJetton.begin();it!=m_VecAddJetton.end();it++)
	{
		if (lGold==(*it))
		{
			return true;
		}
	}
	return false;
}

int64_t robcows_space::logic_table::GetLessGold(uint32_t& lessID)
{
	lessID = 0;
	int64_t lLessGold = 0;
	Vector_pair vecpair ;
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto user = it->second;
		if(user.p_playing==TRUE)
		{
			PAIR pairs(user.p_id,user.p_asset);
			vecpair.push_back(pairs);
		}
	}
	std::sort(vecpair.begin(),vecpair.end(),[](const PAIR& lhs, const PAIR& rhs) {
		return lhs.second > rhs.second;
	});
	if(vecpair.size()==0)
	{
		assert(0);
	}
	PAIR key_value = vecpair.back();
	lessID = key_value.first;
	lLessGold = key_value.second;
	return lLessGold;
}

void robcows_space::logic_table::SetCompareNexus(uint32_t playerid,uint32_t nexusid)
{
	// except self 
	if(playerid==nexusid) return;
	auto it = m_MapCompareNexus.find(playerid);
	if(it==m_MapCompareNexus.end())
	{
		Vec_UserID vecnexus;
		vecnexus.push_back(nexusid);
		m_MapCompareNexus.insert(std::make_pair(playerid,vecnexus));
	}
	else
	{
		Vec_UserID & vecnexus = it->second;
		vecnexus.push_back(nexusid);
	}

	auto nexusit = m_MapCompareNexus.find(nexusid);
	if(nexusit==m_MapCompareNexus.end())
	{
		Vec_UserID vecnexus;
		vecnexus.push_back(playerid);
		m_MapCompareNexus.insert(std::make_pair(nexusid,vecnexus));
	}
	else
	{
		Vec_UserID & vecnexus = nexusit->second;
		vecnexus.push_back(playerid);
	}
}

void robcows_space::logic_table::SetCompareNexus(Vec_UserID &playerids)
{
	for (auto outring_it=playerids.begin();outring_it != playerids.end();outring_it++)
	{
		uint32_t outring_uid = (*outring_it);
		for (auto inring_it=playerids.begin();inring_it != playerids.end();inring_it++)
		{
			SetCompareNexus(outring_uid,(*inring_it));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
void robcows_space::logic_table::robot_heartbeat(double elapsed)
{
#if 0
	if (m_robotcfg.mIsOpen == 0) return;
#else
	if (m_room == nullptr) return;
	if (m_room->getRobotSwitch() == 2) return;
#endif
	
	int minCount = m_robotcfg.mRobotMinCount;
	int maxCount = m_robotcfg.mRobotMaxCount;
	int requireCount = global_random::instance().rand_int(minCount, maxCount);

	m_robot_elapsed -= elapsed;
	if (m_robot_elapsed <= 0)
	{
		m_robot_elapsed = global_random::instance().rand_int(m_robotcfg.mRobotMinEntry, m_robotcfg.mRobotMaxEntry);
		
		int player_count = 0;
		int robot_count = 0;
		for (auto user : m_MapTablePlayer)
		{
			LPlayerPtr& player = user.second.p_playerptr;
			if(player!=nullptr)
			{
				if(player->is_robot())
					robot_count++;
				else
					player_count++;
			}
		}
		int all_count = player_count + robot_count;
		if(all_count==GAME_PLAYER) return;
		// 只剩机器人了
		if(player_count==0 && robot_count>0)
		{ // @只剩下一个机器人清理掉
#if 0
			for (auto user : m_MapTablePlayer)
			{
				LPlayerPtr& player = user.second.p_playerptr;
				// 游戏中不离开
				if(player->get_status()==eUserState_play ) 
					continue;
				if(player && player->is_robot())
				{
					release_robot(player->get_pid());
				}
			}
#endif
		}
		else if (all_count<GAME_PLAYER && robot_count < requireCount)
		{
			request_robot();
		}
		else
		{
			// 金币不足
// 			for (auto user : m_MapTablePlayer)
// 			{
// 				LPlayerPtr& player = user.second.p_playerptr;
// 				// 游戏中不离开
// 				if(player->get_status()==eUserState_play) 
// 					continue;
// 				if(player && player->is_robot() &&
// 					player->get_gold()<m_room->get_data()->mGoldMinCondition)
// 				{
// 					release_robot(player->get_pid());
// 				}
// 			}
		}

		// 清理
// 		for (auto user : m_MapTablePlayer)
// 		{
// 			LPlayerPtr& player = user.second.p_playerptr;
// 			// 游戏中不离开
// 			if(player->get_status()==eUserState_play) 
// 				continue;
// 			if(player && player->is_robot())
// 			{
// 				int32_t nRound = global_random::instance().rand_int(m_robotcfg.mRobotMinRound, m_robotcfg.mRobotMaxRound);
// 				if(player->get_round()>=nRound)
// 				{ // 超出限定局数
// 					release_robot(player->get_pid());
// 				}
// 				else if(player->get_gold()>m_robotcfg.mRobotMaxGold)
// 				{ // 超出携带数量
// 					release_robot(player->get_pid());
// 				}
// 				else
// 				{
// 
// 				}
// 			}
// 		}
	}
}

void robcows_space::logic_table::request_robot()
{
	if (get_room()->getservice() == 0)
	{
		SLOG_CRITICAL << boost::format("Service Stop");
		return;
	}
	// Robot
	static int minVIP = m_robotcfg.mRobotMinVip;
	static int maxVIP = m_robotcfg.mRobotMaxVip;
	int32_t vip_level = global_random::instance().rand_int(minVIP, maxVIP);	
	GOLD_TYPE gold_min = m_robotcfg.mRobotMinTake;
	GOLD_TYPE gold_max = m_robotcfg.mRobotMaxTake;

	GOLD_TYPE enter_gold = global_random::instance().rand_int(gold_min,gold_max);
	int32_t rid = m_room->get_id();
	int32_t tid = get_id();
	//int tag = (rid<<4)|tid;
	int tag = rid + tid * 100;
	SLOG_CRITICAL<<boost::format("request_robot::rid:%1% tid:%2%,tag:%3%")%rid%tid%tag;
	game_engine::instance().request_robot(tag,enter_gold, vip_level);

}

void robcows_space::logic_table::release_robot(int32_t playerid)
{
	// 
	SLOG_CRITICAL<<boost::format("release_robot:playerid:%1%")%playerid;
	// setting robot leave
	auto it = m_MapTablePlayer.find(playerid);
	if(it == m_MapTablePlayer.end()) return;
	auto & user = it->second;
	user.p_leave = true;

	bc_leave_seat(playerid);
	m_room->get_lobby()->robot_leave(playerid);
	game_engine::instance().release_robot(playerid);
}

void robcows_space::logic_table::reverse_result(uint32_t reqid,uint32_t resid)
{
	SLOG_CRITICAL<<boost::format("%1%:%2%")%__FUNCTION__%__LINE__;
	int32_t nRobotCount = robot_counter();
	if(nRobotCount==0) return;
	uint32_t robot_req = 0,robot_res = 0;
	robot_req = robot_id(reqid);
	robot_res = robot_id(resid);
	if(robot_req==0 && robot_res==0) return;
	SLOG_CRITICAL<<boost::format("reverse_result:RobotCount:%1%,%2%,%3%")%nRobotCount%robot_req%robot_res;
	int32_t rate = robot_rate();
	int nRandom = global_random::instance().rand_int(1,100);
	if(nRandom <= rate)
	{
		if (nRobotCount>1)
		{// 此时机器是可以输的
			if(robot_req!=0)
			{
				robot_switch(robot_req,global_random::instance().rand_int(1,100));
			}
			if(robot_res!=0)
			{
				robot_switch(robot_res,global_random::instance().rand_int(1,100));
			}
		}
		else if(nRobotCount==1)
		{
			// 此时机器人是不能输的
			if(robot_req!=0)
			{
				robot_switch(robot_req);
			}
			if(robot_res!=0)
			{
				robot_switch(robot_res);
			}
		}
		else
		{
			// 此时无意义
		}
	}
	else
	{
		// 此时机器是可输可赢
// 		if(robot_req!=0)
// 		{
// 			robot_switch(robot_res,global_random::instance().rand_int(1,100));
// 		}
// 		if(robot_res!=0)
// 		{
// 			robot_switch(robot_res,global_random::instance().rand_int(1,100));
// 		}
	}
}


void robcows_space::logic_table::reverse_resultEx(uint32_t reqid,uint32_t resid)
{
	int32_t nRobotCount = robot_counter(reqid,resid);
	SLOG_CRITICAL<<boost::format("%1%:%2%,RobotCount:%3%")%__FUNCTION__%__LINE__%nRobotCount;
	if(nRobotCount==0)
	{
		// 玩家比牌
	}
	if(nRobotCount==1)
	{
		uint32_t playerid = 0,robotid = 0;
		tagPlayerCtrlAttr attr;
		uint32_t robot_req = 0,robot_res = 0;
		robot_req = robot_id(reqid);
		robot_res = robot_id(resid);
		if(robot_req==0 && robot_res==0) return;
		if(robot_req==0)
		{
			playerid = reqid;
			robotid = resid;
		}
		else
		{
			playerid = resid;
			robotid = reqid;
		}
		SLOG_CRITICAL<<boost::format("[Switch]Robot:%d,Player:%d")%robotid%playerid;
		auto it = m_MapTablePlayer.find(playerid);
		if(it == m_MapTablePlayer.end()) return ;
		auto user = it->second;
		auto player = user.p_playerptr;
		player->get_player_ctrl(attr);
		double fRandom = global_random::instance().rand_01();
		double fWinPercent = attr.fwinPercent;
		SLOG_CRITICAL<<boost::format("[Switch]Robot:%d,Player:%d,PlayerWin:%f")%robotid%playerid%fWinPercent;
		SLOG_CRITICAL<<boost::format("Random:%f,playerid:%d,Tag:%d,Win:%f")%fRandom%playerid%attr.nTag%fWinPercent;
		if(fRandom <= attr.fwinPercent)
		{ // 玩家赢

		}
		else
		{// 机器人赢
			robot_switch(robotid);
		}

	}
	else
	{
		// 机器人比牌
	}
}

void robcows_space::logic_table::reverse_result()
{
	SLOG_CRITICAL<<boost::format("%1%:%2%")%__FUNCTION__%__LINE__;
	int32_t nRobotCount = robot_counter();
	if(nRobotCount==0) return;
	SLOG_CRITICAL<<boost::format("reverse_result:RobotCount:%1%")%nRobotCount;
	int32_t rate = robot_rate();
	int nRandom = global_random::instance().rand_int(1,100);
	if (nRandom <= rate)
	{
		for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
		{ // 活着的机器人
			auto user = it->second;
			uint32_t uid = user.p_id;
			auto player = user.p_playerptr;
			if(user.p_playing==FALSE) continue;
			if( player && player->is_robot())
			{
				if(nRobotCount==1)
					robot_switch(uid);
				else
					robot_switch(uid,global_random::instance().rand_int(1,100));
			}	
		}
	}
}

void robcows_space::logic_table::robot_switch(uint32_t uid,int nRandom/*=100*/)
{
	if(uid!=0)
	{
		int nRnd = global_random::instance().rand_int(1,100);
		if(nRnd<=nRandom)
		{
			auto fresult = m_MapPlayerCard.find(uid);
			if (fresult != m_MapPlayerCard.end())
			{
				auto& card = fresult->second;
				if(card.bCheck == false)
				{
					card.bCheck = m_GameCardMgr.SwitchCard(uid,card);

					int32_t cbOk = card.bCheck;
					SLOG_CRITICAL<<boost::format("robot_switch: %d")%cbOk;
				}
			}

		}
	}
}

int32_t robcows_space::logic_table::robot_counter()
{
	int32_t nCount = 0;
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto user = it->second;
		auto player = user.p_playerptr;
		if(user.p_active==FALSE || player==nullptr) continue;
		if(player->is_robot())
		{
			nCount++;
		}
	}
	return nCount;
}

int32_t robcows_space::logic_table::robot_counter(uint32_t reqid,uint32_t resid)
{
	int32_t nCount = 0;
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto user = it->second;
		auto player = user.p_playerptr;
		if(user.p_active==FALSE || player==nullptr) continue;
		auto uid = user.p_id;
		if(player->is_robot())
		{
			if(uid==reqid || uid==resid) nCount++;
		}
	}
	return nCount;
}

uint32_t robcows_space::logic_table::robot_id(uint32_t uid)
{
	uint32_t robotid = 0;
	auto it = m_MapTablePlayer.find(uid);
	if(it == m_MapTablePlayer.end()) return robotid;
	auto player = it->second.p_playerptr;
	if(player && player->is_robot())
	{
		robotid = uid;
	}

	return robotid;
}

int32_t robcows_space::logic_table::robot_rate()
{
	int32_t nRate = 0;
	int32_t nLabel = GetMaxLabel();
	int32_t nRound = m_nGameRound;
	if(nLabel==Label_UserC)
	{
		nRate = 100;
	}
	else if(nLabel==Label_UserB)
	{
		nRate = (80-nRound*4);
	}
	else if(nLabel==Label_UserA)
	{
		nRate = (44-nRound*4);
	}
	else
	{
		nRate = 50;
	}

	return nRate;
}

int32_t robcows_space::logic_table::GetLabelByValue(int64_t nValue)
{
	if(nValue<=Tag_UserA)
	{
		return Label_UserA;
	}
	else if(nValue<=Tag_UserB)
	{
		return Label_UserB;
	}
	else
	{
		return Label_UserC;
	}
}


int32_t robcows_space::logic_table::GetMaxLabel()
{
	for(auto user: m_MapTablePlayer)
	{
		if(user.second.p_label == Label_UserC)
			return Label_UserC;
	}
	for(auto user: m_MapTablePlayer)
	{
		if(user.second.p_label == Label_UserB)
			return Label_UserB;
	}
	for(auto user: m_MapTablePlayer)
	{
		if(user.second.p_label == Label_UserA)
			return Label_UserA;
	}

	return Label_Robot;

}

int32_t robcows_space::logic_table::GetPlayerLabel(uint32_t playerid)
{
	int32_t nLabel = Tag_UserA;
	auto it = m_MapTablePlayer.find(playerid);
	if(it!=m_MapTablePlayer.end())
	{
		auto & user = it->second;
		auto& player = game_engine::instance().get_lobby().get_player(playerid);
		if(player)
		{
			tagPlayerCtrlAttr attr;
			bool bRet = player->get_player_ctrl(attr);
			int32_t nTag = attr.nTag;
			user.p_tag = nTag;
		}
	}
	return nLabel;
}

void robcows_space::logic_table::CleanRunaway()
{
	// 清理逃跑
	Vec_UserID vecclear;
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if ( player == nullptr) continue;
		auto &state = user.p_leave;
		if(state) continue;
		bool bRunaway = user.p_runaway;
		if (bRunaway)
		{
			// @标记
			state = true;
			auto playerid = user.p_id;
			SLOG_WARNING << boost::format("logic_table::clean runaway >>>> :%1%") % playerid;
			// clear table
			player->set_status(eUserState_null);
			player->reqPlayer_leaveGame();
			vecclear.push_back(playerid);
		}
		
	}
	for (auto id : vecclear)
	{
		SLOG_WARNING << boost::format("logic_table::game_engine clean runaway >>>> :%1%") % id;
		// @存在特殊情况用户已经不存在
		m_room->get_lobby()->player_clean_player(id);
		// @清理
		SLOG_CRITICAL << boost::format("%1%:%2%") % __FUNCTION__%__LINE__;
		leave_table(id);
	}
}

void robcows_space::logic_table::CleanDisconnect()
{
	// 清理断线
	for (auto user : m_MapTablePlayer)
	{
		auto player = user.second.p_playerptr;
		if(player == nullptr) continue;
		if(player->get_state() == e_player_state::e_ps_disconnect)
		{
			SLOG_WARNING << boost::format("CleanDisconnect");
			// clear table
			player->reqPlayer_leaveGame();
		}
	}
}


int32_t logic_table::release_robot_seat()
{
	// @只能释放空闲状态下的机器人
	int32_t seat = INVALID_CHAIR;
	SLOG_CRITICAL<<boost::format("%1%:%2%")%__FUNCTION__%__LINE__;
	int32_t nRobotCount = 0;
	struct tagRobotSeat
	{
		uint32_t robotid;
		int32_t  robotseat;

		tagRobotSeat() :robotid(0), robotseat(0){}
	};
	std::vector<tagRobotSeat> ceder;
	for(auto it= m_MapTablePlayer.begin();it!= m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto uid = user.p_id;
		if(player && player->is_robot())
		{
			nRobotCount++;
			bool bRet = player->can_leave_table();
			if(bRet)
			{
				tagRobotSeat rseat;
				rseat.robotid = uid;
				rseat.robotseat = player->get_seat();
				ceder.push_back(rseat);
			}
		}
	}
	//if (nRobotCount <= 1) return seat;
	std::random_device rd;
	std::mt19937_64 g(rd());
	std::shuffle(ceder.begin(), ceder.end(), g);
	if (ceder.size() > 0)
	{
		tagRobotSeat frseat = ceder.back();
		uint32_t ruid = frseat.robotid;
		auto player_it = m_MapTablePlayer.find(ruid);
		if (player_it != m_MapTablePlayer.end())
		{
			auto player = player_it->second.p_playerptr;
			if (player)
			{
				player->leave_table();
				player->set_status(eUserState_null);
				release_robot(ruid);
				seat = frseat.robotseat;
			}
		}
	}
	return seat;
}

bool robcows_space::logic_table::can_release_robot()
{
	bool bRet = false;
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto user = it->second;
		auto player = user.p_playerptr;
		if (player && player->is_robot() && player->can_leave_table())
		{
			bRet = true;
			break;
		}
	}
	return bRet;
}

int32_t logic_table::cling_robotid()
{
	/* @ 依附机器人 
	*  @ 一个位置只能存在一个真实玩家
	*  @ 在本轮游戏结束后才能进入
	*/
	int32_t seat = INVALID_CHAIR;
	uint32_t robotid = 0;
	SLOG_CRITICAL << boost::format("%1%:%2%") % __FUNCTION__%__LINE__;
	Vec_UserID ceder;
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto uid = user.p_id;
		auto idx = user.p_idx;
		if (player && player->is_robot())
		{
			auto findseat = m_MapExtraPlayer.find(idx);
			if (findseat == m_MapExtraPlayer.end())
			{
				ceder.push_back(uid);
			}
		}
	}
	std::random_device rd;
	std::mt19937_64 g(rd());
	std::shuffle(ceder.begin(), ceder.end(), g);
	if (ceder.size() > 0)
	{
		uint32_t ruid = ceder.back();
		auto player_it = m_MapTablePlayer.find(ruid);
		if (player_it != m_MapTablePlayer.end())
		{
			auto& user = player_it->second;
			auto player = user.p_playerptr;
			if (player)
			{
				robotid = ruid;
				seat = user.p_idx;
				// @ 需要清理此机器人
				user.p_kick = true;
			}
		}
	}
	return seat;
}

void logic_table::joinnextround()
{
	// @清理机器人
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		auto &state = user.p_leave;
		if (state == true) continue;
		auto uid = user.p_id;
		auto seat = user.p_idx;
		if (player->is_robot() && user.p_kick)
		{
			SLOG_CRITICAL << boost::format("joinnextround :%1%") % uid;
			release_robot(uid);
			state = true;
			// @椅子归还
			m_VecChairMgr.push_back(seat);
		}
	}
	// @加入对局中
	for (auto it = m_MapExtraPlayer.begin(); it != m_MapExtraPlayer.end(); it++)
	{
		auto table_player = it->second;
		auto seat = table_player.p_idx;
		auto findseat = std::find(m_VecChairMgr.begin(), m_VecChairMgr.end(), seat);
		if (findseat != m_VecChairMgr.end())
		{
			m_MapTablePlayer.insert(std::make_pair(table_player.p_id, table_player));

			inc_dec_count();
			bc_enter_seat(table_player);
			m_VecChairMgr.erase(findseat);
		}
		else
		{
			SLOG_CRITICAL << boost::format("joinnextround: %1% can't find seat:%2%")%table_player.p_id%seat;
		}
	}

	m_MapExtraPlayer.clear();

}

void robcows_space::logic_table::OnEnterBackground(uint32_t playerid)
{
	SLOG_CRITICAL << boost::format("%1% :%2%") % __FUNCTION__%playerid;
	auto player_it = m_MapTablePlayer.find(playerid);
	if (player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR << boost::format("%1%: Can't find this user:%2%") % __FUNCTION__%playerid;
	}
	else
	{
		auto &user = player_it->second;
		user.p_background = true;
	}
}

void robcows_space::logic_table::OnDealEnterBackground()
{
	for (auto user : m_MapTablePlayer)
	{
		auto player = user.second.p_playerptr;
		if (player->is_robot()) continue;
		uint32_t playerid = user.second.p_id;
		bool bbackground = user.second.p_background;
		if (player && bbackground)
		{
			SLOG_CRITICAL << boost::format("logic_table::clean enter background >>>> :%1%") % playerid;
			auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_clean_out, robcows_protocols::e_mst_l2c_clean_out);
			sendmsg->set_sync_gold(player->get_gold());
			sendmsg->set_reason(robcows_protocols::e_msg_cleanout_def::e_cleanout_enter_background);
			player->send_msg_to_client(sendmsg);
			// clear table
			player->reqPlayer_leaveGame();
		}
	}
}

void robcows_space::logic_table::CleanTableEvent()
{
	for (auto &user : m_MapTablePlayer)
	{
		auto& tuser = user.second;
		auto player = tuser.p_playerptr;
		if (player == nullptr) continue;
		auto &state = tuser.p_leave;
		if (state == true) continue;
		uint32_t playerid = tuser.p_id;
		int64_t lGold = player->get_gold();
		if (player->is_robot())
		{
			int32_t nRound = tuser.p_round;
			if (nRound == 0) nRound = m_robotcfg.mRobotMaxRound;
			if (get_room()->getservice() == 0 || get_room()->getRobotSwitch() == 2)
			{// @服务维护或者关闭机器人
				player->set_status(eUserState_null);
				release_robot(playerid);
				state = true;
			}
			else if ( lGold < m_room->get_data()->mGoldMinCondition )
			{// @清理游戏条件不足
				player->set_status(eUserState_null);
				release_robot( playerid);
				state = true;
			}
			else if (player->get_round() >= nRound || lGold >= m_robotcfg.mRobotMaxGold)
			{
				SLOG_CRITICAL << boost::format("CleanTableEvent :%1%") % nRound;
				player->set_status(eUserState_null);
				release_robot(playerid);
				state = true;
			}

		}
		else
		{
			int reason = 0;
			int32_t noaction = tuser.p_noaction;
			bool bbackground = tuser.p_background;
			if (get_room()->getservice() == 0)
			{// @服务维护
				reason = 8;
				state = true;
			}
			else if (tuser.p_kick == true)
			{// @被踢事件
				reason = 7;
				tuser.p_kick = false;
				state = true;
			}
			else if (player->get_state() == e_player_state::e_ps_disconnect)
			{// @断线用户
				player->set_status(eUserState_null);
				player->reqPlayer_leaveGame();
				state = true;
			}
			else if (player->get_wait() >= 3)
			{// @游戏条件不足
				reason = 3;
				state = true;
			}
			else if (noaction >= 1)
			{// @游戏条件不足
				reason = 4;
				state = true;
			}
			else if (lGold < m_room->get_data()->mGoldMinCondition)
			{// @游戏条件不足
				reason = 5;
				state = true;
			}
			else if (bbackground)
			{// @清理后台用户
				reason = 6;
				state = true;
			}
			else if (player->get_serverflag() == 1)
			{// @其他事件
				player->set_status(eUserState_null);
				player->reqPlayer_leaveGame(3);
				state = true;
			}

			if ( reason > 0)
			{
				auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_clean_out, robcows_protocols::e_mst_l2c_clean_out);
				sendmsg->set_reason((robcows_protocols::e_msg_cleanout_def)reason);
				sendmsg->set_sync_gold(lGold);
				player->send_msg_to_client(sendmsg);
				player->set_status(eUserState_null);
				player->reqPlayer_leaveGame();
			}
		}
	}
}

void logic_table::CleanKickedPlayer()
{
	// 游戏结束踢出玩家
	for (auto &user : m_MapTablePlayer)
	{
		auto& tuser = user.second;
		auto player = tuser.p_playerptr;
		if (player == nullptr) continue;
		if (player->is_robot()) continue;
		auto &state = tuser.p_leave;
		if (state == true) continue;
		uint32_t playerid = user.second.p_id;
		int reason = 0;
		if (get_room()->getservice() == 0)
		{
			// @ 踢出用户：系统维护
			reason = 8;
			state = true;
		}
		else if (tuser.p_kick == true)
		{
			/*
			* @ 踢出用户 离开World服务
			*/
			reason = 7;
			tuser.p_kick = false;
			state = true;
		}
		else if (player->get_serverflag() == 1)
		{
			state = true;
			player->reqPlayer_leaveGame(3);
		}
		if (reason == 7 || reason == 8)
		{
			auto sendmsg = PACKET_CREATE(robcows_protocols::packetl2c_clean_out, robcows_protocols::e_mst_l2c_clean_out);
			sendmsg->set_reason((robcows_protocols::e_msg_cleanout_def)reason);
			sendmsg->set_sync_gold(player->get_gold());
			player->send_msg_to_client(sendmsg);

			player->reqPlayer_leaveGame();
		}
	}
}

void robcows_space::logic_table::CleanRobot()
{
	for (auto &user : m_MapTablePlayer)
	{
		auto& tuser = user.second;
		auto player = tuser.p_playerptr;
		if (player == nullptr) continue;
		auto &state = tuser.p_leave;
		if (state == true) continue;
		uint32_t playerid = user.second.p_id;
		int64_t lGold = player->get_gold();
		if (player &&  player->is_robot())
		{
			int32_t nRound = global_random::instance().rand_int(m_robotcfg.mRobotMinRound, m_robotcfg.mRobotMaxRound);
			if(player->get_round() >= nRound || lGold >= m_robotcfg.mRobotMaxGold)
			{
				release_robot(playerid);
				state = true;
				break;
			}
		}
	}
}

void logic_table::SysRegulatoryPolicy()
{
	std::map<uint32_t, int32_t> mapRobotPercent;
	std::map<uint32_t, int32_t> mapPercent;
	int32_t nRobotTotal = 0;
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto playerid = user.p_id;
		if (player == nullptr) continue;
		if (user.p_active == FALSE) continue;
		int32_t percent = user.p_percent * 10000;
		if (get_room()->getkiller() == 10)
		{
			if (user.p_tag == Tag_UserC)
			{
				percent = user.p_percent * 10000;
			}
			else
			{
				percent = get_room()->getCutRound();
			}
		}
		else
		{
			percent = user.p_percent * 10000;
		}
		//////////////////////////////////////////////////////////////////////////
		if (player->is_robot())
		{
			mapRobotPercent[playerid] = percent;
		}
		else
		{
			nRobotTotal += 10000 - percent;
			mapPercent[playerid] = percent;
		}
	}
	Vec_UserID vecfind;
	vecfind.clear();
	int32_t nRobotCount = mapRobotPercent.size(), nUserCount = mapPercent.size();
	if (nRobotTotal == 0 && nUserCount == 0)
	{
		// 全部是机器人随机发牌
		for (auto rpercent : mapRobotPercent)
		{
			vecfind.push_back(rpercent.first);
		}
	}
	else
	{
		// 计算机器人概率
		int32_t nRobotPercent = nRobotTotal / nUserCount;
		for (auto &robot : mapRobotPercent)
		{
			robot.second = nRobotPercent;
		}
		//	mapPercent.insert(mapRobotPercent.begin(),mapRobotPercent.end());
		std::copy(mapRobotPercent.begin(), mapRobotPercent.end(), std::inserter(mapPercent, mapPercent.end()));
		// 排序
		typedef std::pair<uint32_t, int32_t> PercentPAIR;
		std::vector<PercentPAIR> vec_id_percent(mapPercent.begin(), mapPercent.end());
		std::sort(vec_id_percent.begin(), vec_id_percent.end(), [](const PercentPAIR& lhs, const PercentPAIR& rhs) {
			return lhs.second < rhs.second; });
		// 
		std::random_device rd;
		std::mt19937_64 g(rd());
		for (int i = 0; i < nUserCount + nRobotCount; i++)
		{
			int32_t nTotalPercent = 0;
			for (auto ppercent : vec_id_percent)
			{
				nTotalPercent += ppercent.second;
			}

			std::uniform_int_distribution<> dis(0, nTotalPercent);
			int32_t nRandom = dis(g);
			int32_t nSumRate = 0;
			for (auto it = vec_id_percent.begin(); it != vec_id_percent.end(); it++)
			{
				nSumRate += it->second;
				if (nRandom <= nSumRate)
				{
					vecfind.push_back(it->first);
					vec_id_percent.erase(it);
					break;
				}
				else
				{
					// 
				}
			}
		}
	}
	// 取牌
	for (auto f_id : vecfind)
	{
		GameCard card;
		memset(&card, 0, sizeof(card));
		m_GameCardMgr.GetUserCard(card);
		std::random_device rd;
		std::mt19937_64 g(rd());
		std::shuffle(card.nCard.begin(), card.nCard.end(),g);
		m_MapPlayerCard.insert(std::make_pair(f_id, card));
	}
} 

void robcows_space::logic_table::SysKillerPolicy()
{
	std::random_device rd;
	std::mt19937_64 g(rd());
	// @ 0：default 1：kill 2: donate
	int killopt = get_room()->getkiller();
	int cutround = get_room()->getCutRound();
	cutround = std::abs(cutround);
	if (cutround == 0)
	{
		killopt = 11;
	}
	// 
	if (killopt == 10)
	{ // start
		if (m_cbGameRound == cutround)
		{
			m_cbKillRound = 0;
			m_cbGameRound = 0;
		}
		// kill point
		if (m_cbKillRound == 0)
		{
			std::uniform_int_distribution<> dis(1, cutround);
			m_cbKillRound = dis(g);
		}
		m_cbGameRound++;
	}
	else if (killopt == 11)
	{ // stop
		m_cbKillRound = 0;
	}
}

void robcows_space::logic_table::PreSysRegulatoryPolicy()
{
	std::random_device rd;
	std::mt19937_64 g(rd());

	SysKillerPolicy();

	// @ 0：default 1：kill 2: donate
	int32_t cbKiller = 0;
	int killopt = get_room()->getkiller();
	int cutround = get_room()->getCutRound();
#if 0
	m_cbKillType = 0;
	if (killopt == 10)
	{ // start
		std::uniform_int_distribution<> dis(1, 10000);
		auto disRet = dis(g);
		if (cutround > 0)
		{
			if (disRet <= cutround)
			{
				// kill
				cbKiller = 1;
			}
		}
		else if(cutround < 0)
		{
			if (disRet <= std::abs(cutround))
			{
				// donate
				cbKiller = 2;
			}
		}
		else
		{
			cbKiller = 0;
		}
		SLOG_CRITICAL << boost::format("%1%:%2%:[Kill]disRet:%3% ,cutround:%4% ") % __FUNCTION__%__LINE__%disRet%cutround;
	}
	else if (killopt == 11)
	{ // stop
		cbKiller = 0;
	}
#else
	if (killopt == 10)
	{
		if (m_cbKillRound !=0 && m_cbGameRound == m_cbKillRound)
		{
			if (cutround > 0)
			{
				cbKiller = 1;
			}
			else if (cutround < 0)
			{
				cbKiller = 2;
			}
		}
	}
	else if (killopt == 11)
	{
		cbKiller = 0;
	}
#endif
	m_cbKillType = cbKiller;
	SLOG_CRITICAL << boost::format("%1%:%2%:[Kill-State]:%3%") % __FUNCTION__%__LINE__%cbKiller;

	Vec_UserID vecrobot;
	Vec_UserID vecother;
	Vec_UserID vecfind;
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto playerid = user.p_id;
		if (player == nullptr) continue;
		if (user.p_active == FALSE) continue;
		if (player->is_robot())
		{
			vecrobot.push_back(playerid);
		}
		else
		{
			vecother.push_back(playerid);
		}
	}
	if (cbKiller == 1)
	{
		// @系统赢
		m_MapPlayerCard.clear();
		m_GameCardMgr.Wash_CardControl();
		
		std::shuffle(vecrobot.begin(), vecrobot.end(), g);
		// @随机2个机器人赢
		for (auto id : vecrobot)
		{
			if (vecfind.size() > 2)
			{
				vecother.push_back(id);
			}
			else
			{
				vecfind.push_back(id);
			}
		}
		std::shuffle(vecother.begin(), vecother.end(), g);
		std::copy(vecother.begin(), vecother.end(), std::back_inserter(vecfind));
		
	}
	else if (cbKiller == 2)
	{
		// @玩家赢
		m_MapPlayerCard.clear();
		m_GameCardMgr.Wash_CardControl();
		
		std::shuffle(vecother.begin(), vecother.end(), g);
		// @随机2个玩家
		for (auto id: vecother)
		{
			if (vecfind.size() > 2)
			{
				vecrobot.push_back(id);
			}
			else
			{
				vecfind.push_back(id);
			}
		}
		std::shuffle(vecrobot.begin(), vecrobot.end(), g);
		std::copy(vecrobot.begin(), vecrobot.end(), std::back_inserter(vecfind));
	}
	else
	{ 
		// @纯随机发牌
		m_MapPlayerCard.clear();
		m_GameCardMgr.Wash_CardNomal();
		for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
		{
			auto &user = it->second;
			auto player = user.p_playerptr;
			if (player == nullptr) continue;
			if (user.p_active == FALSE) continue;
			uint32_t uid = user.p_id;
			vecfind.push_back(uid);
// 			GameCard card;
// 			memset(&card, 0, sizeof(card));
// 			m_GameCardMgr.GetUserCard(card);
// 			int32_t nModel = card.nModel;
// 			SLOG_CRITICAL << boost::format("[CHK] playerid:%1% Model:%2% Card:[%3%,%4%,%5%,%6%,%7%]")%uid%nModel 
// 				%card.nCard[0]%card.nCard[1]%card.nCard[2]%card.nCard[3]%card.nCard[4];
// 
// 			std::shuffle(card.nCard.begin(), card.nCard.end(), g);
// 			m_MapPlayerCard.insert(std::make_pair(uid, card));
		}
	}
	// 
	for (auto f_id : vecfind)
	{
		GameCard card;
		memset(&card, 0, sizeof(card));
		m_GameCardMgr.GetUserCard(card);

		int32_t nModel = card.nModel;
		SLOG_CRITICAL << boost::format("[CHK] playerid:%1% Model:%2% Card:[%3%,%4%,%5%,%6%,%7%]") % f_id%nModel
			%card.nCard[0] % card.nCard[1] % card.nCard[2] % card.nCard[3] % card.nCard[4];

		std::shuffle(card.nCard.begin(), card.nCard.end(), g);
		m_MapPlayerCard.insert(std::make_pair(f_id, card));

#ifdef UsingLog
		char szCardinfo[MAX_CARDS_HAND] = { 0 };
		for (int i = 0; i < MAX_CARDS_HAND; i++)
		{
			szCardinfo[i] = card.nCard[i];
		}
		auto count = mGameLog.pinfo_size();
		for (int i = 0; i < count; i++)
		{
			auto pinfo = mGameLog.mutable_pinfo(i);
			auto pid = pinfo->pid();
			if (pid == f_id)
			{
				pinfo->set_cardtype(nModel);
				pinfo->set_cardhand(szCardinfo,MAX_CARDS_HAND);
				break;
			}
		}
#endif // UsingLog
	}
}

