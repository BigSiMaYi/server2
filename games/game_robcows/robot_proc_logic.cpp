#include "stdafx.h"
#include "robot_def.h"
#include "robot_mgr.h"
#include "robot_player.h"

#include "game_engine.h"
#include "logic_lobby.h"

#include "robcows_logic.pb.h"
#include "robot_proc_logic.h"

ROBCOWS_SPACE_USING

using namespace boost;

void init_robot_proc_logic()
{
	packetl2c_robot_enter_rfactory::regedit_factory();
	packetl2c_robot_leave_rfactory::regedit_factory();

	packetl2c_ask_ready_result_rfactory::regedit_factory();

	packetl2c_scene_info_free_rfactory::regedit_factory();
	packetl2c_scene_info_play_rfactory::regedit_factory();

	packetl2c_scene_info_play_bank_rfactory::regedit_factory();
	packetl2c_scene_info_play_bet_rfactory::regedit_factory();
	packetl2c_scene_info_play_opencard_rfactory::regedit_factory();
	packetl2c_scene_info_play_display_rfactory::regedit_factory();

	packetl2c_notice_sparetime_rfactory::regedit_factory();
	packetl2c_game_start_rfactory::regedit_factory();

	packetl2c_notice_robbanker_rfactory::regedit_factory();
	packetl2c_notice_bet_rfactory::regedit_factory();
	packetl2c_notice_opencard_rfactory::regedit_factory();
	packetl2c_fifth_card_rfactory::regedit_factory();
	packetl2c_game_result_rfactory::regedit_factory();
}

//机器人进入
bool packetl2c_robot_enter_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_robot_enter> msg)
{	
	__ENTER_FUNCTION_CHECK;
#if 0
	RobotPlayerPtr& rplayer = robot_mgr::instance().get_robot_player(player_id);
	auto& lobby = game_engine::instance().get_lobby();
	LPlayerPtr p = lobby.get_player(player_id);
	auto& player = lobby.get_player(player_id);
	if (rplayer == nullptr)
	{
		auto r_player = robot_player::malloc();
	//	auto playerinfo = msg->playerinfo();
		
		r_player->init(player,msg->roomid(),msg->tableid());
		// 加入管理器
		robot_mgr::instance().robot_player_enter(r_player);
		// 初始化场景
		r_player->initScene();
	}
#else
	RobotPlayerPtr& rplayer = robot_mgr::instance().get_robot_player(player_id);
	if (rplayer == nullptr)
	{
		auto r_player = robot_player::malloc();
		int32_t roomid  = msg->roomid();
		int32_t tableid = msg->tableid();
		auto playerinfo = msg->playerinfo();
		uint32_t pid = playerinfo.player_id();
		int64_t lgold = playerinfo.player_gold();
		int32_t seat = playerinfo.seat_index();

		r_player->init(pid,lgold,roomid,tableid,seat);
		// 加入管理器
		robot_mgr::instance().robot_player_enter(r_player);
		// 初始化场景
		r_player->initScene();
	}

	SLOG_CRITICAL<<"[COUNTER]Robot Counter:"<<robot_mgr::instance().get_robot_count();
#endif

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//机器人离开
bool packetl2c_robot_leave_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_robot_leave> msg)
{	
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		robot_mgr::instance().robot_player_leave(player_id);
	}
	SLOG_CRITICAL<<"[COUNTER]Robot Counter:"<<robot_mgr::instance().get_robot_count();
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

