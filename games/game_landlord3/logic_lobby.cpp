#include "stdafx.h"
#include "logic_lobby.h"
#include "logic_room.h"
#include "logic_player.h"
#include <i_game_player.h>

#include "proc_landlord_logic.h"
#include "proc_landlord_protocol.h"

#include "Landlord3_RoomCFG.h"
#include "Landlord3_BaseInfo.h"
#include "Landlord3_RobotCFG.h"
#include "Landlord3_RobotGameCFG.h"


#include "game_db_log.h"
#include "game_engine.h"
#include "i_game_ehandler.h"
#include "time_helper.h"
#include <enable_random.h>

#include "robot_mgr.h"

LANDLORD_SPACE_USING

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
	Landlord3_RoomCFG::GetSingleton()->Load();
	Landlord3_BaseInfo::GetSingleton()->Load();
	Landlord3_RobotCFG::GetSingleton()->Load();
	Landlord3_RobotGameCFG::GetSingleton()->Load();
}

void logic_lobby::reload_fishcfg()
{
}

void logic_lobby::init_protocol()
{
	init_proc_landlord_logic();
	init_proc_landlord_protocol();
}

void logic_lobby::init_game(int count, int32_t room_id, int child_id)
{
	m_max_player = count;
	if(m_max_player<=0||m_max_player>100000)
		m_max_player = 20000;
	// ��ʼ��Э��
	init_protocol();
	// ��ʼ������
	init_config();
	// ������Ϸ����
	const boost::unordered_map<int, Landlord3_RoomCFGData>& list = Landlord3_RoomCFG::GetSingleton()->GetMapData();
	for (auto it = list.begin(); it != list.end(); ++it)
	{
		if(!it->second.mIsOpen) continue;
		// ��������
        auto id = it->first;
        if (room_id == id)
        {
            auto room = boost::make_shared<logic_room>(&(it->second), this, child_id);
            // ���淿����Ϣ
            m_rooms.insert(std::make_pair(it->second.mRoomID, room));
        }
	}
	
	// Robot
	//robot_mgr::instance();

	m_init = true;
}

void logic_lobby::heartbeat( double elapsed )
{
	if(!m_init)return;

	for (auto& item : m_rooms)
	{
        auto room = item.second;
        room->heartbeat(elapsed);
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


//����һ�������� ���صĻ�����δ���뷿�䣿
void logic_lobby::response_robot(int32_t playerid, int tag)
{
	SLOG_CRITICAL<<boost::format("Robot:%1% Enter room:%2%")%playerid%tag;
	auto it = m_all_players.find(playerid);
	if (it != m_all_players.end())
	{
		if(it->second->get_table() != nullptr)
		{
			// ����
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
void logic_lobby::kill_points(uint16_t rid, int32_t cutRound, bool status)
{
	if (rid == 0)
	{
		for (auto& room : m_rooms)
		{
			//room.second->kill_points(cutRound, status);
		}
	}
	else
	{
		auto itr = m_rooms.find(rid);
		if (itr != m_rooms.end())
		{
			//itr->second->kill_points(cutRound, status);
		}
	}
}
void landlord_space::logic_lobby::robot_leave(int32_t playerid)
{
	auto sendmsg = PACKET_CREATE(packetl2c_robot_leave, e_mst_l2c_robot_leave);
	sendmsg->set_player_id(playerid);
	robot_mgr::instance().recv_packet(playerid, sendmsg->packet_id(), sendmsg);

	SLOG_CRITICAL<<"logic_lobby::robot_leave :"<<playerid;
}

void landlord_space::logic_lobby::robot_enter(LPlayerPtr& player, int tag)
{
    int rid = tag % 100;
    int tid = tag / 100;

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
	//playerinfo->set_player_region(player->GetUserRegion());
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
void logic_lobby::service_ctrl(int32_t opttype, int32_t roomID)
{
	if (roomID == 0)
	{
		for (auto& item : m_rooms)
		{
			//item.second->service_ctrl(opttype);
		}
	}
	else
	{
		auto itr = m_rooms.find(roomID);
		if (itr != m_rooms.end())
		{
			//itr->second->service_ctrl(opttype);
		}
	}
}

void logic_lobby::kick_player(uint32_t playerid, int bforce)
{
	for (auto& room : m_rooms)
	{
		//room.second->kick_player(playerid, bforce);
	}
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

	m_all_players.insert(std::make_pair(igplayer->get_playerid(), lp));

	SLOG_CRITICAL << boost::format("player_enter_game:%1%")%igplayer->get_playerid();

    //@ͬ���û�״̬;
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

	return true;
}

void logic_lobby::player_leave_game(uint32_t playerid)
{
	if(!m_init)return;

	auto it = m_all_players.find(playerid);
	if(it == m_all_players.end()) return;
	SLOG_CRITICAL << boost::format("player_leave_game:%1%")%playerid;

	// deal table
	it->second->leave_table();

	m_all_players.erase(it);

    //@ͬ���û�״̬;
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
		return 10;//��Ҳ���	
	if(data->mGoldMaxCondition !=0 && data->mGoldMaxCondition < pl->get_gold())
		return 80; // ��ҹ���
	if(data->mVipCondition > pl->get_viplvl())
		return 13;//vip����	

	if(data->mTicketCondition > pl->get_ticket())
		return 11;//��ȯ����
	
	if(tid==0)
	{
		if(!room->second->has_seat(tid))
			return 12;//��������
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
//���浱��ͳ��
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

		db_log::instance().push_update(2, BSON("Date"<<mongo::Date_t(nt)), 
			BSON("$inc"<<BSON("TodayOutlay"<<total_lose_gold << "TodayIncome" << total_win_gold
			<< "room1Income" << room_win_gold[0] << "room1Outlay" << room_lose_gold[0]
			<< "room2Income" << room_win_gold[1] << "room2Outlay" << room_lose_gold[1]
			<< "room3Income" << room_win_gold[2] << "room3Outlay" << room_lose_gold[2]
			<< "room4Income" << room_win_gold[3] << "room4Outlay" << room_lose_gold[3]
			)));
	}
}