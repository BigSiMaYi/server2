#include "stdafx.h"
#include "proc_zjh_logic.h"
#include <i_game_player.h>
#include "logic_player.h"
#include "logic_table.h"

#include "logic_room.h"

#include "game_engine.h"


ZJH_SPACE_USING
using namespace boost;

void init_proc_zjh_logic()
{
	packetc2l_get_scene_info_factory::regedit_factory();
	packetl2c_scene_info_free_factory::regedit_factory();
	packetl2c_scene_info_play_factory::regedit_factory();

	packetc2l_ask_ready_factory::regedit_factory();
	packetl2c_ask_ready_result_factory::regedit_factory();

	packetc2l_ask_operator_gen_factory::regedit_factory();
	packetl2c_operator_gen_factory::regedit_factory();

	packetc2l_ask_operator_kan_factory::regedit_factory();
	packetl2c_operator_kan_factory::regedit_factory();

	packetc2l_ask_operator_qi_factory::regedit_factory();
	packetl2c_operator_qi_factory::regedit_factory();

	packetc2l_ask_operator_jia_factory::regedit_factory();
	packetl2c_operator_jia_factory::regedit_factory();

	packetc2l_ask_operator_bi_factory::regedit_factory();
	packetl2c_operator_bi_factory::regedit_factory();

	packetc2l_ask_operator_showhand_factory::regedit_factory();
	packetl2c_ask_operator_showhand_result_factory::regedit_factory();
	packetl2c_operator_showhand_factory::regedit_factory();

	packetl2c_notice_allin_factory::regedit_factory();
	packetc2l_ask_operator_allin_factory::regedit_factory();
	packetl2c_operator_allin_factory::regedit_factory();
	packetc2l_ask_operator_pk_factory::regedit_factory();

	packetl2c_all_pk_factory::regedit_factory();

	packetc2l_let_see_factory::regedit_factory();
	packetl2c_let_see_result_factory::regedit_factory();

	packetl2c_open_card_factory::regedit_factory();
	packetl2c_game_result_factory::regedit_factory();
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////

	packetl2c_user_enter_seat_factory::regedit_factory();
	packetl2c_user_leave_seat_factory::regedit_factory();

	packetl2c_game_start_factory::regedit_factory();

	packetl2c_notice_sparetime_factory::regedit_factory();

	packetl2c_notice_start_factory::regedit_factory();

	packetl2c_robot_gameinfo_factory::regedit_factory();
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
	case eGameState_Spare:
	case eGameState_End:
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
	case eGameState_FaPai:
	case eGameState_Play:
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
// 跟注
bool packetc2l_ask_operator_gen_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
												 shared_ptr<packetc2l_ask_operator_gen> msg)
{
	__ENTER_FUNCTION_CHECK
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_ask_operator_gen_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_ask_operator_gen_factory get table err!";
		return false;
	}
	int64_t lgold = msg->operator_gold();
	lptable->onGameFlow(lcplayer->get_pid(),lgold);

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}
// 看牌
bool packetc2l_ask_operator_kan_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
														shared_ptr<packetc2l_ask_operator_kan> msg)
{
	__ENTER_FUNCTION_CHECK
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_ask_operator_kan_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_ask_operator_kan_factory get table err!";
		return false;
	}
	lptable->onGameCheck(lcplayer->get_pid());

	__LEAVE_FUNCTION_CHECK
	return !EX_CHECK;
}

// 弃牌
bool packetc2l_ask_operator_qi_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
														shared_ptr<packetc2l_ask_operator_qi> msg)
{
	__ENTER_FUNCTION_CHECK
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_ask_operator_qi_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_ask_operator_qi_factory get table err!";
		return false;
	}
	lptable->onGameGiveUp(lcplayer->get_pid());

	__LEAVE_FUNCTION_CHECK
	return !EX_CHECK;

}

// 加注
bool packetc2l_ask_operator_jia_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
													   shared_ptr<packetc2l_ask_operator_jia> msg)
{
	__ENTER_FUNCTION_CHECK
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_ask_operator_jia_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_ask_operator_jia_factory get table err!";
		return false;
	}
	int64_t lAddGold = msg->operator_gold();
	lptable->onGameAdd(lcplayer->get_pid(),lAddGold);

	__LEAVE_FUNCTION_CHECK
	return !EX_CHECK;
}

// 比牌
bool packetc2l_ask_operator_bi_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
													   shared_ptr<packetc2l_ask_operator_bi> msg)
{
	__ENTER_FUNCTION_CHECK
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_ask_operator_bi_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_ask_operator_bi_factory get table err!";
		return false;
	}
	uint32_t reqid = lcplayer->get_pid();
	uint32_t resid = msg->compareid();
	lptable->onGameCompare(reqid,resid);

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

// 梭哈
bool packetc2l_ask_operator_showhand_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
													   shared_ptr<packetc2l_ask_operator_showhand> msg)
{
	__ENTER_FUNCTION_CHECK
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_ask_operator_showhand_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_ask_operator_showhand_factory get table err!";
		return false;
	}
	lptable->onGameShowHand(lcplayer->get_pid());

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

// 孤掷一注
bool packetc2l_ask_operator_allin_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
															 shared_ptr<packetc2l_ask_operator_allin> msg)
{
	__ENTER_FUNCTION_CHECK
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_ask_operator_allin_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_ask_operator_allin_factory get table err!";
		return false;
	}
	lptable->onGameAllIn(lcplayer->get_pid());

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

// 金币不足比牌
bool packetc2l_ask_operator_pk_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
														  shared_ptr<packetc2l_ask_operator_pk> msg)
{
	__ENTER_FUNCTION_CHECK
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_ask_operator_pk_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_ask_operator_pk_factory get table err!";
		return false;
	}
	lptable->onGameCompareEx(lcplayer->get_pid());

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}
// 允许看牌
bool packetc2l_let_see_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
														  shared_ptr<packetc2l_let_see> msg)
{
	__ENTER_FUNCTION_CHECK
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(!lcplayer) 
	{
		SLOG_ERROR<<"packetc2l_ask_operator_allin_factory get user err!";
		return false;
	}
	auto lptable = lcplayer->get_table();
	if(!lptable)
	{
		SLOG_ERROR<<"packetc2l_ask_operator_allin_factory get table err!";
		return false;
	}

	lptable->onGameAllowToSee(lcplayer->get_pid());

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}



bool my_greater(LPlayerPtr& a, LPlayerPtr& b)
{
	return a->get_gold() > b->get_gold();
}

