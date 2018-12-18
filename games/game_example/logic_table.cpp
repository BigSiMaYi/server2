#include "stdafx.h"
#include "logic_table.h"
#include "logic_player.h"
#include "logic_room.h"
#include "game_engine.h"
#include "i_game_ehandler.h"
#include "game_db.h"
#include "logic_lobby.h"
#include <net\packet_manager.h>
#include "fish_logic.pb.h"

EXAMPLE_SPACE_USING

static const int MAX_TALBE_PLAYER = 4;//桌子人数
static const int MAX_OP_PLAYER = 4;//观战人数

logic_table::logic_table(void)
:m_room(nullptr)
,m_players(MAX_TALBE_PLAYER)//每个桌子4个人
,m_player_count(0)
,m_elapse(0.0)
,m_checksave(0)
{
	init_game_object();
}


logic_table::~logic_table(void)
{
}

void logic_table::init_talbe(uint16_t tid, logic_room* room)
{
	m_room = room;
	TableID->set_value(tid);
	if(!load_table())
		create_table();

}

uint32_t logic_table::get_id()
{
	return TableID->get_value();
}

void logic_table::inc_dec_count(bool binc)
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

	for (int i = 0;i<MAX_TALBE_PLAYER;i++)
	{
		if(m_players[i] != nullptr)
		{
			m_players[i]->heartbeat(elapsed);
		}
	}

	//同步协议
	m_elapse += elapsed;
	if (m_elapse >= 0.2)
	{
		m_elapse = 0.0;
		broadcast_msglist_to_client();
	}

	m_checksave+= elapsed;
	if (m_checksave > 30)//桌子信息30s保存1次
	{
		store_game_object();
	}
}

bool logic_table::is_full()
{
	return m_player_count >=MAX_TALBE_PLAYER;
}

unsigned int logic_table::get_max_table_player()
{
	return m_players.size();
}

LPlayerPtr& logic_table::get_player(int index)
{
	return m_players[index];
}

LPlayerPtr& logic_table::get_player_byid(uint32_t pid)
{
	for (int i = 0; i < MAX_TALBE_PLAYER; i++)
	{
		if (m_players[i] != nullptr && m_players[i]->get_pid() == pid)
		{
			return m_players[i];
		}
	}
	return logic_player::EmptyPtr;
}

logic_room* logic_table::get_room()
{
	return m_room;
}


int logic_table::enter_table(LPlayerPtr player)
{
	if(!player->join_table(this))
		return 2;

	for (int i =0;i<MAX_TALBE_PLAYER;i++)
	{
		if(m_players[i] == nullptr)
		{	
			m_players[i] = player;
			inc_dec_count();
			bc_enter_seat(i, player);
			return 1;
		}
	}

	player->join_table(nullptr);
	return 12;
}
void logic_table::leave_table(uint32_t pid)
{
	for (int i =0;i<MAX_TALBE_PLAYER;i++)
	{
		if(m_players[i] != nullptr && m_players[i]->get_pid() == pid)
		{
		
			
			m_players[i] = nullptr;
			inc_dec_count(false);
			m_room->on_leave_table();
			bc_leave_seat(pid);
			return;
		}
	}
}

bool logic_table::change_sit(uint32_t pid, uint32_t seat_index)
{
	if (seat_index >= MAX_TALBE_PLAYER)
		return false;

	if (m_players[seat_index] != nullptr)
		return false;

	LPlayerPtr oldPlayer;
	for (int i = 0; i < MAX_TALBE_PLAYER; i++)
	{
		if (m_players[i] != nullptr && m_players[i]->get_pid() == pid)
		{
			oldPlayer = m_players[i];
			m_players[i] = nullptr;
			inc_dec_count(false);
			break;
		}
	}

	if (oldPlayer == nullptr)
		return false;

	m_players[seat_index] = oldPlayer;
	inc_dec_count();

	bc_enter_seat(seat_index, oldPlayer);

	return true;
}

