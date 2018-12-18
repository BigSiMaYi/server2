#include "stdafx.h"
#include "game_engine.h"
#include <i_game_engine.h>
#include <i_game_ehandler.h>
#include "logic_lobby.h"
#include "game_db.h"
#include "game_db_log.h"
#include <i_game_player.h>
#include "logic_player.h"
//#include "global_sys_mgr.h"

LANDLORD_SPACE_USING

game_engine::game_engine(void)
{

}


game_engine::~game_engine(void)
{
}

//static const uint16_t GAME_ID = 15;
//static const uint32_t GAME_VER = 1;

//初始化引擎
bool game_engine::init_engine(enable_xml_config& config)
{
	if(get_handler() == nullptr)
		return false;
#if defined(WIN32)
	std::string filename = config.get<std::string>("game_dll");
	size_t npos = filename.find_last_of('/');
	if(npos != std::string::npos)
		filename = filename.erase(0, npos+1);
	npos = filename.find_last_of('.');
	if(npos != std::string::npos)
		filename = filename.erase(npos);
	com_log::InitLog(filename);
#endif

	int max_player = config.get<int>("max_player");
	//@获取roomID;
	int single_room = config.get<int>("single_room");

	//m_lobby.init_game(max_player, single_room);
	
	int child_RoomID = -1;
	uint16_t pMaxNum = 0;
	if (0 == single_room) {
		single_room = -1;
	}//@1.如果同类房间多开的话需要检测一下child_room是否有值;
	else {
		//@获得房间孩子;
		child_RoomID = config.get<int>("child_room");
		if (child_RoomID == 0) {
			child_RoomID = -1;
		}
		//@获取单子房间最大人数 桌数×单桌子人数;
		pMaxNum = config.get<int>("max_player_num");
		if (pMaxNum == 0) {
			pMaxNum = 200;
		}
	}
	init_db(config, GAME_ID, single_room, child_RoomID);

	m_lobby.init_game(max_player, single_room, child_RoomID);
	std::string game_ver = config.get_ex<std::string>("game_ver", "1.0.0");
	get_handler()->on_init_engine(GAME_ID, game_ver, single_room, child_RoomID, pMaxNum);

	return true;
}


uint16_t game_engine::get_gameid()
{
	return GAME_ID;
}

//每帧调用
void game_engine::heartbeat( double elapsed )
{
	m_lobby.heartbeat(elapsed);
}

//退出引擎
void game_engine::exit_engine()
{
	m_lobby.release_game();

	if(get_handler() != nullptr)
		get_handler()->on_exit_engine();

	com_log::flush();
}

//////////////////////////////////////////////////////////////////////////
//服务器通知游戏逻辑
//玩家进入游戏
bool game_engine::player_enter_game(iGPlayerPtr igplayer)
{
	//这里如果返回false：客户端提示点击太快....;
	return m_lobby.player_enter_game(igplayer);
}


//玩家离开游戏
void game_engine::player_leave_game(uint32_t playerid)
{
	m_lobby.player_leave_game(playerid);
}

void game_engine::player_kick_player(uint32_t playerid, int bforce /*= 0*/)
{
	//@ bforce =0 ,玩家正常离开；
	//@ bforce =1,当前游戏局结束将其提出房间,需要调用正常写分流程;
	//@ bforce =2,立马踢出,游戏按逃跑处理;
	if (bforce == 1 || bforce == 2)
	{
		m_lobby.kick_player(playerid, bforce);
	}
	if (bforce == 25)
	{
		auto player = m_lobby.get_player(playerid);
		if (player)
		{
			int ret = player->can_leave_table();
			if (ret == 0)
			{
				m_lobby.player_leave_game(playerid);
			}
		}
	}
}

void game_engine::gmPlatformOpt(int32_t optype, int32_t gameID, int32_t roomID /*= 0*/, int32_t cutRound /*= 20*/, int32_t exData1 /*= 0*/, int32_t exData2 /*= 0*/)
{
	if (gameID == GAME_ID)
	{
		if (optype == 100)
		{
			//服务暂停;
			m_lobby.service_ctrl(optype, roomID);
		}
		else if (optype == 10 && exData1 == 0)
		{
			m_lobby.kill_points(roomID, cutRound, true);
		}
		else if (optype == 11)
		{
			m_lobby.kill_points(roomID, cutRound, false);
		}
		else if (optype == 10 && exData1 != 0)
		{
			if (exData1 == 1)
			{
				m_lobby.robot_ctrl(roomID, true);
			}
			else if (exData1 == 2)
			{
				m_lobby.robot_ctrl(roomID, false);
			}
		}
	}
}

int game_engine::player_join_friend_game(iGPlayerPtr igplayer, uint32_t friendid)
{
	int ret = m_lobby.player_join_friend_game(igplayer, friendid);
	if(ret != 1)
		m_lobby.player_leave_game(igplayer->get_playerid());

	return ret;
}

logic_lobby& game_engine::get_lobby()
{
	return m_lobby;
}

void game_engine::init_db(enable_xml_config& xml_cfg, int32_t gameID, int32_t roomID, int32_t child_roomID)
{
	if(xml_cfg.check("db_crypto"))
	{
		if(xml_cfg.get<int>("db_crypto") == 1)
		{
			db_log::instance().set_userpwd(xml_cfg.get<std::string>("db_user"), xml_cfg.get<std::string>("db_pwd"));
			db_game::instance().set_userpwd(xml_cfg.get<std::string>("db_user"), xml_cfg.get<std::string>("db_pwd"));
		}
		else
		{
			db_log::instance().set_userpwd(xml_cfg.get<std::string>("db_user"), xml_cfg.get<std::string>("db_pwd"),false);
			db_game::instance().set_userpwd(xml_cfg.get<std::string>("db_user"), xml_cfg.get<std::string>("db_pwd"),false);
		}
	}

	if(xml_cfg.check("gamedb_url") && xml_cfg.check("gamedb_name"))
	{
		auto gamedburl = xml_cfg.get<std::string>("gamedb_url");
		auto gamedbname = xml_cfg.get<std::string>("gamedb_name");
		db_game::instance().init_db(gamedburl, gamedbname);
	}

	if(xml_cfg.check("logdb_url") && xml_cfg.check("logdb_name"))
	{
		auto logdburl = xml_cfg.get<std::string>("logdb_url");
		auto logdbname = xml_cfg.get<std::string>("logdb_name");
		db_log::instance().init_db(logdburl, logdbname);

		//db_log::instance().push_delete(7, BSON("gameId" << gameID << "roomId" << roomID));
	}

	//db初始化之后才能加载系统
	//global_sys_mgr::instance().sys_load();
}


//返回一个机器人 返回的机器人未进入房间？
void game_engine::response_robot(int32_t playerid, int tag)
{
	m_lobby.response_robot(playerid, tag);
}

//请求一个机器人
void game_engine::request_robot(int tag, int64_t needgold, int needvip)
{
	get_handler()->request_robot(tag, needgold, needvip);
}


void  game_engine::release_robot(int32_t playerid)
{
	get_handler()->release_robot(playerid);
}

void game_engine::sync_room_players(int32_t roomid, int32_t player_count)
{
	get_handler()->syncRoomPcnt(player_count, roomid);
}
