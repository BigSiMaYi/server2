#include "stdafx.h"
#include "robot_def.h"
#include "robot_mgr.h"
#include "robot_player.h"

#include "game_engine.h"
#include "logic_lobby.h"

#include "landlord3_logic.pb.h"
#include "robot_proc_logic.h"

LANDLORD_SPACE_USING

using namespace boost;

void init_robot_proc_logic()
{
	packetl2c_robot_enter_rfactory::regedit_factory();
	packetl2c_robot_leave_rfactory::regedit_factory();


	packetl2c_scene_info_free_rfactory::regedit_factory();
	packetl2c_scene_info_play_rfactory::regedit_factory();

	packetl2c_ask_ready_result_rfactory::regedit_factory();
	packetl2c_game_start_rfactory::regedit_factory();


	packetl2c_game_result_rfactory::regedit_factory();

    packetl2c_notice_robbanker_rfactory::regedit_factory();
}

//机器人进入
bool packetl2c_robot_enter_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_robot_enter> msg)
{	
	__ENTER_FUNCTION_CHECK;

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
	else
	{
		SLOG_CRITICAL << "robot id:"<< player_id << "not found";
		game_engine::instance().release_robot(player_id);
	}

	SLOG_CRITICAL<<"[COUNTER]Robot Counter:"<<robot_mgr::instance().get_robot_count() << ", enter robot id: "<< player_id;


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
	SLOG_CRITICAL<<"[COUNTER]Robot Counter:"<<robot_mgr::instance().get_robot_count()<<", leave robot id: " << player_id;
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

// 游戏开始了
bool packetl2c_game_start_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_game_start> msg)
{	
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		int64_t lUserGold =0;
		int64_t lBaseGold = msg->basegold();
		uint32_t bankerid = 0;
		int64_t lPool = 0;
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
		player->onEventGameStart(tableUser,lUserGold,lBaseGold,bankerid,lPool);
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
		player->onEventEnd(msg->winid(), 0);
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

    }

    __LEAVE_FUNCTION_CHECK
        return !EX_CHECK;
}