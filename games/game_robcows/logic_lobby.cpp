#include "stdafx.h"
#include "logic_lobby.h"
#include "logic_room.h"
#include "logic_player.h"
#include <i_game_player.h>

#include "proc_robcows_logic.h"
#include "proc_robcows_protocol.h"

#include "RobCows_RoomCFG.h"
#include "RobCows_BaseInfo.h"
#include "RobCows_RobotCFG.h"
#include "RobCows_RobotGameCFG.h"
#include "RobCows_RobotRoomCFG.h"

#include "game_db_log.h"
#include "game_engine.h"
#include "time_helper.h"
#include <enable_random.h>

#include "robot_mgr.h"

ROBCOWS_SPACE_USING

logic_lobby::logic_lobby(void)
:m_init(false)
,m_max_player(0)
,m_check_cache(0.0)
{
	
}

logic_lobby::~logic_lobby(void)
{
}

void logic_lobby::init_config()
{	
	RobCows_RoomCFG::GetSingleton()->Load();
	RobCows_BaseInfo::GetSingleton()->Load();
	RobCows_RobotCFG::GetSingleton()->Load();
	RobCows_RobotGameCFG::GetSingleton()->Load();
	RobCows_RobotRoomCFG::GetSingleton()->Load();
}

void logic_lobby::reload_fishcfg()
{
}

void logic_lobby::init_protocol()
{
	init_proc_robcows_logic();
	init_proc_robcows_protocol();
}

void robcows_space::logic_lobby::init_game(int count, int32_t single_room /*= -1*/)
{
	m_max_player = count;
	if(m_max_player<=0||m_max_player>100000)
		m_max_player = 20000;
	// 初始化协议
	init_protocol();
	// 初始化配置
	init_config();
	// 创建游戏房间
	const boost::unordered_map<int, RobCows_RoomCFGData>& list = RobCows_RoomCFG::GetSingleton()->GetMapData();
	for (auto it = list.begin(); it != list.end(); ++it)
	{
		if (!it->second.mIsOpen) continue;
		auto rid = it->second.mRoomID;
		if (rid == single_room)
		{
			// 创建房间
			auto room = boost::make_shared<logic_room>(&(it->second), this);
			// 保存房间信息
			m_rooms.insert(std::make_pair(it->second.mRoomID, room));
		}
	}
	
	// Robot
	robot_mgr::instance();

	m_init = true;
}

void logic_lobby::heartbeat( double elapsed )
{
	if(!m_init)return;

	for (auto it = m_rooms.begin(); it != m_rooms.end(); ++it)
	{
		it->second->heartbeat(elapsed);
	}

	robot_mgr::instance().heartbeat(elapsed);
	// 
	m_check_cache += elapsed;

	if(m_check_cache > 60)
	{
		save_cache();
		m_check_cache = 0;
	}
}


//返回一个机器人 返回的机器人未进入房间？
void logic_lobby::response_robot(int32_t playerid, int tag)
{
	SLOG_CRITICAL<<boost::format("Robot:%1% Enter room:%2%")%playerid%tag;
	auto it = m_all_players.find(playerid);
	if (it != m_all_players.end())
	{
		if(it->second->get_table() != nullptr)
		{
			// 清理
			game_engine::instance().release_robot(playerid);
		}
		else
		{
			int rid = tag % 100;
			int tid = tag / 100;
			int ret = enter_room(playerid,rid,tid);
			if(ret!=1)
			{
				game_engine::instance().release_robot(playerid);
			}
			else
			{
				robot_enter(it->second,tag);
			}
		}
	}
}

void robcows_space::logic_lobby::robot_leave(int32_t playerid)
{
	auto sendmsg = PACKET_CREATE(packetl2c_robot_leave, e_mst_l2c_robot_leave);
	sendmsg->set_player_id(playerid);
	robot_mgr::instance().recv_packet(playerid, sendmsg->packet_id(), sendmsg);

	SLOG_CRITICAL<<"logic_lobby::robot_leave :"<<playerid;
}

