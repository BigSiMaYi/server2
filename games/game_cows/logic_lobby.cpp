#include "stdafx.h"
#include "logic_lobby.h"
#include "logic_room.h"
#include "logic_player.h"
#include <i_game_player.h>

#include "proc_cows_logic.h"
#include "proc_cows_protocol.h"

#include "Cows_RoomCFG.h"
#include "Cows_BaseInfo.h"
#include "Cows_CardsCFG.h"
#include "Cows_RobotCFG.h"
#include "logic_main.h"

#include "game_db_log.h"
#include "game_engine.h"
#include "time_helper.h"
#include <enable_random.h>

COWS_SPACE_USING

logic_lobby::logic_lobby(void)
	:m_init(false)
	, m_max_player(0)
	, m_check_cache(0.0)
{

}

logic_lobby::~logic_lobby(void)
{
}

void logic_lobby::init_config()
{
	Cows_RobotCFG::GetSingleton()->Load();
	Cows_RoomCFG::GetSingleton()->Load();
	Cows_BaseInfo::GetSingleton()->Load();
	Cows_CardsCFG::GetSingleton()->Load();
}

void logic_lobby::reload_fishcfg()
{
}

void logic_lobby::init_protocol()
{
	init_proc_cows_logic();
	init_proc_cows_protocol();
}

void logic_lobby::init_game(int count, int room_id, int child_id)
{
	m_max_player = count;
	if (m_max_player <= 0 || m_max_player > 100000)
		m_max_player = 20000;

	init_protocol();
	init_config();

	const boost::unordered_map<int, Cows_RoomCFGData>& list = Cows_RoomCFG::GetSingleton()->GetMapData();
	for (auto it = list.begin(); it != list.end(); ++it)
	{
		if (!it->second.mIsOpen)
			continue;
		auto id = it->first;
		if (room_id == id)
		{
			auto room = boost::make_shared<logic_room>(&(it->second), this, child_id);
			m_rooms.insert(std::make_pair(it->second.mRoomID, room));
		}
	}

	m_init = true;
}

void logic_lobby::heartbeat(double elapsed)
{
	if (!m_init)return;

	for (auto it = m_rooms.begin(); it != m_rooms.end(); ++it)
	{
		it->second->heartbeat(elapsed);
	}

	m_check_cache += elapsed;
	if (m_check_cache > 60)
	{
		save_cache();
		m_check_cache = 0;
	}
}

//返回一个机器人 返回的机器人未进入房间？
void logic_lobby::response_robot(int32_t playerid, int tag)
{
	auto it = m_all_players.find(playerid);
	if (it != m_all_players.end())
	{
		enter_room(playerid, tag);
	}
}

void logic_lobby::kill_points(uint16_t rid, int32_t cutRound, bool status)
{
	if (rid == 0)
	{
		for (auto& room : m_rooms)
		{
			room.second->kill_points(cutRound, status);
		}
	}
	else
	{
		auto itr = m_rooms.find(rid);
		if (itr != m_rooms.end())
		{
			itr->second->kill_points(cutRound, status);
		}
	}
}

void logic_lobby::release_game()
{
	if (!m_init)return;

	for (auto it = m_all_players.begin(); it != m_all_players.end(); ++it)
	{
		it->second->release();
	}
	m_all_players.clear();
}

void logic_lobby::service_ctrl(int32_t opttype, int32_t roomID)
{
	if (roomID == 0)
	{
		for (auto& item : m_rooms)
		{
			item.second->service_ctrl(opttype);
		}
	}
	else
	{
		auto itr = m_rooms.find(roomID);
		if (itr != m_rooms.end())
		{
			itr->second->service_ctrl(opttype);
		}
	}
}

void cows_space::logic_lobby::kick_player(uint32_t playerid, int bforce)
{
	for (auto& room : m_rooms)
	{
		room.second->kick_player(playerid, bforce);
	}
}

bool logic_lobby::player_enter_game(iGPlayerPtr igplayer)
{
	if (!m_init)return false;

	if (m_all_players.size() >= m_max_player)
		return false;

	if (m_all_players.find(igplayer->get_playerid()) != m_all_players.end())
		return false;

	auto lp = logic_player::malloc();
	lp->init(igplayer);
	igplayer->set_handler(lp);
	lp->enter_game(this);

	m_all_players.insert(std::make_pair(igplayer->get_playerid(), lp));

	auto geh = game_engine::instance().get_handler();
	if (geh)
	{
		auto pid = igplayer->get_playerid();
		geh->sync_userGameStatus(igplayer->get_playerid(),
			game_engine::instance().get_gameid(),
			0,
			0,
			0,
			0
		);
	}
	//自动进入房间
	//enter_room(igplayer->get_playerid(), 1);
	return true;
}

void logic_lobby::player_leave_game(uint32_t playerid)
{
	if (!m_init)return;

	auto it = m_all_players.find(playerid);
	if (it == m_all_players.end())
		return;

	it->second->leave_room();

	//@用户离开房间刷新状态;
	auto geh = game_engine::instance().get_handler();
	if (geh && !it->second->is_robot())
	{
		geh->sync_userGameStatus(playerid,
			0,
			0,
			0,
			0,
			0
		);
	}

	m_all_players.erase(it);
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

const LROOM_MAP& logic_lobby::get_rooms()
{
	return m_rooms;
}

int logic_lobby::enter_room(uint32_t pid, uint16_t rid)
{
	auto it = m_all_players.find(pid);
	if (it == m_all_players.end())
		return 2;

	auto room = m_rooms.find(rid);
	if (room == m_rooms.end())
		return 2;

	auto data = room->second->get_data();
	auto pl = it->second;

	if (data->mGoldCondition > pl->get_gold())
		return 10;//金币不够	

	if (data->mVipCondition > pl->get_viplvl())
		return 13;//vip不够	

	if (data->mTicketCondition > pl->get_ticket())
		return 11;//礼券不够

	return room->second->enter_room(pl);
}

void logic_lobby::enter_room_nocheck(uint32_t pid, uint16_t rid)
{
	auto it = m_all_players.find(pid);
	if (it == m_all_players.end())
	{
		SLOG_ERROR << "logic_lobby::enter_room_nocheck can't find player id:" << pid;
		return;
	}

	auto room = m_rooms.find(rid);
	if (room == m_rooms.end())
	{
		SLOG_ERROR << "logic_lobby::enter_room_nocheck can't find room id:" << rid;
		return;
	}

	auto pl = it->second;
	room->second->enter_room(pl);
}

LPlayerPtr& logic_lobby::get_player(uint32_t pid)
{
	auto it = m_all_players.find(pid);
	if (it != m_all_players.end())
	{
		return it->second;
	}
	return logic_player::EmptyPtr;
}

//缓存当日统计
void logic_lobby::save_cache()
{
	int64_t total_bet_gold = 0;
	int64_t total_win_gold = 0;
	int64_t	total_lose_gold = 0;
	int64_t room_lose_gold[4] = { 0 };
	int64_t room_win_gold[4] = { 0 };

	bool need_save = false;
	int i = 0;
	for (auto it = m_rooms.begin(); it != m_rooms.end(); ++it)
	{
		int64_t win_gold = it->second->get_today_win_gold();
		int64_t lose_gold = it->second->get_today_lose_gold();

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
		it->second->clear_today_gold();
	}

	if (need_save)
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