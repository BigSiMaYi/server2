#include "stdafx.h"
#include "proc_cows_protocol.h"
#include "proc_cows_logic.h"
#include <i_game_player.h>
#include "logic_player.h"
#include "logic_room.h"
#include "game_engine.h"
#include "logic_main.h"

COWS_SPACE_USING
using namespace boost;

void init_proc_cows_protocol()
{
	packetc2l_get_room_info_factory::regedit_factory();
	packetl2c_get_room_info_result_factory::regedit_factory();
	packetc2l_enter_table_factory::regedit_factory();
	packetl2c_enter_table_result_factory::regedit_factory();
	packetc2l_leave_table_factory::regedit_factory();
	packetl2c_leave_table_result_factory::regedit_factory();
	packetc2l_check_state_factory::regedit_factory();
	packetl2c_check_state_result_factory::regedit_factory();
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
	int i=0;
	for (auto it = rooms.begin(); it != rooms.end(); ++it)
	{
		auto r = sendmsg->add_rooms();
		r->set_roomid(it->second->get_id());
		int nPlayerCnt = it->second->get_cur_cout();
		r->set_playercnt(nPlayerCnt);
		i++;
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
	player->send_msg_to_client(sendmsg);

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//退出桌子
bool packetc2l_leave_table_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
													 shared_ptr<packetc2l_leave_table> msg)
{	
	__ENTER_FUNCTION_CHECK;

	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	int ret = lcplayer->can_leave_room();
	if (ret==0)
	{
		lcplayer->leave_room();
	}

	auto sendmsg = PACKET_CREATE(packetl2c_leave_table_result, e_mst_l2c_leave_table_result);
	sendmsg->set_sync_gold(lcplayer->get_gold());
	if (ret==0)
	{
		sendmsg->set_result(msg_type_def::e_msg_result_def::e_rmt_success);
	}
	else if (ret == 53)
	{
		sendmsg->set_result(msg_type_def::e_msg_result_def::e_rmt_now_is_banker);
	}
	else if (ret == 2)
	{
		sendmsg->set_result(msg_type_def::e_msg_result_def::e_rmt_can_not_leave);
	}
	
	player->send_msg_to_client(sendmsg);

	__LEAVE_FUNCTION_CHECK
	return !EX_CHECK;
}

bool packetc2l_check_state_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
												   shared_ptr<packetc2l_check_state> msg)
{	
	__ENTER_FUNCTION_CHECK;

	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	auto sendmsg = PACKET_CREATE(packetl2c_check_state_result, e_mst_l2c_check_state_result);
	sendmsg->set_is_intable(lcplayer->get_room() != nullptr);	

	player->send_msg_to_client(sendmsg);

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool msg_home_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player,
	shared_ptr<msg_home> msg)
{
	__ENTER_FUNCTION_CHECK;

	auto lcplayer = CONVERT_POINT(logic_player, player->get_handler());
	if (lcplayer)
	{
		lcplayer->set_kick_status(6);
		auto room = lcplayer->get_room();
		if (room)
		{
			auto game_main = room->get_game_main();
			if (game_main)
			{
				int32_t cost_ticket = 0;
				game_main->ask_leave_banker(lcplayer, false, cost_ticket);
				game_main->cancel_apply_banker(lcplayer);
			}
		}
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}