void robcows_space::logic_lobby::robot_enter(LPlayerPtr& player, int tag)
{
	int32_t rid = tag % 100;
	int32_t tid = tag / 100;

	auto sendmsg = PACKET_CREATE(packetl2c_robot_enter, e_mst_c2l_robot_enter);
	sendmsg->set_roomid(rid);
	sendmsg->set_tableid(tid);
	auto playerinfo = sendmsg->mutable_playerinfo();
	playerinfo->set_seat_index(player->get_seat());
	playerinfo->set_player_id(player->get_pid());
	playerinfo->set_player_nickname(player->get_nickname());
	playerinfo->set_player_headframe(player->get_photo_frame());
	playerinfo->set_player_headcustom(player->get_icon_custom());
	playerinfo->set_player_gold(player->get_gold());
	playerinfo->set_player_sex(player->get_sex());
	playerinfo->set_player_viplv(player->get_viplvl());
	playerinfo->set_player_ticket(player->get_ticket());
	playerinfo->set_player_region(player->GetUserRegion());
	robot_mgr::instance().recv_packet(player->get_pid(),sendmsg->packet_id(),sendmsg);

	SLOG_CRITICAL<<"logic_lobby::robot_enter :"<<player->get_pid() <<",roomid:"<<rid<<",tableid:"<<tid;
	
}


void logic_lobby::release_game()
{
	if(!m_init)return;

	for (auto it = m_all_players.begin(); it != m_all_players.end(); ++it)
	{
		it->second->release();
	}
	m_all_players.clear();

	for (auto it = m_rooms.begin(); it != m_rooms.end(); ++it)
	{
		it->second->release();
	}
	m_rooms.clear();

	db_log::instance().close();

}

bool logic_lobby::player_enter_game(iGPlayerPtr igplayer)
{
	if(!m_init)return false;

	if(m_all_players.size()>= m_max_player)
		return false;

	if(m_all_players.find(igplayer->get_playerid()) != m_all_players.end())
		return false;

	auto lp = logic_player::malloc();
	lp->init(igplayer);
	igplayer->set_handler(lp);
	lp->enter_game(this);

	//@同步用户状态;
	auto geh = game_engine::instance().get_handler();
	if (geh)
	{
		geh->sync_userGameStatus(igplayer->get_playerid(),
			game_engine::instance().get_gameid(),
			0,
			0,
			0,
			0
		);
	}

	m_all_players.insert(std::make_pair(igplayer->get_playerid(), lp));

	SLOG_CRITICAL << boost::format("player_enter_game:%1%")%igplayer->get_playerid();

	return true;
}

void logic_lobby::player_leave_game(uint32_t playerid, bool bforce /*= false*/)
{
	if(!m_init)return;

	auto it = m_all_players.find(playerid);
	if(it == m_all_players.end()) return;
	SLOG_CRITICAL << boost::format("player_leave_game:%1%")%playerid;


	//@同步用户状态;
	auto geh = game_engine::instance().get_handler();
	if (geh)
	{
		geh->sync_userGameStatus(playerid,
			0,
			0,
			0,
			0,
			0
		);
	}

	// deal table
	it->second->leave_table();

	m_all_players.erase(it);
}

void logic_lobby::player_kick_player(uint32_t playerid, int bforce /*= 0*/)
{
	if (0 == bforce)
	{
		SLOG_WARNING << boost::format("Warning: 254");
	}
	else if (1 == bforce)
	{
		gameover_getout(playerid);
	}
	else if (2 == bforce)
	{
		player_leave_game(playerid, true);
	}
	else if (25 == bforce)
	{
		player_clean_player(playerid);
	}
	else
	{
		SLOG_WARNING << boost::format("Warning: 266");
	}
}

