#include "stdafx.h"
#include "proc_bmw_logic.h"
#include <i_game_player.h>
#include "logic_player.h"
#include "logic_table.h"

#include "logic_room.h"


#include "game_engine.h"


BMW_SPACE_USING
using namespace boost;

void init_proc_bmw_logic()
{
	packetc2l_get_scene_info_factory::regedit_factory();
	packetl2c_scene_info_free_factory::regedit_factory();
	packetl2c_scene_info_play_factory::regedit_factory();

	packetc2l_ask_ready_factory::regedit_factory();
	packetl2c_ask_ready_result_factory::regedit_factory();

	packetc2l_ask_place_bet_factory::regedit_factory();
	packetl2c_ask_place_bet_result_factory::regedit_factory();

	packetc2l_ask_place_again_factory::regedit_factory();
	packetl2c_ask_place_again_result_factory::regedit_factory();

	packetc2l_ask_apply_banker_factory::regedit_factory();
	packetl2c_ask_apply_banker_result_factory::regedit_factory();

	packetc2l_ask_unapply_banker_factory::regedit_factory();
	packetl2c_ask_unapply_banker_result_factory::regedit_factory();

	packetc2l_ask_history_factory::regedit_factory();
	packetl2c_respone_history_factory::regedit_factory();

	packetl2c_change_banker_factory::regedit_factory();
	packetl2c_run_result_factory::regedit_factory();

	packetl2c_game_result_factory::regedit_factory();
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////

	packetl2c_user_enter_seat_factory::regedit_factory();
	packetl2c_user_leave_seat_factory::regedit_factory();


	packetl2c_notice_start_factory::regedit_factory();

	packetl2c_robot_enter_factory::regedit_factory();
	packetl2c_robot_leave_factory::regedit_factory();

	packetl2c_clean_out_factory::regedit_factory();
}

//获取游戏信息
bool packetc2l_get_scene_info_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
													 shared_ptr<packetc2l_get_scene_info> msg)
{	
	__ENTER_FUNCTION_CHECK;
	// 发送游戏场景消息
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_get_scene_info_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_get_scene_info_factory get table err!";
		return false;
	}
	uint32_t uid = lcplayer->get_pid();
	lptable->req_scene(uid);
	int8_t status = lptable->get_status();
	switch (status)
	{
	case eGameState_Free:
		{
			auto sendmsg = lcplayer->get_table()->get_scene_info_msg_free();
			if(lcplayer->is_robot())
			{
				robot_mgr::instance().recv_packet(uid, sendmsg->packet_id(), sendmsg);
			}
			else
			{
				player->send_msg_to_client(sendmsg);
			}
			
		}
		break;
	case eGameState_Place:
	case eGameState_Play:
	case eGameState_End:
		{
			auto sendmsg = lcplayer->get_table()->get_scene_info_msg_play(uid);
			if(lcplayer->is_robot())
			{
				robot_mgr::instance().recv_packet(uid, sendmsg->packet_id(), sendmsg);
			}
			else
			{
				player->send_msg_to_client(sendmsg);
			}
		}
		break;
	default:
		break;
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//请求准备
bool packetc2l_ask_ready_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
													shared_ptr<packetc2l_ask_ready> msg)
{
	__ENTER_FUNCTION_CHECK
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_ask_ready_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_ask_ready_factory get table err!";
		return false;
	}
	lcplayer->onEventUserReady();
	
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//请求下注
bool packetc2l_ask_place_bet_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
												 shared_ptr<packetc2l_ask_place_bet> msg)
{
	__ENTER_FUNCTION_CHECK
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_ask_place_bet_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_ask_place_bet_factory get table err!";
		return false;
	}
	
	lptable->onGameBetArea(lcplayer->get_pid(),msg->betarea(),msg->betvalue());

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

// 续投
bool packetc2l_ask_place_again_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
													 shared_ptr<packetc2l_ask_place_again> msg)
{
	__ENTER_FUNCTION_CHECK
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_ask_place_bet_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_ask_place_bet_factory get table err!";
		return false;
	}

	lptable->onGamePlaceAgain(lcplayer->get_pid());

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}
//请求上庄
bool packetc2l_ask_apply_banker_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
													 shared_ptr<packetc2l_ask_apply_banker> msg)
{
	__ENTER_FUNCTION_CHECK
		auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_ask_apply_banker_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_ask_apply_banker_factory get table err!";
		return false;
	}

	lptable->onGameApplyBanker(lcplayer->get_pid());

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//请求下庄
bool packetc2l_ask_unapply_banker_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
														shared_ptr<packetc2l_ask_unapply_banker> msg)
{
	__ENTER_FUNCTION_CHECK
		auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_ask_unapply_banker_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_ask_unapply_banker_factory get table err!";
		return false;
	}

	lptable->onGameUnApplyBanker(lcplayer->get_pid());

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetc2l_ask_history_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
														  shared_ptr<packetc2l_ask_history> msg)
{
	__ENTER_FUNCTION_CHECK
		auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_ask_unapply_banker_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_ask_unapply_banker_factory get table err!";
		return false;
	}

	lptable->onGameRequestHistory(lcplayer->get_pid());

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool my_greater(LPlayerPtr& a, LPlayerPtr& b)
{
	return a->get_gold() > b->get_gold();
}