int logic_table::broadcast_msg_to_client(std::vector<uint32_t>& pids, uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg)
{
	return game_engine::instance().get_handler()->broadcast_msg_to_client(pids, packet_id, msg);
}

void logic_table::broadcast_msglist_to_client()
{

	if (!m_msglist.empty())
	{
		if (m_player_count > 0)
		{
			std::vector<uint32_t> pids;
			for (unsigned int i = 0; i < m_players.size(); i++)
			{
				if (m_players[i] != nullptr)
				{
					pids.push_back(m_players[i]->get_pid());
				}
			}
			game_engine::instance().get_handler()->broadcast_msglist_to_client(pids, m_msglist);
		}
		m_msglist.clear();
	}
}

void logic_table::bc_enter_seat(int seat_index, LPlayerPtr& player)
{
	//auto sendmsg = PACKET_CREATE(fish_protocols::packetl2c_bc_enter_seat, fish_protocols::e_mst_l2c_bc_enter_seat);
	//auto seat_info = sendmsg->mutable_seat_info();
	//seat_info->set_seat_index(seat_index);
	//seat_info->set_nickname(player->get_nickname());
	//seat_info->set_player_gold(player->get_gold());
	//seat_info->set_player_id(player->get_pid());
	//seat_info->set_turret_rate(player->get_turret_rate());
	//seat_info->set_turret_id(player->TurretID->get_value());
	//seat_info->set_player_ticket(player->get_ticket());
	//seat_info->set_turret_power(player->get_turret()->get_turret_power());

	//seat_info->set_sex(player->get_sex());
	//seat_info->set_icon_custom(player->get_icon_custom());
	//seat_info->set_photo_frame(player->get_photo_frame());
	//seat_info->set_vip(player->get_viplvl());

	//auto blist = player->get_turret()->get_bufflist();
	//seat_info->mutable_bufflist()->Reserve(blist.size());
	//for (auto it = blist.begin(); it != blist.end(); ++it)
	//{
	//	auto binfo = seat_info->add_bufflist();
	//	binfo->set_buffid(it->first);
	//	binfo->set_outtime(it->second);
	//}

	//add_msg_to_list(sendmsg);
}

void logic_table::bc_leave_seat(int player_id)
{
	//auto sendmsg = PACKET_CREATE(fish_protocols::packetl2c_bc_leave_seat, fish_protocols::e_mst_l2c_bc_leave_seat);
	//sendmsg->set_player_id(player_id);

	//add_msg_to_list(sendmsg);
}

//////////////////////////////////////////////////////////////////////////
void logic_table::create_table()
{
	TotalIncome->set_value(0);
	TotalOutlay->set_value(0);
}
bool logic_table::load_table()
{
	/*mongo::BSONObj b = db_game::instance().findone(DB_FISHTABLE, BSON("room_id"<<m_room->get_id() << "table_id" << TableID->get_value()));
	if(b.isEmpty())
		return false;

	return from_bson(b);*/

	return false;
}
void logic_table::init_game_object()
{
	TableID = CONVERT_POINT(Tfield<int16_t>, regedit_tfield(e_got_int16, "table_id"));
	TotalIncome = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "TotalIncome"));
	TotalOutlay = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "TotalOutlay"));
}
bool logic_table::store_game_object(bool to_all)
{
	if(!has_update())
		return true;

	//auto err = db_game::instance().update(DB_FISHTABLE, BSON("room_id"<<m_room->get_id() << "table_id" << TableID->get_value()), BSON("$set"<<to_bson(to_all)));
	//if(!err.empty())
	//{
	//	SLOG_ERROR << "logic_table::store_game_object :" <<err;
	//	return false;
	//}

	m_checksave = 0;
	return true;
}

void logic_table::add_income(int score)
{
	TotalIncome->add_value(score);
	get_room()->TotalIncome->add_value(score);
}
void logic_table::add_outlay(int score)
{
	TotalOutlay->add_value(score);
	get_room()->TotalOutlay->add_value(score);
}