void logic_lobby::gameover_getout(uint32_t playerid)
{ // 游戏结束之后踢出用户
	for (auto room : m_rooms)
	{
		auto proom = room.second;
		if (proom) proom->getout_room(playerid);
	}
}

int logic_lobby::player_join_friend_game(iGPlayerPtr igplayer, uint32_t friendid)
{
	//if(player_enter_game(igplayer))
	//{
	//	auto it = m_all_players.find(friendid);
	//	if(it == m_all_players.end())
	//		return 2;

	//	auto table = it->second->get_table();
	//	if(table != nullptr)
	//		return join_table(igplayer->get_playerid(), table->get_room()->get_id(), table->TableID->get_value());

	//	return 1;
	//}
	
	return 2;
}

void logic_lobby::player_clean_player(uint32_t playerid)
{
	// @ 1、lobby
	auto it = m_all_players.find(playerid);
	if (it != m_all_players.end())
	{
		auto geh = game_engine::instance().get_handler();
		if (geh)
		{
			geh->sync_userGameStatus(playerid,
				0,
				0,
				0,
				0,
				0
			);
		}
		// deal table
		it->second->leave_table();
		m_all_players.erase(it);
		SLOG_CRITICAL << boost::format("player_clear_player lobby:%1%") % playerid;
	}
	else
	{
		for (auto it = m_rooms.begin(); it != m_rooms.end(); ++it)
		{
			auto room = it->second;
			if (room)
			{
				room->cleanplayer(playerid);
			}
		}

		SLOG_CRITICAL << boost::format("player_clear_player room:%1%") % playerid;
	}


}

const LROOM_MAP& logic_lobby::get_rooms()
{
	return m_rooms;
}

int logic_lobby::enter_room(uint32_t pid, uint16_t rid, uint16_t tid/*=0*/)
{
	SLOG_CRITICAL<<"logic_lobby:enter_room check begin";
	auto it = m_all_players.find(pid);
	if(it == m_all_players.end())
		return 2;

	auto room = m_rooms.find(rid);
	if(room == m_rooms.end())
		return 2;

	auto data = room->second->get_data();
	auto pl = it->second;

	if(data->mGoldMinCondition > pl->get_gold())
		return 10;//金币不够	
	if(data->mGoldMaxCondition !=0 && data->mGoldMaxCondition < pl->get_gold())
		return 80; // 金币过多
	if(data->mVipCondition > pl->get_viplvl())
		return 13;//vip不够	

	if(data->mTicketCondition > pl->get_ticket())
		return 11;//礼券不够
	
	if(tid==0)
	{
#if 0
		if(!room->second->has_seat(tid))
			return 12;//房间已满
#else
		if(pl->is_robot())
		{
			if(!room->second->has_seat_robot(tid))
				return 12;//房间已满
		}
		else
		{
			if(!room->second->has_seat(tid))
				return 12;//房间已满
		}

#endif
	}
	return room->second->enter_table(pl,tid);
}


void logic_lobby::leave_room(uint32_t pid)
{
	auto it = m_all_players.find(pid);
	if(it == m_all_players.end())
		return;

	it->second->leave_table();
}

void logic_lobby::enter_room_nocheck(uint32_t pid, uint16_t rid)
{
// 	auto it = m_all_players.find(pid);
// 	if(it == m_all_players.end())
// 	{
// 		SLOG_ERROR << "logic_lobby::enter_room_nocheck can't find player id:"<<pid;
// 		return;
// 	}
// 
// 	auto room = m_rooms.find(rid);
// 	if(room == m_rooms.end())
// 	{
// 		SLOG_ERROR << "logic_lobby::enter_room_nocheck can't find room id:"<<rid;
// 		return;
// 	}
// 
// 	auto pl = it->second;
// 	room->second->enter_table(pl);
}

LPlayerPtr& logic_lobby::get_player(uint32_t pid)
{
	auto it = m_all_players.find(pid);
	if(it != m_all_players.end())
	{
		return it->second;
	}
	return logic_player::EmptyPtr;
}

