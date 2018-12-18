#include "stdafx.h"
#include "logic_lobby.h"
#include "logic_room.h"
#include "logic_player.h"
#include <i_game_player.h>
#include "logic_table.h"
#include "time_helper.h"
#include "game_db_log.h"
#include "M_VIPProfitCFG.h"
#include "proc_example_logic.h"
#include "game_db.h"

EXAMPLE_SPACE_USING

logic_lobby::logic_lobby(void)
:m_init(false)
,m_max_player(0)
{
	
}

logic_lobby::~logic_lobby(void)
{
}

void logic_lobby::init_config()
{
	M_VIPProfitCFG::GetSingleton()->Load();
}



void logic_lobby::init_protocol()
{
	init_proc_example_logic();
}

void logic_lobby::init_game(int count)
{
	m_max_player = count;
	if(m_max_player<=0||m_max_player>100000)
		m_max_player = 20000;

	init_protocol();
	init_config();

	//const boost::unordered_map<int, Fish_RoomCFGData>& list = Fish_RoomCFG::GetSingleton()->GetMapData();
	//for (auto it = list.begin(); it != list.end(); ++it)
	//{
	//	if(!it->second.mIsOpen)
	//		continue;

	//	auto room = boost::make_shared<logic_room>(&(it->second), this);
	//	m_rooms.insert(std::make_pair(it->second.mRoomID, room));
	//}
	m_init = true;
}

void logic_lobby::heartbeat( double elapsed )
{
	if(!m_init)return;

	for (auto it = m_rooms.begin(); it != m_rooms.end(); ++it)
	{
		it->second->heartbeat(elapsed);
	}
}

void logic_lobby::release_game()
{
	if(!m_init)return;

	for (auto it = m_all_players.begin(); it != m_all_players.end(); ++it)
	{
		it->second->release();
	}
	m_all_players.clear();

	//db_log::instance().flush();
}

bool logic_lobby::player_enter_game(iGPlayerPtr igplayer)
{
	if(!m_init)return false;

	if(m_all_players.size()>= m_max_player)
		return false;

	if(m_all_players.find(igplayer->get_playerid()) != m_all_players.end())
		return false;

	auto lp = logic_player::malloc();
	lp->set_player(igplayer);
	igplayer->set_handler(lp);
	lp->enter_game(this);


	m_all_players.insert(std::make_pair(igplayer->get_playerid(), lp));

	return true;
}

void logic_lobby::player_leave_game(uint32_t playerid)
{
	if(!m_init)return;

	auto it = m_all_players.find(playerid);
	if(it == m_all_players.end())
		return;

	it->second->leave_table();
	m_all_players.erase(it);
}

int logic_lobby::player_join_friend_game(iGPlayerPtr igplayer, uint32_t friendid)
{
	if(player_enter_game(igplayer))
	{
		auto it = m_all_players.find(friendid);
		if(it == m_all_players.end())
			return 2;

		auto table = it->second->get_table();
		if(table != nullptr)
			return join_table(igplayer->get_playerid(), table->get_room()->get_id(), table->TableID->get_value());

		return 1;
	}
	
	return 2;
}

const LROOM_MAP& logic_lobby::get_rooms()
{
	return m_rooms;
}

int logic_lobby::join_table(uint32_t pid, uint16_t rid, uint16_t tid)
{
	auto it = m_all_players.find(pid);
	if(it == m_all_players.end())
		return 2;

	auto room = m_rooms.find(rid);
	if(room == m_rooms.end())
		return 2;

	//auto data = room->second->get_data();
	auto pl = it->second;

	//if(data->mGoldCondition > pl->get_gold())
	//	return 10;//金币不够	

	//if(data->mVipCondition > pl->get_viplvl())
	//	return 13;//vip不够	

	//if(data->mLevelCondition > pl->Level->get_value())
	//	return 14;//等级不够

	//if(data->mTicketCondition > pl->get_ticket())
	//	return 11;//礼券不够	

	if(tid == 0)
	{
		if(!room->second->has_seat(tid))
			return 12;//房间已满
	}	
	
	return room->second->enter_table(pl, tid);
}


void logic_lobby::leave_talbe(uint32_t pid)
{
	auto it = m_all_players.find(pid);
	if(it == m_all_players.end())
		return;

	it->second->leave_table();
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

LPlayerPtr& logic_lobby::get_player(uint32_t pid)
{
	auto it = m_all_players.find(pid);
	if(it != m_all_players.end())
	{
		return it->second;
	}
	return logic_player::EmptyPtr;
}

//返回一个机器人 返回的机器人未进入房间
void logic_lobby::response_robot(int32_t playerid, int tag)
{

}