// 场景消息
bool packetl2c_scene_info_free_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_scene_info_free> msg)
{	
	__ENTER_FUNCTION_CHECK;
	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		int32_t spare = 0 ;
		spare = msg->freeinfo().sparetime();
		int nSize = msg->freeinfo().playerinfo_size();
		for (int i=0;i<nSize;i++)
		{
			auto p = msg->freeinfo().playerinfo(i);
			int seat = p.seat_index();
			uint32_t uid = p.player_id();
			player->initGame(seat,uid,spare);
		}
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_ask_ready_result_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_ask_ready_result> msg)
{	
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		player->onEventUserReady(msg->playerid());
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_scene_info_play_bank_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_scene_info_play_bank> msg)
{	
	__ENTER_FUNCTION_CHECK;
	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		int nSize = msg->playinfo().playerinfo_size();
		for (int i=0;i<nSize;i++)
		{
			auto p = msg->playinfo().playerinfo(i);
			uint32_t uid = p.player_id();
			int seat = p.seat_index();
			int state = p.player_status();
			player->initGame_play(seat,uid,state);
		}
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_scene_info_play_bet_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_scene_info_play_bet> msg)
{	
	__ENTER_FUNCTION_CHECK;
	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		int nSize = msg->playinfo().playerinfo_size();
		for (int i=0;i<nSize;i++)
		{
			auto p = msg->playinfo().playerinfo(i);
			uint32_t uid = p.player_id();
			int seat = p.seat_index();
			int state = p.player_status();
			player->initGame_play(seat,uid,state);
		}
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_scene_info_play_opencard_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_scene_info_play_opencard> msg)
{	
	__ENTER_FUNCTION_CHECK;
	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		int nSize = msg->playinfo().playerinfo_size();
		for (int i=0;i<nSize;i++)
		{
			auto p = msg->playinfo().playerinfo(i);
			uint32_t uid = p.player_id();
			int seat = p.seat_index();
			int state = p.player_status();
			player->initGame_play(seat,uid,state);
		}
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_scene_info_play_display_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_scene_info_play_display> msg)
{	
	__ENTER_FUNCTION_CHECK;
	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		int nSize = msg->playinfo().playerinfo_size();
		for (int i=0;i<nSize;i++)
		{
			auto p = msg->playinfo().playerinfo(i);
			uint32_t uid = p.player_id();
			int seat = p.seat_index();
			int state = p.player_status();
			player->initGame_play(seat,uid,state);
		}
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

// 场景消息
bool packetl2c_scene_info_play_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_scene_info_play> msg)
{	
	__ENTER_FUNCTION_CHECK;
	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		int nSize = msg->playinfo().playerinfo_size();
		for (int i=0;i<nSize;i++)
		{
			auto p = msg->playinfo().playerinfo(i);
			uint32_t uid = p.player_id();
			int seat = p.seat_index();
			int state = p.player_status();
			player->initGame_play(seat,uid,state);
		}
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_notice_sparetime_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_notice_sparetime> msg)
{	
	__ENTER_FUNCTION_CHECK;
	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		player->onTellSpare();
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_game_start_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_game_start> msg)
{
	__ENTER_FUNCTION_CHECK;
	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		TableUser usr;
		memset(&usr,0,sizeof(usr));
		std::map<uint32_t,TableUser> tableUser;
		// activeinfo
		int nActiveSize = msg->state_size();
		int nActiveIDSize = msg->stateid_size();
		assert(nActiveIDSize==nActiveSize);
		for (int i=0;i<nActiveSize;i++)
		{
			usr.p_id = msg->stateid(i);
			usr.p_active = msg->state(i);
			tableUser.insert(std::make_pair(usr.p_id,usr));
		}

		player->onTellStart(tableUser);
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;

}

bool packetl2c_notice_robbanker_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_notice_robbanker> msg)
{	
	__ENTER_FUNCTION_CHECK;
	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		TableUser usr;
		memset(&usr, 0, sizeof(usr));
		std::map<uint32_t, TableUser> tableUser;
		// activeinfo
		int nActiveSize = msg->state_size();
		int nActiveIDSize = msg->stateid_size();
		assert(nActiveIDSize == nActiveSize);
		for (int i = 0; i < nActiveSize; i++)
		{
			usr.p_id = msg->stateid(i);
			usr.p_active = msg->state(i);
			tableUser.insert(std::make_pair(usr.p_id, usr));
		}

		player->onTellRobBanker(tableUser);
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_notice_bet_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_notice_bet> msg)
{	
	__ENTER_FUNCTION_CHECK;
	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		player->onTellBetRate(msg->bankerid(),msg->robrate());
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_notice_opencard_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_notice_opencard> msg)
{	
	__ENTER_FUNCTION_CHECK;
	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		player->onTellOpenCard();
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_fifth_card_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_fifth_card> msg)
{
	__ENTER_FUNCTION_CHECK;
	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		player->onTellOpenCard();
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_game_result_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_game_result> msg)
{	
	__ENTER_FUNCTION_CHECK;
	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		player->onTellGameResult();
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}