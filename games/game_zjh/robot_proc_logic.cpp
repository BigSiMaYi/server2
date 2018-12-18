#include "stdafx.h"
#include "robot_def.h"
#include "robot_mgr.h"
#include "robot_player.h"

#include "game_engine.h"
#include "logic_lobby.h"

#include "zjh_logic.pb.h"
#include "robot_proc_logic.h"

ZJH_SPACE_USING

using namespace boost;

void init_robot_proc_logic()
{
	packetl2c_robot_gameinfo_rfactory::regedit_factory();
	packetl2c_robot_enter_rfactory::regedit_factory();
	packetl2c_robot_leave_rfactory::regedit_factory();


	packetl2c_scene_info_free_rfactory::regedit_factory();
	packetl2c_scene_info_play_rfactory::regedit_factory();

	packetl2c_ask_ready_result_rfactory::regedit_factory();
	packetl2c_game_start_rfactory::regedit_factory();
	packetl2c_notice_start_rfactory::regedit_factory();

	packetl2c_operator_kan_rfactory::regedit_factory();
	packetl2c_operator_gen_rfactory::regedit_factory();
	packetl2c_operator_jia_rfactory::regedit_factory();
	packetl2c_operator_bi_rfactory::regedit_factory();
	packetl2c_operator_qi_rfactory::regedit_factory();
	packetl2c_ask_operator_showhand_result_rfactory::regedit_factory();
	packetl2c_operator_showhand_rfactory::regedit_factory();

	packetl2c_notice_allin_rfactory::regedit_factory();

	packetl2c_operator_allin_rfactory::regedit_factory();

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
		int64_t lUserGold = msg->usergold();
		int64_t lBaseGold = msg->basegold();
		uint32_t bankerid = msg->bankerid();
		int64_t lPool = msg->pool_gold();
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

bool packetl2c_robot_gameinfo_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_robot_gameinfo> msg)
{
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		int nSize = msg->playercard_size();
		for (int32_t i=0;i<nSize ; i++)
		{
			auto info = msg->playercard(i);
			auto uid = info.playerid();
			auto cardinfo = info.cardinfo();
			GameCard card;
			memset(&card, 0, sizeof(card));
			int nSize = cardinfo.pokers_size();
			for (int i = 0; i < nSize; i++)
			{
				int hex = cardinfo.pokers(i);
				
				card.nCard[i] = hex;
			}
			card.nModel = cardinfo.pokertype();
			if (cardinfo.has_pokerstatus())
			{
				card.bCheck = cardinfo.pokerstatus();
			}
			//
			player->onGetGameInfo(uid, card);
		}
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

// 游戏开始了
bool packetl2c_notice_start_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_notice_start> msg)
{	
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		player->onEventNoticeStart(msg->playerid());
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_operator_kan_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_operator_kan> msg)
{
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		uint32_t uid = msg->playerid();
		CARDLEVEL clvl;
		memset(&clvl,0,sizeof(clvl));
		if( msg->has_cardinfo())
		{
			auto cardinfo = msg->cardinfo();
			int nSize = cardinfo.pokers_size();
			for (int i=0;i<nSize;i++)
			{
				int hex =  cardinfo.pokers(i);
				clvl.nCard[i] = hex;
			}

			clvl.nModel = cardinfo.pokertype();
		}
		else
		{

		}
		player->onEventCheck(uid,clvl);
	
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_operator_gen_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_operator_gen> msg)
{
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		player->onEventFlow(msg->playerid(),msg->nextid(),
			msg->operator_gold(), msg->round(), msg->pool_gold());
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_operator_jia_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_operator_jia> msg)
{
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		int64_t lCur_Jetton = msg->cur_jetton();
		uint32_t nextid = 0;
		int32_t nRound = 0;
		if(msg->has_nextid())
			nextid = msg->nextid();
		if(msg->has_round())
			nRound = msg->round();
		player->onEventAdd(msg->playerid(),nextid,lCur_Jetton,msg->operator_gold(),nRound,msg->pool_gold());
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_operator_bi_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_operator_bi> msg)
{
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		uint32_t reqid = msg->requestid();
		uint32_t resid = msg->compareid();
		uint32_t winid = msg->winid();
		uint32_t nextid = 0;
		int32_t nRound = 0;
		if(msg->has_nextid())
			nextid = msg->nextid();
		if(msg->has_round())
			nRound = msg->round();

		player->onEventCompare(reqid,msg->operator_gold(),resid,winid,nextid,nRound,msg->pool_gold());
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_operator_qi_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_operator_qi> msg)
{
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		uint32_t uid = msg->playerid();
		uint32_t nextid = 0;
		if(msg->has_nextid())
			nextid = msg->nextid();
		int32_t nRound = 0;
		if(msg->has_round())
			nRound = msg->round();
		player->onEventGiveUP(uid,nextid,nRound);
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_ask_operator_showhand_result_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_ask_operator_showhand_result> msg)
{
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{

		player->onEventShowHand(msg->playerid(),msg->nextid(),msg->operator_gold(),msg->pool_gold());
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_operator_showhand_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_operator_showhand> msg)
{
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{

	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_notice_allin_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_notice_allin> msg)
{
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{

		player->onEventNoticeAllIn(msg->playerid());
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool packetl2c_operator_allin_rfactory::packet_process(uint32_t player_id, shared_ptr<packetl2c_operator_allin> msg)
{
	__ENTER_FUNCTION_CHECK;

	RobotPlayerPtr& player = robot_mgr::instance().get_robot_player(player_id);
	if (player != nullptr)
	{
		uint32_t uid = msg->playerid();

		uint32_t nextid = 0;
		if(msg->has_nextid())
			nextid = msg->nextid();
		int32_t nRound = 0;
		if(msg->has_round())
			nRound = msg->round();
		player->onEventAllIn(uid,msg->bwin(),msg->operator_gold(),msg->pool_gold(),nextid,nRound);

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
		player->onEventEnd(msg->winid(),msg->endtype());
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}