logic_table* logic_lobby::get_player_table(uint32_t pid)
{
	auto it = m_all_players.find(pid);
	if(it != m_all_players.end())
	{
		return it->second->get_table();
	}
	return nullptr;
}

void logic_lobby::gmPlatformOpt(int32_t optype, int32_t gameID, int32_t roomID /*= 0*/, int32_t cutRound /*= 0*/, int32_t exData1 /*= 0*/, int32_t exData2 /*= 0*/)
{
	//if (gameID != game_engine::instance().get_gameid()) return;
	// 处理对应 Game_ID
	if (0 == roomID)
	{ // 处理所有房间
		for (auto room : m_rooms)
		{
			auto proom = room.second;
			if (proom == nullptr) continue;
			if (optype == 100)
			{ // stop service
				proom->setservice(0);
			}
			else if (optype == 10 && exData1 == 0)
			{ // kill
				proom->setkiller(optype, cutRound);
			}
			else if (optype == 11)
			{ // stop kill
				proom->setkiller(optype, cutRound);
			}
			else if (optype == 10 && (exData1 == 1 || exData1 == 2))
			{ // robot
				proom->setRobotSwitch(exData1);
			}
			else
			{
				SLOG_WARNING << boost::format("gmPlatformOpt Refuse optype:%d") % optype;
			}
		}
	}
	else
	{ // 处理指定房间
		auto froomit = m_rooms.find(roomID);
		if (froomit != m_rooms.end())
		{
			auto proom = froomit->second;
			if (optype == 100)
			{ // stop service
				proom->setservice(0);
			}
			else if (optype == 10 && exData1 == 0)
			{ // kill
				proom->setkiller(optype, cutRound);
			}
			else if (optype == 11)
			{ // stop kill
				proom->setkiller(optype, cutRound);
			}
			else if (optype == 10 && (exData1 == 1 || exData1 == 2))
			{ // robot
				proom->setRobotSwitch(exData1);
			}
			else
			{
				SLOG_WARNING << boost::format("gmPlatformOpt Refuse optype:%d") % optype;
			}
		}
	}
}

//缓存当日统计
void logic_lobby::save_cache()
{
	int64_t total_bet_gold = 0;
	int64_t total_win_gold = 0;
	int64_t	total_lose_gold = 0;
	int64_t room_lose_gold[4] = {0};
	int64_t room_win_gold[4] = {0};

	bool need_save = false;
	int i = 0;
	for (auto it = m_rooms.begin(); it != m_rooms.end(); ++it)
	{
		int64_t win_gold = 0;
		int64_t lose_gold = 0;
		
		total_win_gold += win_gold;
		total_lose_gold += lose_gold;
		if (i < 4)
		{
			room_win_gold[i] = win_gold;
			room_lose_gold[i] = lose_gold;
			i++;
		}


		if (!need_save && (lose_gold > 0 || win_gold > 0))
		{
			need_save = true;
		}
		
	}

	if(need_save)
	{
		auto now = time_helper::instance().get_cur_date();
		time_t nt = time_helper::convert_from_date(now) * 1000;

		db_log::instance().push_update(2, BSON("Date" << mongo::Date_t(nt)),
			BSON("$inc" << BSON("TodayOutlay" << (long long)total_lose_gold << "TodayIncome" << (long long)total_win_gold
				<< "room1Income" << (long long)room_win_gold[0] << "room1Outlay" << (long long)room_lose_gold[0]
				<< "room2Income" << (long long)room_win_gold[1] << "room2Outlay" << (long long)room_lose_gold[1]
				<< "room3Income" << (long long)room_win_gold[2] << "room3Outlay" << (long long)room_lose_gold[2]
				<< "room4Income" << (long long)room_win_gold[3] << "room4Outlay" << (long long)room_lose_gold[3]
			)));
	}
}