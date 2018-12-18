#include "stdafx.h"
#include "proc_robcows_logic.h"
#include <i_game_player.h>
#include "logic_player.h"
#include "logic_table.h"

#include "logic_room.h"


#include "game_engine.h"


ROBCOWS_SPACE_USING
using namespace boost;

void init_proc_robcows_logic()
{
	packetc2l_get_scene_info_factory::regedit_factory();
	packetl2c_scene_info_free_factory::regedit_factory();
	packetl2c_scene_info_play_bank_factory::regedit_factory();
	packetl2c_scene_info_play_bet_factory::regedit_factory();
	packetl2c_scene_info_play_opencard_factory::regedit_factory();
	packetl2c_scene_info_play_display_factory::regedit_factory();

	packetl2c_scene_info_play_factory::regedit_factory();

	packetc2l_ask_ready_factory::regedit_factory();
	packetl2c_ask_ready_result_factory::regedit_factory();


	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////

	packetl2c_user_enter_seat_factory::regedit_factory();
	packetl2c_user_leave_seat_factory::regedit_factory();

	packetl2c_game_start_factory::regedit_factory();

	packetl2c_notice_sparetime_factory::regedit_factory();

	packetl2c_notice_robbanker_factory::regedit_factory();
	packetc2l_ask_robbank_factory::regedit_factory();
	packetl2c_robbank_result_factory::regedit_factory();

	packetl2c_notice_bet_factory::regedit_factory();
	packetc2l_ask_bet_factory::regedit_factory();
	packetl2c_bet_result_factory::regedit_factory();
	
	packetl2c_fifth_card_factory::regedit_factory();

	packetl2c_notice_opencard_factory::regedit_factory();
	packetc2l_ask_opencard_factory::regedit_factory();
	packetl2c_opencard_result_factory::regedit_factory();

	packetl2c_showcard_factory::regedit_factory();
	
	packetl2c_game_result_factory::regedit_factory();

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
	case eGameState_End:
	case eGameState_Spare:
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
	//case eGameState_FaPai:
	case eGameState_Banker:
		{
			auto sendmsg = lcplayer->get_table()->get_scene_info_msg_play_bank(uid);
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
	case eGameState_Bet:
		{
			auto sendmsg = lcplayer->get_table()->get_scene_info_msg_play_bet(uid);
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
	case eGameState_OpenCard:
		{
			auto sendmsg = lcplayer->get_table()->get_scene_info_msg_play_opencard(uid);
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
	case eGameState_Display:
		{
			auto sendmsg = lcplayer->get_table()->get_scene_info_msg_play_display();
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

// 请求抢庄
bool packetc2l_ask_robbank_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
												 shared_ptr<packetc2l_ask_robbank> msg)
{
	__ENTER_FUNCTION_CHECK
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_ask_robbank_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_ask_robbank_factory get table err!";
		return false;
	}
	lptable->onGameRobBanker(lcplayer->get_pid(),msg->robrate());

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

// 请求下注
bool packetc2l_ask_bet_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
												   shared_ptr<packetc2l_ask_bet> msg)
{
	__ENTER_FUNCTION_CHECK
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_ask_robbank_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_ask_robbank_factory get table err!";
		return false;
	}
	lptable->onGameBetRate(lcplayer->get_pid(),msg->betrate());

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

// 请求亮牌
bool packetc2l_ask_opencard_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
											   shared_ptr<packetc2l_ask_opencard> msg)
{
	__ENTER_FUNCTION_CHECK
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_ask_opencard_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_ask_opencard_factory get table err!";
		return false;
	}
	lptable->onGameOpenCard(lcplayer->get_pid());

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool my_greater(LPlayerPtr& a, LPlayerPtr& b)
{
	return a->get_gold() > b->get_gold();
}

