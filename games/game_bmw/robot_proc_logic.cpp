#include "stdafx.h"
#include "robot_def.h"
#include "robot_mgr.h"
#include "robot_player.h"

#include "game_engine.h"
#include "logic_lobby.h"

#include "bmw_logic.pb.h"
#include "robot_proc_logic.h"

BMW_SPACE_USING

using namespace boost;

void init_robot_proc_logic()
{
	packetl2c_robot_enter_rfactory::regedit_factory();
	packetl2c_robot_leave_rfactory::regedit_factory();


	packetl2c_scene_info_free_rfactory::regedit_factory();
	packetl2c_scene_info_play_rfactory::regedit_factory();

	packetl2c_ask_ready_result_rfactory::regedit_factory();

	packetl2c_change_banker_rfactory::regedit_factory();

	packetl2c_ask_apply_banker_result_rfactory::regedit_factory();

	packetl2c_ask_unapply_banker_result_rfactory::regedit_factory();

	packetl2c_notice_start_rfactory::regedit_factory();

	packetl2c_ask_place_bet_result_rfactory::regedit_factory();

	packetl2c_notice_placefull_rfactory::regedit_factory();

	packetl2c_run_result_rfactory::regedit_factory();

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
		tagBankerInfo info;
		memset(&info,0,sizeof(info));
		auto free = msg->freeinfo();
		auto bankinfo = free.bankerinfo();
		info.uid = bankinfo.id();
		info.money = bankinfo.money();
		info.round = bankinfo.round();
		info.result = bankinfo.result();
		std::list<uint32_t> blist;

		int nSize = free.bankerlist_size();
		for(int i=0;i< nSize; i++)
		{
			uint32_t uid = free.bankerlist(i);
			blist.push_back(uid);
		}
		// 当前庄 上庄列表
		player->initGame_free(info,blist);
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
	
		auto free = msg->playinfo();
		// 当前游戏状态
		int32_t nGameState = free.status();
		int32_t nLeftTime  = free.lefttime();
		// 当前下注信息
		AreaInfo area;
		memset(&area,0,sizeof(area));
		int nChipSize = free.arearinfo_size();
		for (int i=0;i<nChipSize;i++)
		{
			auto areainfo = free.arearinfo(i);
			int32_t idx = areainfo.areaid();
			if(idx==PLACE_AREA) continue;
			area.anayone[idx] = areainfo.areamoney();
			area.total[idx] = areainfo.areatotal();
		}
		// 当前庄
		tagBankerInfo info;
		memset(&info,0,sizeof(info));
		auto bankinfo = free.bankerinfo();
		info.uid = bankinfo.id();
		info.money = bankinfo.money();
		info.round = bankinfo.round();
		info.result = bankinfo.result();
		// 上庄列表
		std::list<uint32_t> blist;
		int nSize = free.bankerlist_size();
		for(int i=0;i< nSize; i++)
		{
			uint32_t uid = free.bankerlist(i);
			blist.push_back(uid);
		}
		// 当前庄 上庄列表
		player->initGame_play(nGameState,nLeftTime,info,area,blist);
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

bool packetl2c_change_banker_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_change_banker> msg)
{	
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{ // 当前庄信息
		tagBankerInfo info;
		memset(&info,0,sizeof(info));
		auto banker = msg->bankerinfo();
		info.uid = banker.id();
		info.money = banker.money();
		info.round = banker.round();
		info.result = banker.result();

		player->onEventNoticeBanker(info);
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_notice_start_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_notice_start> msg)
{	
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{ // 开始下注
		
		player->onEventBeginPlace();

	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

// 上庄结果
bool packetl2c_ask_apply_banker_result_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_ask_apply_banker_result> msg)
{	
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{ // 
		uint32_t nBanker = msg->bankid();
		// 上庄列表
		std::list<uint32_t> blist;
		int nSize = msg->banklist_size();
		for(int i=0;i< nSize; i++)
		{
			uint32_t uid = msg->banklist(i);
			blist.push_back(uid);
		}

		player->onEventApplyBanker(nBanker,blist);
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

// 下庄结果
bool packetl2c_ask_unapply_banker_result_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_ask_unapply_banker_result> msg)
{	
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{ // 
		uint32_t nBanker = msg->bankid();
		// 上庄列表
		std::list<uint32_t> blist;
		int nSize = msg->banklist_size();
		for(int i=0;i< nSize; i++)
		{
			uint32_t uid = msg->banklist(i);
			blist.push_back(uid);
		}
		player->onEventUnApplyBanker(nBanker,blist);
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

// 下注结果
bool packetl2c_ask_place_bet_result_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_ask_place_bet_result> msg)
{	
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{ // 
		int32_t nRet = msg->betres();
		if(nRet==1)
		{
			player->onEventPlace(msg->betid(),msg->betarea(),msg->betvalue());
		}
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}
// 下注 已满
bool packetl2c_notice_placefull_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_notice_placefull> msg)
{	
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{ // 
		player->onEventNoticeFull();
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}
// 
bool packetl2c_run_result_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_run_result> msg)
{	
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{ // 停止下注

		player->onEventNoticeRun(msg->areaid());

	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

// 结算数据
bool packetl2c_game_result_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_game_result> msg)
{
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{ 
		player->onEventEnd(player_id,0);
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}