#include "stdafx.h"
#include "proc_landlord_protocol.h"
#include "proc_landlord_logic.h"
#include <i_game_player.h>
#include "logic_player.h"
#include "logic_table.h"
#include "logic_room.h"
#include "game_engine.h"

LANDLORD_SPACE_USING
using namespace boost;

void init_proc_landlord_protocol()
{
	packetc2l_get_room_info_factory::regedit_factory();
	packetl2c_get_room_info_result_factory::regedit_factory();
	packetc2l_enter_table_factory::regedit_factory();
	packetl2c_enter_table_result_factory::regedit_factory();
	packetc2l_leave_table_factory::regedit_factory();
	packetl2c_leave_table_result_factory::regedit_factory();
	packetc2l_check_state_factory::regedit_factory();
	packetl2c_check_state_result_factory::regedit_factory();
	packetc2l_change_table_factory::regedit_factory();
	packetl2c_change_table_result_factory::regedit_factory();

	msg_home_factory::regedit_factory();
}

//获取游戏信息
bool packetc2l_get_room_info_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
												 shared_ptr<packetc2l_get_room_info> msg)
{	
	__ENTER_FUNCTION_CHECK;

	auto sendmsg = PACKET_CREATE(packetl2c_get_room_info_result, e_mst_l2c_get_room_info_result);

	auto rooms = game_engine::instance().get_lobby().get_rooms();
	sendmsg->mutable_rooms()->Reserve(rooms.size());
	for (auto it = rooms.begin(); it != rooms.end(); ++it)
	{
		auto r = sendmsg->add_rooms();
		r->set_roomid(it->second->get_id());
	}

	player->send_msg_to_client(sendmsg);

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//进入桌子
bool packetc2l_enter_table_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
													 shared_ptr<packetc2l_enter_table> msg)
{	
	__ENTER_FUNCTION_CHECK;
	int ret = game_engine::instance().get_lobby().enter_room(player->get_playerid(), msg->roomid());
	auto sendmsg = PACKET_CREATE(packetl2c_enter_table_result, e_mst_l2c_enter_table_result);
	sendmsg->set_result((msg_type_def::e_msg_result_def)ret);
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	sendmsg->set_chairid(lcplayer->get_seat());
	player->send_msg_to_client(sendmsg);
	if (ret != 1)
	{
		if (lcplayer->can_leave_table())
		{
			player->reqPlayer_leaveGame();
			SLOG_CRITICAL << "packetc2l_enter_table reqPlayer_leaveGame player id: " << player->get_playerid();
		}
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//退出桌子
bool packetc2l_leave_table_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
													 shared_ptr<packetc2l_leave_table> msg)
{	
	__ENTER_FUNCTION_CHECK;

	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	bool ret = lcplayer->can_leave_table();

	auto sendmsg = PACKET_CREATE(packetl2c_leave_table_result, e_mst_l2c_leave_table_result);
	sendmsg->set_sync_gold(lcplayer->get_gold());
	if (ret)
	{
		sendmsg->set_result(msg_type_def::e_msg_result_def::e_rmt_success);
	}
	else
	{
		sendmsg->set_result(msg_type_def::e_msg_result_def::e_rmt_game_begun);
	}
	
	player->send_msg_to_client(sendmsg);
	if (ret)
	{
		//lcplayer->leave_table();
		lcplayer->reqPlayer_leaveGame();
		SLOG_CRITICAL << "packetc2l_leave_table reqPlayer_leaveGame player id: " << player->get_playerid();
	}
	__LEAVE_FUNCTION_CHECK
	return !EX_CHECK;
}

bool packetc2l_check_state_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
												   shared_ptr<packetc2l_check_state> msg)
{	
	__ENTER_FUNCTION_CHECK;

	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	auto sendmsg = PACKET_CREATE(packetl2c_check_state_result, e_mst_l2c_check_state_result);
	if (lcplayer)
	{
		auto status = lcplayer->get_status();
		bool st = (lcplayer->get_table() != nullptr);

		SLOG_CRITICAL << "packetc2l_check_state, playerid:" << lcplayer->get_pid() <<", status:"<< status <<", is intable:" << st;
	}
	sendmsg->set_is_intable(lcplayer->get_table() != nullptr);	

	player->send_msg_to_client(sendmsg);


	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetc2l_change_table_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
												   shared_ptr<packetc2l_change_table> msg)
{	
	__ENTER_FUNCTION_CHECK;
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	auto sendmsg = PACKET_CREATE(packetl2c_change_table_result, e_mst_l2c_change_table_result);
	if(lcplayer->get_table() != nullptr)
	{
		uint16_t curtableid = lcplayer->get_table()->get_id();
		auto troom = lcplayer->get_table()->get_room();

		if(troom->has_seat(lcplayer, curtableid, lcplayer->is_robot()))
		{
			bool bCanLeave = lcplayer->can_leave_table();
			int ret = 1;
			if(bCanLeave)
			{
				lcplayer->leave_table();
				ret = troom->enter_table(lcplayer, curtableid);
			}
			else
			{
				ret = 2;
			}
			sendmsg->set_result((msg_type_def::e_msg_result_def)ret);
		}		
	}

	player->send_msg_to_client(sendmsg);

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}


bool msg_home_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player,
	shared_ptr<msg_home> msg)
{
	__ENTER_FUNCTION_CHECK;
	auto lcplayer = CONVERT_POINT(logic_player, player->get_handler());
	if (!lcplayer)
	{
		SLOG_ERROR << "msg_home_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if (!lptable)
	{
		SLOG_ERROR << "msg_home_factory get table err!";
		return false;
	}
	//lptable->onEventAuto(lcplayer->get_pid(), true);

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}