#include "stdafx.h"
#include "proc_example_logic.h"
#include "game_engine.h"
#include <i_game_player.h>
#include "logic_table.h"
#include "logic_player.h"

EXAMPLE_SPACE_USING
	using namespace boost;

void init_proc_example_logic()
{
	packetc2l_game_play_factory::regedit_factory();
	packetl2c_game_play_result_factory::regedit_factory();
}

//获取游戏信息
bool packetc2l_game_play_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
													 shared_ptr<packetc2l_game_play> msg)
{	
	__ENTER_FUNCTION_CHECK;

	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(lcplayer && lcplayer->get_table())
	{
		auto sendmsg = PACKET_CREATE(packetl2c_game_play_result, e_mst_l2c_game_play_result);
		sendmsg->set_random(0);
		player->send_msg_to_client(sendmsg);	
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}
