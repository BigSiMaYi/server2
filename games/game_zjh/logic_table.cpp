#include "stdafx.h"
#include "logic_table.h"
#include "logic_player.h"

#include "logic_room.h"
#include "game_engine.h"
#include "i_game_ehandler.h"
#include "game_db.h"
#include "logic_lobby.h"
#include <net/packet_manager.h>

#include "zjh_def.pb.h"
#include "zjh_logic.pb.h"
#include "zjh_robot.pb.h"

#include "proc_zjh_logic.h"

#include "enable_random.h"

#include "robot_mgr.h"
#include "GoldFlower_CardCFG.h"
#include "time_helper.h"



ZJH_SPACE_USING

static const int MAX_TALBE_PLAYER = 5;//桌子人数
static const int MAX_OP_PLAYER = 4;//观战人数

#define MAX_TIME	180

std::string get_tagname(int32_t nTag)
{
	if (nTag == Tag_UserA)
	{
		return "A";
	}
	else if(nTag == Tag_UserB)
	{
		return "B";
	}
	else if (nTag == Tag_UserC)
	{
		return "C";
	}
	else if (nTag == Tag_Robot)
	{
		return "R";
	}
	return "GM";
}

logic_table::logic_table(void)
:m_room(nullptr)
,m_players(GAME_PLAYER)//每个桌子4个人
,m_player_count(0)
,m_elapse(0.0)
,m_checksave(0)
, m_cbKillType(0)
, m_cbKillRound(0)
, m_cbGameRound(0)
{
	
	init_game_object();

	initGame();
	// 设置 Chair ID
	for (int32_t i = 0; i < GAME_PLAYER; i++)
	{
		m_VecChairMgr.push_back(i);
	}
	// Robot
	memset(&m_robotcfg,0,sizeof(m_robotcfg));

}


logic_table::~logic_table(void)
{
}

void logic_table::release()
{
	store_game_object();
	
}

void logic_table::init_table(uint16_t tid, logic_room* room)
{
	// 保存当前房间信息
	m_room = room;
	// 设置 Table ID
	TableID->set_value(tid);
	
	if(!load_table())
		create_table();

	set_status(eGameState_Free);
	//
	m_ReadyTime = GoldFlower_BaseInfo::GetSingleton()->GetData("PreTime")->mValue;
	m_OperaTime = GoldFlower_BaseInfo::GetSingleton()->GetData("OperaTime")->mValue;
	m_ResultTime = GoldFlower_BaseInfo::GetSingleton()->GetData("ResultTime")->mValue;
	// Robot
	const boost::unordered_map<int, GoldFlower_RobotCFGData>& list = GoldFlower_RobotCFG::GetSingleton()->GetMapData();
	for (auto it = list.begin(); it != list.end(); ++it)
	{
		if(it->second.mRoomID == m_room->get_id())
		{
			m_robotcfg = it->second;
			break;
		}
	}
	m_robot_elapsed = global_random::instance().rand_int(m_robotcfg.mRobotMinEntry, m_robotcfg.mRobotMaxEntry);

#ifndef UsingLog
	//@logTest
	//mGameLog.set_begintime();
	mGameLog.Clear();
	//@70809092430:gameType:roomID:tableID:selfIncrement
	//mGameLog.set_gameroundindex("123456");
	mGameLog.set_begintime(time_helper::instance().get_cur_time());
	mGameLog.set_endtime(time_helper::instance().get_cur_time());

	//@1.设置游戏信息;
	auto gameinfo = new logic2logsvr::GameInfo();
	mGameLog.set_allocated_ginfo(gameinfo);
	//gameinfo
	gameinfo->set_gameid(game_engine::instance().get_gameid());
	gameinfo->set_roomid(m_room->get_id());
	gameinfo->set_tableid(tid);
	
	auto pinfo = mGameLog.mutable_pinfo();
	//@2.添加单个用户;
	auto ppinfo = new logic2logsvr::PlayerInfoZJH();
	pinfo->AddAllocated(ppinfo);

	//@2.1设置手牌;
	auto p2info = pinfo->Mutable(0);
//	p2info->set_cardhand(m_bUserHandCard[0], 4);

	//@2.2设置当前操作;
	auto ocurop = p2info->add_optarray();
	ocurop->set_vargold(123);
	ocurop->set_opt(1);
	
	//@发送;
	std::string strLog;
//	mGameLog.SerializeToString(&strLog);
	strLog = mGameLog.SerializeAsString();

	game_engine::instance().get_handler()->sendLogInfo(strLog,GAME_ID);
#endif

}

uint32_t logic_table::get_id()
{
	return TableID->get_value();
}

int8_t logic_table::get_status()
{

	return TableStatus->get_value();
}

void logic_table::set_status(int8_t state)
{
	TableStatus->set_value(state);
}

void logic_table::inc_dec_count(bool binc/* = true*/)
{
	if(binc)
	{
		m_player_count++;
	}
	else
	{
		m_player_count--;

		if(m_player_count == 0)
			store_game_object();
	}
}

void logic_table::heartbeat( double elapsed )
{
	if(m_player_count==0)
		return;

	
	//同步协议
	m_elapse += elapsed;
	if(m_elapse >= 0.2)
	{
		m_elapse = 0.0;
		broadcast_msglist_to_client();
	}

	m_checksave+= elapsed;
	if (m_checksave > 30)//桌子信息30s保存1次
	{
		store_game_object();

// 		// 30 秒读取一次
// 		ReadCardRate();
	}
	// player
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if(player != nullptr)
			player->heartbeat(elapsed);	
	}
	// @延时清理比牌机器人
	if (m_checkdelay > 0)
	{
		m_checkdelay -= elapsed;
		if (m_checkdelay < 0)
		{
			m_uCompareID = 0;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	if(m_duration>0)
	{
		m_duration -= elapsed;
	}
	int8_t nStatus = get_status();
	switch (nStatus)
	{
	case eGameState_Free:
		{
			// 空闲时，清理一下
			CleanKickedPlayer();

			int nPlayerCount = GetAllUserCount();
			int nReadyCount = GetActiveUserCount();

			if(nReadyCount>=PLAY_MIN_COUNT && nPlayerCount>nReadyCount)
			{
				onGameNoticeSpare();
			}
			else if(nReadyCount>=PLAY_MIN_COUNT && nPlayerCount==nReadyCount)
			{
				onEventGameStart();
			}
			else
			{

			}
			/*
			*  人数超出最低游戏人数，超出限定时间未准备玩家踢出游戏
			*/
			if (nPlayerCount >= PLAY_MIN_COUNT && m_oversee==0)
			{
				m_oversee = GITOUT;
			}
			else if (nPlayerCount < PLAY_MIN_COUNT)
			{
				m_oversee = 0;
			}
			if (m_oversee > 0)
			{
				m_oversee -= elapsed;
			}
			if (m_oversee < 0)
			{
				m_oversee = 0;
				for (auto &user : m_MapTablePlayer)
				{
					auto &tuser = user.second;
					auto &state = tuser.p_leave;
					// robot is leave
					if ( state == true) continue;
					auto player = tuser.p_playerptr;
					if (nullptr == player) continue;
					auto uid = tuser.p_id;
					if (tuser.p_active == FALSE)
					{
						state = true;
						if (player->is_robot())
						{
							SLOG_CRITICAL << boost::format("TimeOver: diss-miss :%d") % uid;
							SLOG_CRITICAL << boost::format("release_robot:line:%1%") % __LINE__;
							release_robot(uid);
						}
						else
						{
							auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_clean_out, zjh_protocols::e_mst_l2c_clean_out);
							sendmsg->set_reason(zjh_protocols::e_msg_cleanout_def::e_cleanout_not_action_twice);
							sendmsg->set_sync_gold(player->get_gold());
							player->send_msg_to_client(sendmsg);
							// clear table
							player->reqPlayer_leaveGame();
						}
					}
				}
			}
			// 清理机器人
// 			time_t now = time_helper::instance().get_tick_count();
// 			for(auto user : m_MapTablePlayer)
// 			{
// 				auto tuser = user.second;
// 				// robot is leave
// 				if(tuser.p_leave==true) continue;
// 				auto player = tuser.p_playerptr;
// 				auto uid = tuser.p_id;
// 				if(nullptr == player ) continue;
// 
// 				if(!player->is_robot()) continue;
// 				if( now - tuser.p_time >= DISSMISS)
// 				{
// 					SLOG_CRITICAL<<boost::format("TimeOver: diss-miss :%d")%uid;
// 					SLOG_CRITICAL << boost::format("release_robot:line:%1%") % __LINE__;
// 					release_robot(uid);
// 				}	
// 			}
		}
		break;
	case eGameState_Spare:
		{
			// 直接开始
			int nPlayerCount = GetAllUserCount();
			int nReadyCount = GetActiveUserCount();
			if(nReadyCount>=PLAY_MIN_COUNT && nPlayerCount==nReadyCount)
			{
				m_duration = 0;
				onEventGameStart();
			}
			// 
			if(m_duration < 0)
			{
				if(checkGameStart())
				{
					onEventGameStart();
				}
				else
				{
					m_duration = 0;
					set_status(eGameState_Free);
				}
			}
		}
		break;
	case eGameState_FaPai:
		{
			if(m_duration < 0)
			{
				m_duration = 0.0;
				onGameNoticeStart();
			}
		}
		break;
	case eGameState_Play:
		{
			if (m_duration<0)
			{
				SLOG_DEBUG<<"logic_table::heartbeat[Time Out]: "<<m_nCurOperatorID;
				m_duration = 0.0;
				if(m_nCurOperatorID!=0)
				{

					onGameGiveUp(m_nCurOperatorID,true);
				}
				else
				{
					SLOG_CRITICAL<<"logic_table::heartbeat curOperatorID==0";
				}
			}
		}
		break;
	case eGameState_End:
		{
			if (m_duration < 0)
			{
				m_duration = 0.0;

				set_status(eGameState_Free);

				// 桌子复位
				repositGame();
			}
		}
		break;
	default:
		break;
	}

	// TEST

	// Robot
	robot_heartbeat(elapsed);
	// TEST
}

bool logic_table::is_full()
{
	return m_player_count >= GAME_PLAYER;
}

bool zjh_space::logic_table::is_opentable()
{
	return m_player_count >= 1;
}


bool zjh_space::logic_table::is_all_robot()
{
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if(player && !player->is_robot()) 
			return false;
	}
	return true;
}

int32_t zjh_space::logic_table::all_robot()
{
	int32_t nCount = 0;
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if(player && player->is_robot()) 
			nCount++;
	}
	return nCount;
}

int32_t zjh_space::logic_table::all_man()
{
	int32_t nCount = 0;
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if (player && !player->is_robot())
			nCount++;
	}
	return nCount;
}

unsigned int logic_table::get_max_table_player()
{
	return m_players.size();
}

LPlayerPtr& logic_table::get_player(int index)
{
	if(isVaildSeat(index))
		return m_players[index];

	return logic_player::EmptyPtr;

}

LPlayerPtr& logic_table::get_player_byid(uint32_t pid)
{
	for (int i = 0; i < GAME_PLAYER; i++)
	{
		if (m_players[i] != nullptr && m_players[i]->get_pid() == pid)
		{
			return m_players[i];
		}
	}
	return logic_player::EmptyPtr;
}

void logic_table::PlatformKiller(int32_t optype, int32_t cutRound)
{
	if (m_player_count > 0)
	{
		m_cbKillRound = 0;
		m_cbGameRound = 0;
	}
}

logic_room* logic_table::get_room()
{
	return m_room;
}

//logic_main* logic_table::get_game_main()
//{
//	return m_pMain;
//}

int32_t logic_table::get_seat_byid(uint32_t pid)
{
	int32_t seat = INVALID_CHAIR;
	auto it = m_MapTablePlayer.find(pid);
	if(it != m_MapTablePlayer.end())
	{
		seat = it->second.p_idx;
	}
	return seat;
}

uint32_t zjh_space::logic_table::get_id_byseat(int32_t seat)
{
	uint32_t uid = 0;
	for (auto user : m_MapTablePlayer)
	{
		if(user.second.p_idx == seat)
		{
			uid = user.second.p_id;
			break;
		}
	}
	return uid;
}

int logic_table::enter_table(LPlayerPtr player)
{
	if(player->get_table()!=nullptr) 
	{
		SLOG_ERROR<<"logic_table enter_table is in table";
		return 2;
	}

	// 随机分配椅子
	if(m_VecChairMgr.size()>0)
	{
		std::random_shuffle(std::begin(m_VecChairMgr),std::end(m_VecChairMgr));
		int32_t seat = m_VecChairMgr.back();
		m_VecChairMgr.pop_back();
		// find seat -- order by 
	//	bc_enter_seat(seat, player);
		// 
		uint32_t id = player->get_pid();
		m_pids.push_back(player->get_pid());

		inc_dec_count();
		//
		TablePlayer table_player;
		memset(&table_player,0,sizeof(table_player));
		table_player.p_playerptr = player;
		table_player.p_idx = seat;
		uint32_t uid = player->get_pid();
		table_player.p_id  = uid;
		table_player.p_asset = player->get_gold();
		table_player.p_leave = false;
		table_player.p_background = false;
		table_player.p_kick = false;
		table_player.p_time = time_helper::instance().get_tick_count();
		tagPlayerCtrlAttr attr;
		bool bRet = player->get_player_ctrl(attr);
		int nTag = Tag_UserA;
		if(bRet)
		{
			nTag = attr.nTag;
		}
		table_player.p_tag = nTag;
		table_player.p_percent = attr.fwinPercent;
		m_MapTablePlayer.insert(std::make_pair(uid,table_player));

		if(get_status()==eGameState_Play)
		{
			player->set_status(eUserState_Wait);
		}
		else if(get_status()==eGameState_FaPai)
		{
			player->set_status(eUserState_Wait);
		}
		else
		{
			player->set_status(eUserState_free);
		}

		SLOG_CRITICAL<<boost::format("logic_table::enter_table:%1%,seatid:%2%")%get_id()%seat;
		SLOG_CRITICAL<<boost::format("logic_table::enter_table:[Tag:%d,Win:%f]")%attr.nTag%attr.fwinPercent;

		bc_enter_seat(table_player);
		return 1;
	}
#ifdef UsingCling
	// 已满情况下
	if (player && player->is_robot())
	{
		player->join_table(nullptr, -1);
		return 12;
	} 
	// @ cling to robot
	int32_t seat = cling_robotid();
	if (seat != INVALID_CHAIR)
	{ // 
	  /* 1、一个位置真实玩家只附加一个
	  *  2、游戏结束将附加玩家加入对局
	  */
		uint32_t id = player->get_pid();
		m_pids.push_back(player->get_pid());
		//
		TablePlayer table_player;
		memset(&table_player, 0, sizeof(table_player));
		table_player.p_playerptr = player;
		table_player.p_idx = seat;
		uint32_t uid = player->get_pid();
		table_player.p_id = uid;
		table_player.p_asset = player->get_gold();
		table_player.p_leave = false;
		table_player.p_background = false;
		table_player.p_kick = false;
		table_player.p_time = time_helper::instance().get_tick_count();
		tagPlayerCtrlAttr attr;
		bool bRet = player->get_player_ctrl(attr);
		int nTag = Tag_UserA;
		if (bRet)
		{
			nTag = attr.nTag;
		}
		table_player.p_tag = nTag;
		table_player.p_percent = attr.fwinPercent;
		player->set_status(eUserState_Wait);
		m_MapExtraPlayer.insert(std::make_pair(seat, table_player));
		SLOG_CRITICAL << boost::format("logic_table::enter_table-2:%1%,seatid:%2%") % get_id() % seat;
		player->join_table(this, seat);
		return 1;
	}
#endif // UsingCling
	//
	player->join_table(nullptr,-1);
	return 12;
}

void logic_table::leave_table(uint32_t pid, bool bkick/*=false*/)
{
#if 0
	for (int i =0;i<GAME_PLAYER;i++)
	{
		LPlayerPtr& player = get_player(i);
		if(player != nullptr && player->get_pid() == pid)
		{
			player->set_status(eUserState_null);
			player->clear_round();

			player = nullptr;
			//
			m_room->leave_table(pid);
			// 
			inc_dec_count(false);

			bc_leave_seat(pid);	
			
			break;
		}
	}
#else
	// 清理用户
	auto it = m_MapTablePlayer.find(pid);
	if(it != m_MapTablePlayer.end())
	{
		auto user = it->second;
		LPlayerPtr& player = user.p_playerptr;
		// 异常离开
		if (can_leave_table(pid) == false)
		{ // 强退
			SLOG_ERROR << boost::format("Warning: Forced leave game :%1%") % pid;
// 			if (bkick == true)
// 			{ // 强制踢出
// 				auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_clean_out, zjh_protocols::e_mst_l2c_clean_out);
// 				sendmsg->set_reason((zjh_protocols::e_msg_cleanout_def)7);
// 				player->send_msg_to_client(sendmsg);
// 
// 				player->reqPlayer_leaveGame();
// 			}
// 			int64_t lUserExpend = user.p_expend;
// 			int64_t lGold = 0;
// 			if (lUserExpend > 0)
// 			{
// 				lGold -= lUserExpend;
// 				player->write_property(lGold);
// 				SLOG_CRITICAL << boost::format("User[%1%]-Forced-Leave:expend:%2%") % pid%lGold;
// 			}
		}


		int32_t seat = user.p_idx;
#ifdef UsingCling
		// @ 让座机器人除外
		if (player != nullptr && player->is_robot())
		{
			if (user.p_kick == false)
			{
				m_VecChairMgr.push_back(seat);
			}
		}
		else
		{
			m_VecChairMgr.push_back(seat);
		}
#else
		m_VecChairMgr.push_back(seat);
#endif // UsingCling

		// 允许弃牌离开
		if(player && player->get_status() == eUserState_dead)
		{
			int64_t lUserExpend = user.p_expend;
			int64_t lGold = 0;
			if(lUserExpend > 0)
			{
				lGold -= lUserExpend;
				player->write_property(lGold);
				SLOG_CRITICAL<<boost::format("User[%1%]-Leave-Table:expend:%2%")%pid%lGold;
			}
#ifdef UsingLog
			auto count = mGameLog.pinfo_size();
			for (int i = 0; i < count; i++)
			{
				auto pinfo = mGameLog.mutable_pinfo(i);
				auto pinfoid = pinfo->pid();
				if (pid == pinfoid)
				{
					pinfo->set_vargold(lGold);
					pinfo->set_endtime(time_helper::instance().get_cur_time());
				}
			}
#endif // UsingLog
		}
		
		// 清理用户
// 		if(player)
// 		{
// 			int64_t lUserExpend = user.p_expend;
// 			int64_t lGold = 0;
// 			if(lUserExpend > 0)
// 			{
// 				lGold -= lUserExpend;
// 				player->write_property(lGold);
// 				SLOG_CRITICAL<<boost::format("User[%1%]-Leave-Table:expend:%2%")%pid%lGold;
// 			}
// 
// 			player->set_status(eUserState_null);
// 			player->clear_round();
// 			player->set_wait(0);
// 		}
		m_room->leave_table(pid);
		// 
		inc_dec_count(false);

		bc_leave_seat(pid);	

		// run away
		int32_t nGameState = get_status();
		if( nGameState==eGameState_FaPai || nGameState==eGameState_Play )
		{
			if(pid == m_nCurOperatorID)
			{
				m_nCurOperatorID = GetNextPlayerByID(pid);
			}
			if(pid == m_nFirstID)
			{
				m_nFirstID = GetNextPlayerByID(pid);
			}
		}
		
		// clear
		m_MapTablePlayer.erase(it);

		// 
		if( nGameState == eGameState_Spare)
		{
			int nPlayerCount = GetAllUserCount();
			int nReadyCount = GetActiveUserCount();

			if(checkStartSpare())
			{
				if(nReadyCount>=PLAY_MIN_COUNT && nPlayerCount==nReadyCount)
				{
					onEventGameStart();
				}
			}
			else
			{
				m_duration = 0;
				set_status(eGameState_Free);
			}
		}
		else if( nGameState==eGameState_FaPai || 
			nGameState==eGameState_Play )
		{
			if(player && player->get_status() == eUserState_play)
			{
				// runaway
				onPlayerRunaway(pid);
			}

		}
		else if(nGameState==eGameState_End)
		{

		}
		else 
		{

		}
	}

#endif

#ifdef UsingCling
	// 清理等待
	for (auto itextra = m_MapExtraPlayer.begin(); itextra != m_MapExtraPlayer.end();)
	{
		auto table_player = itextra->second;
		if (table_player.p_id == pid)
		{
			int32_t seat = table_player.p_idx;
			// @椅子延时归还
			//m_VecChairMgr.push_back(seat);
			m_room->leave_table(pid);
			m_MapExtraPlayer.erase(itextra++);

		}
		else
		{
			itextra++;
		}
	}
#endif // UsingCling
	// 清理 pid_vec
	for (unsigned int i = 0; i < m_pids.size(); i++)
	{
		if (m_pids[i] == pid)
		{
			m_pids[i] = m_pids[m_pids.size() - 1];
			m_pids.pop_back();
			break;
		}
	}

}

bool logic_table::can_leave_table(uint32_t pid)
{
	auto it = m_MapTablePlayer.find(pid);
	if(it != m_MapTablePlayer.end())
	{
		auto user = it->second;
		LPlayerPtr player = user.p_playerptr;
		if(player)
		{
			if(player->get_status()==eUserState_play)
			{
				return false;
			}
			else if(player->get_status()==eUserState_dead)
			{// 允许弃牌离开
				// @排除比牌动画过程中离开
				if(m_uCompareID != pid)
					return true;
			}
			else
			{
				return true;
			}
		}
	}
#ifdef UsingCling
	// @队列用户离开
	for (auto it = m_MapExtraPlayer.begin(); it != m_MapExtraPlayer.end(); it++)
	{
		auto findextra = it->second;
		if (findextra.p_id == pid)
		{
			return true;
		}
	}
#endif // UsingCling
	// 
	return false;
}

void zjh_space::logic_table::getout_table(uint32_t pid)
{
	auto it = m_MapTablePlayer.find(pid);
	if (it != m_MapTablePlayer.end())
	{
		auto& tuser = it->second;
		auto player = tuser.p_playerptr;
		tuser.p_kick = true;
	}
}

bool logic_table::change_sit(uint32_t pid, uint32_t seat_index)
{

	return true;
}

int logic_table::broadcast_msg_to_client(std::vector<uint32_t>& pids, uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg)
{
//	printf("broadcast_msg_to_client: %d:%d,,%d\n",pids.size(),pids[pids.size()-1],packet_id);

	return game_engine::instance().get_handler()->broadcast_msg_to_client(pids, packet_id, msg);

}


void logic_table::bc_enter_seat(int seat_index, LPlayerPtr& player)
{
	printf("logic_table enter seat ,seat:%d \n",seat_index);

	player->join_table(this,seat_index);
	
	//  notice other
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_user_enter_seat,zjh_protocols::e_mst_l2c_user_enter_seat);
	auto playerinfo = sendmsg->mutable_playerinfo();
	playerinfo->set_seat_index(seat_index);
	playerinfo->set_player_id(player->get_pid());
	playerinfo->set_player_nickname(player->get_nickname());
	playerinfo->set_player_headframe(player->get_photo_frame());
	playerinfo->set_player_headcustom(player->get_icon_custom());
	playerinfo->set_player_gold(player->get_gold());
	playerinfo->set_player_sex(player->get_sex());
	playerinfo->set_player_viplv(player->get_viplvl());
	playerinfo->set_player_ticket(player->get_ticket());
	playerinfo->set_player_region(player->GetUserRegion());
	broadcast_msg_to_client2(sendmsg);

}


void zjh_space::logic_table::bc_enter_seat(TablePlayer& tabplayer)
{
	LPlayerPtr player = tabplayer.p_playerptr;
	if(!player) return;
	int seat_index = tabplayer.p_idx;
	player->join_table(this,seat_index);
	int32_t nTag = tabplayer.p_tag;
	double fPercent = tabplayer.p_percent;
	//  notice other
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_user_enter_seat,zjh_protocols::e_mst_l2c_user_enter_seat);
	auto playerinfo = sendmsg->mutable_playerinfo();
	playerinfo->set_seat_index(seat_index);
	playerinfo->set_player_id(player->get_pid());
	playerinfo->set_player_nickname(player->get_nickname());
	playerinfo->set_player_headframe(player->get_photo_frame());
	playerinfo->set_player_headcustom(player->get_icon_custom());
	playerinfo->set_player_gold(player->get_gold());
	playerinfo->set_player_sex(player->get_sex());
	playerinfo->set_player_viplv(player->get_viplvl());
	playerinfo->set_player_ticket(player->get_ticket());
	playerinfo->set_player_region(player->GetUserRegion());

	//auto playertag = playerinfo->mutable_playertag();
	//playertag->set_type(nTag);
	//playertag->set_percent(fPercent);

	broadcast_msg_to_client2(sendmsg);

}

void logic_table::bc_leave_seat(uint32_t player_id)
{
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_user_leave_seat, zjh_protocols::e_mst_l2c_user_leave_seat);
	sendmsg->set_playerid(player_id);
	broadcast_msg_to_client2(sendmsg);
}

//////////////////////////////////////////////////////////////////////////
void logic_table::create_table()
{
	
	

}

void logic_table::init_game_object()
{
	TableID = CONVERT_POINT(Tfield<int16_t>, regedit_tfield(e_got_int16, "table_id"));

	TableStatus = CONVERT_POINT(Tfield<int8_t>, regedit_tfield(e_got_int8,"table_status"));
}

bool logic_table::load_table()
{
	mongo::BSONObj b = db_game::instance().findone(DB_ZJHTABLE, BSON("room_id"<<m_room->get_id() << "table_id" << TableID->get_value()));
	if(b.isEmpty())
		return false;

	return from_bson(b);
	
}

bool logic_table::store_game_object(bool to_all)
{
	if(!has_update())
		return true;

	auto err = db_game::instance().update(DB_ZJHTABLE, BSON("room_id"<<m_room->get_id() << "table_id" << TableID->get_value()), BSON("$set"<<to_bson(to_all)));
	if(!err.empty())
	{
		SLOG_ERROR << "logic_table::store_game_object :" <<err;
		return false;
	}

	m_checksave = 0;

	return true;
}


void zjh_space::logic_table::req_scene(uint32_t playerid)
{
	auto player_it = m_MapTablePlayer.find(playerid);
	if (player_it != m_MapTablePlayer.end())
	{
		auto &user = player_it->second;
		user.p_background = false;
		auto player = user.p_playerptr;
		if (player) player->set_serverflag(0);
	}
}

boost::shared_ptr<zjh_protocols::packetl2c_scene_info_free> logic_table::get_scene_info_msg_free()
{
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_scene_info_free,zjh_protocols::e_mst_l2c_scene_info_free);
	auto freeinfo = sendmsg->mutable_freeinfo();
	int8_t nGameState = get_status();
	int32_t nSpareTime = 0;
	if(nGameState==eGameState_Spare)
	{
		nSpareTime = m_duration;
	}
	freeinfo->set_status(nGameState);
	freeinfo->set_roomid(m_room->get_id());
	freeinfo->set_tableid(get_id());
	freeinfo->set_sparetime(nSpareTime);
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto seat = user.p_idx;
		auto nTag = user.p_tag;
		auto fPercent = user.p_percent;
		if(player!=nullptr)
		{
			auto playerinfo = freeinfo->add_playerinfo();
			playerinfo->set_seat_index(seat);
			playerinfo->set_player_id(player->get_pid());
			playerinfo->set_player_nickname(player->get_nickname());
			playerinfo->set_player_headframe(player->get_photo_frame());
			playerinfo->set_player_headcustom(player->get_icon_custom());
			playerinfo->set_player_gold(player->get_gold());
			playerinfo->set_player_sex(player->get_sex());
			playerinfo->set_player_viplv(player->get_viplvl());
			playerinfo->set_player_ticket(player->get_ticket());
			// Bug: 2017-8-30 
			int32_t nUserState = player->get_status();
			if (nUserState == eUserState_null)
			{
				player->set_status(eUserState_free);
			}
			playerinfo->set_player_status(player->get_status());
			playerinfo->set_player_region(player->GetUserRegion());

			/*auto playertag = playerinfo->mutable_playertag();
			playertag->set_type(nTag);
			playertag->set_percent(fPercent);*/

			playerinfo->clear_usergold();
			playerinfo->clear_cardinfo();
		}
	}

	return sendmsg;
}

boost::shared_ptr<zjh_protocols::packetl2c_scene_info_play> zjh_space::logic_table::get_scene_info_msg_play(uint32_t uid)
{
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_scene_info_play, zjh_protocols::e_mst_l2c_scene_info_play);
	// 
	auto playinfo = sendmsg->mutable_playinfo();
	int8_t nGameState = get_status();
	playinfo->set_status(nGameState);
	int32_t rid = m_room->get_id();
	playinfo->set_roomid(rid);
	playinfo->set_tableid(get_id());
	playinfo->set_bankerid(m_uBankerID);
	playinfo->set_curid(m_nCurOperatorID);
	playinfo->set_leftcd(m_duration);
	playinfo->set_curchip(m_lCurJetton);
	playinfo->set_curround(m_nGameRound);
	playinfo->set_pool_gold(m_lTotalSliver);

	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto seat = user.p_idx;
		auto usergold = user.p_asset;
		auto userid = user.p_id;
		auto cflag = user.p_cardflag;
		if (player != nullptr)
		{
			auto playerinfo = playinfo->add_playerinfo();
			playerinfo->set_seat_index(seat);
			playerinfo->set_player_id(userid);
			playerinfo->set_player_nickname(player->get_nickname());
			playerinfo->set_player_headframe(player->get_photo_frame());
			playerinfo->set_player_headcustom(player->get_icon_custom());
			playerinfo->set_player_gold(player->get_gold());
			playerinfo->set_player_sex(player->get_sex());
			playerinfo->set_player_viplv(player->get_viplvl());
			playerinfo->set_player_ticket(player->get_ticket());
			playerinfo->set_player_region(player->GetUserRegion());

			// play wait
			playerinfo->set_player_status(player->get_status());

			playerinfo->set_usergold(usergold);
			// card info 
			GameCard card;
			if (userid == uid && (cflag&&eCardState_Ming == eCardState_Ming || cflag == eCardState_Ming))
			{
				auto fresult = m_MapPlayerCard.find(uid);
				if (fresult != m_MapPlayerCard.end())
				{
					card = fresult->second;
				}
			}
			auto cardinfo = playerinfo->mutable_cardinfo();
			cardinfo->mutable_pokers()->Reserve(MAX_CARDS_HAND);
			for (int j = 0; j < MAX_CARDS_HAND; j++)
			{
				cardinfo->add_pokers(card.nCard[j]);
			}
			int nCardModel = card.nModel;
			cardinfo->set_pokertype(nCardModel);
			cardinfo->set_pokerstatus(cflag);
		}
	}

	return sendmsg;
}

template<class T>
void zjh_space::logic_table::broadcast_msg_to_client2(T msg)
{
#if 0
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if (player != nullptr )
		{
#if 0
			player->send_msg_to_client(msg);
#else
			if(!player->is_robot())
			{
				player->send_msg_to_client(msg);
			}
			else
			{
				// Robot
				robot_mgr::instance().recv_packet(player->get_pid(),msg->packet_id(),msg);
			}
#endif
		}
	}
#else
	add_msg_to_list(msg);
#endif

}


template<class T>
void zjh_space::logic_table::broadcast_msg_to_client3(T msg)
{
	Vec_UserID userpids;
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if (player != nullptr)
		{
			auto uid = player->get_pid();
			if (!player->is_robot())
			{
				userpids.push_back(uid);
			}
			else
			{
				robot_mgr::instance().recv_packet(uid, msg->packet_id(), msg);
			}
		}
	}
	if (userpids.size() > 0)
	{
		game_engine::instance().get_handler()->broadcast_msg_to_client(userpids,msg->packet_id(),msg);
	}

}

int32_t zjh_space::logic_table::GetActiveUserCount()
{
	int32_t nActiveCount = 0;
	for(auto user : m_MapTablePlayer)
	{
		auto tuser = user.second;
		// robot is leave
		if(tuser.p_leave==true) continue;
		if(tuser.p_active == TRUE)
			nActiveCount++;
	}

	return nActiveCount;
}

int32_t zjh_space::logic_table::GetAllUserCount()
{
	
	return m_MapTablePlayer.size();
}

bool zjh_space::logic_table::onEventUserReady(uint32_t playerid)
{
	int32_t nResult = eReadyResult_Succeed;
	//
	auto it = m_MapTablePlayer.find(playerid);
	if(it == m_MapTablePlayer.end())
	{
		SLOG_CRITICAL<<"logic_table::onEventUserReady Can't Ready:nullptr";
		nResult = eReadyResult_Faild_unknow;
	}
	else
	{
		auto& tuser = it->second;
		auto player = tuser.p_playerptr;
		if(player->is_robot())
		{
			SLOG_CRITICAL<<boost::format("onEventUserReady:Robot-id:%1%")%playerid;
			if(tuser.p_leave==true)
			{
				SLOG_CRITICAL<<boost::format("onEventUserReady:Robot-id:%1%,leave")%playerid;
				return true;
			}
		}
		else
		{
			SLOG_CRITICAL<<boost::format("onEventUserReady:Player-id:%1%")%playerid;
		}
		if(player->get_status()>eUserState_free)
		{
			SLOG_CRITICAL<<"logic_table::onEventUserReady Can't Ready:usestate";
			nResult = eReadyResult_Faild_userstate;
		}
		else if(get_status()!=eGameState_Free && get_status()!=eGameState_Spare)
		{
			SLOG_CRITICAL<<"logic_table::onEventUserReady Can't Ready:gamestate";
			nResult = eReadyResult_Faild_gamestate;
		}
		else if(player->get_gold()< m_room->get_data()->mGoldMinCondition)
		{
			SLOG_CRITICAL<<"logic_table::onEventUserReady Can't Ready:gold";
			nResult = eReadyResult_Faild_gold;
		}
		else
		{
			// succeed
			tuser.p_active = TRUE;
			nResult = eReadyResult_Succeed;
			player->set_status(eUserState_ready);
			SLOG_CRITICAL<<boost::format("onEventUserReady succeed:id:%1%")%playerid;
		}
	}

	auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_ask_ready_result,zjh_protocols::e_mst_l2c_ask_ready_result);
	sendmsg->set_playerid(playerid);
	sendmsg->set_result(nResult);
	// TODO Bug to fix : can't send msg
//	int ret = broadcast_msg_to_client(sendmsg);
	// TODO send msg ok
//	m_room->broadcast_msg_to_client(sendmsg);

#if 0
	broadcast_msg_to_client2(sendmsg);
#else
	broadcast_msg_to_client3(sendmsg);
#endif

	return true;
}

void zjh_space::logic_table::initGame(bool bResetAll/*=false*/)
{
	set_status(eGameState_Free);

	// 初始化游戏参数
	m_uBankerID = 0;

	m_nCurOperatorID = 0;

	m_nLastWiner = 0;
	
	m_lCurJetton = 0;

	m_lTotalSliver = 0;
	m_lInventPool = 0;
	m_lEqualize = 0;


	m_nFirstID = 0;
	m_MapCompareNexus.clear();
	m_VecActive.clear();
	m_VecPlaying.clear();
	m_VecAllin.clear();

	m_nShowHandCnt = 0;

	m_nGameRound = 0;

	m_nEndType = eGameEndType_null;
	m_oversee = 0;
}


void zjh_space::logic_table::repositGame()
{
	SLOG_CRITICAL<<"------------RepositGame------------";
	// 游戏数据初始化
	m_uBankerID = 0;

	m_nCurOperatorID = 0;
	//m_nLastWiner = 0;
	m_lCurJetton = 0;

	m_lTotalSliver = 0;
	m_lInventPool = 0;
	m_lEqualize = 0;


	m_nFirstID = 0;
	m_MapCompareNexus.clear();
	m_VecActive.clear();
	m_VecPlaying.clear();
	m_uCompareID = 0;
	m_nGameRound = 0;

	m_nEndType = eGameEndType_null;
	set_status(eGameState_Free);

	m_nShowHandCnt = 0;
	m_VecAllin.clear();
	m_oversee = 0;

#ifdef UsingCling
	// @附加玩家加入对局
	joinnextround();
#endif // UsingCling

	// @满了给玩家留一个空位
	bool bKeepOne = (m_MapTablePlayer.size()==GAME_PLAYER) ? true : false;
	bool bCleanByRound = false, bCleanByMinGold = false, bCleanByMaxGold = false;
	// 重置数据
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		//if (user.p_leave == true) continue;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		auto uid = user.p_id;
		user.p_active = FALSE;
		user.p_playing = FALSE;
		user.p_asset = player->get_gold();
		user.p_expend = 0;
		user.p_cardflag = eCardState_null;
		user.p_time = time_helper::instance().get_tick_count();

		player->set_status(eUserState_free);

		// 清理机器人
		if(player->is_robot())
		{
			if (m_room->getRobotSwitch() == 2)
			{
				SLOG_CRITICAL << boost::format("release_robot:line:%1%") % __LINE__;
				release_robot(uid);
			}
			else if(player->get_gold()<m_room->get_data()->mGoldMinCondition)
			{
				SLOG_CRITICAL << boost::format("release_robot:line:%1%") % __LINE__;
				release_robot(uid);
				bCleanByMinGold = true;
			}
		}
	}
	// 清理机器人 
	// Modfy 9/29/2017:一次清理一个
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto &user = it->second;
		if (user.p_leave == true) continue;
		auto player = user.p_playerptr;
		if (player == nullptr) continue;
		auto uid = user.p_id;
		if (player->is_robot())
		{
			int32_t nRound = global_random::instance().rand_int(m_robotcfg.mRobotMinRound, m_robotcfg.mRobotMaxRound);
			int32_t nRRound = player->get_round();
			SLOG_CRITICAL << boost::format("repositGame:[uid:%d,round:%d,robot:%d]") % uid%nRRound%nRound;
			if (bCleanByRound && nRRound >= nRound)
			{ // 达到轮数条件
				SLOG_CRITICAL << boost::format("release_robot:line:%1%") % __LINE__;
				release_robot(uid);
				bCleanByRound = true;
			}
			else if (bCleanByMaxGold &&  player->get_gold() > m_robotcfg.mRobotMaxGold)
			{ // 超出携带数量
				SLOG_CRITICAL << boost::format("release_robot:line:%1%") % __LINE__;
				release_robot(uid);
				bCleanByMaxGold = true;
			}
		}
	}
	// @此功能更改
	// @清理机器人留出一个位置给玩家
// 	if ( bKeepOne && (!bCleanByMinGold) && (!bCleanByMaxGold) && (!bCleanByRound))
// 	{
// 		for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
// 		{
// 			auto &user = it->second;
// 			if (user.p_leave == true) continue;
// 			auto player = user.p_playerptr;
// 			if (player == nullptr) continue;
// 			auto uid = user.p_id;
// 			if (player->is_robot())
// 			{
// 				SLOG_CRITICAL << boost::format("release_robot:line:%1%") % __LINE__;
// 				release_robot(uid);
// 				break;
// 			}
// 		}
// 	}

	// 清理超出限制机器人 : 一次清理一个
// 	for (auto user : m_MapTablePlayer)
// 	{
// 		LPlayerPtr& player = user.second.p_playerptr;
// 		if (!player) continue;
// 		if (!player->is_robot()) continue;
// 		if (user.second.p_leave == true) continue;
// 		if (player->get_gold() > m_robotcfg.mRobotMaxGold)
// 		{ // 超出携带数量
// 			SLOG_CRITICAL << boost::format("release_robot:line:%1%") % __LINE__;
// 			release_robot(player->get_pid());
// 			break;
// 		}
// 		
// 	}
	
	// 清理不足条件用户
	CleanOutPlayer();
	// 清理掉线用户
	CleanDisconnect();
	// 清理挂起用户
	OnDealEnterBackground();
	// 
	//CleanKickedPlayer();
}

bool zjh_space::logic_table::onGameNoticeSpare()
{
	SLOG_CRITICAL<<"------------onGameNoticeSpare------------";
	set_status(eGameState_Spare);
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_notice_sparetime, zjh_protocols::e_mst_l2c_notice_sparetime);
	sendmsg->set_sparetime(m_ReadyTime);
	broadcast_msg_to_client2(sendmsg);
	m_duration = m_ReadyTime + TIME_LESS;
	return true;
}

bool logic_table::onEventGameStart()
{
	SLOG_CRITICAL<<"------------GameStart------------";
	set_status(eGameState_FaPai);
	std::srand(unsigned(std::time(0)));
	m_VecActive.clear();
	m_VecPlaying.clear();
	m_VecAllin.clear();
	m_VecAddJetton = m_room->get_data()->mAddChipList;
	//
#ifdef UsingLog
	mGameLog.Clear();
	mOptLog.clear();
	// gameinfo
	auto gameinfo = mGameLog.mutable_ginfo();
	gameinfo->set_gameid(game_engine::instance().get_gameid());
	gameinfo->set_roomid(m_room->get_id());
	gameinfo->set_tableid(get_id());
	mGameLog.set_begintime(time_helper::instance().get_cur_time());

// 	std::shared_ptr<logic2logsvr::ZJHGameLog> mLog;
// 	mLog.reset();
// 	mLog = std::make_shared<logic2logsvr::ZJHGameLog>();
// 	auto ginfo = mLog->mutable_ginfo();
// 	ginfo->set_gameid(game_engine::instance().get_gameid());
// 	ginfo->set_roomid(m_room->get_id());
// 	ginfo->set_tableid(get_id());
// 	std::string logstr = mLog->SerializeAsString();
// 	game_engine::instance().get_handler()->sendLogInfo(logstr, game_engine::instance().get_gameid());
#endif // UsingLog
	ReadCardRate();
	m_GameCardMgr.SetRate(m_VecCardRate);
	m_GameCardMgr.Reposition();
	// 底注
	int64_t base = m_room->get_data()->mBaseCondition;
	// check play and active
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto playerid = user.p_id;
		auto leave = user.p_leave;
		if(player==nullptr) continue;
		if(leave==true) continue;
		if(user.p_active==TRUE)
		{
			m_VecActive.push_back(playerid);
			m_VecPlaying.push_back(playerid);

			user.p_playing = TRUE;
			user.p_cardflag = eCardState_An;

			user.p_asset -= base;
			user.p_expend += base;
			user.p_result = 0;
			user.p_time = time_helper::instance().get_tick_count();
			player->set_status(eUserState_play);
			player->add_round();
			player->set_wait(0);
			// card mgr
			tagPlayerCtrlAttr attr;
			bool bRet = player->get_player_ctrl(attr);
			int nTag = Tag_UserB;
			if(bRet)
			{
				nTag = attr.nTag;
			}
			user.p_tag = nTag;
			user.p_percent = attr.fwinPercent;
			m_GameCardMgr.SetUserLabel(playerid,nTag);
			SLOG_CRITICAL<<boost::format("onEventGameStart:[playerid:%d,Tag:%d,Win:%f]")%playerid%attr.nTag%attr.fwinPercent;
			m_GameCardMgr.SetUserPos(playerid, user.p_idx);
			m_lTotalSliver += base;
#ifdef UsingLog
			// log:记录参与本局游戏
			auto plog = mGameLog.add_pinfo();
			plog->set_pid(playerid);
			plog->set_goldbegin(player->get_gold());
			plog->set_seatid(user.p_idx);
			plog->set_flag(get_tagname(nTag));
			plog->set_isrobot(player->is_robot());

			tagPlayerFlags playerflags;
			player->fill_playerflags(playerflags);
			auto pflags = plog->mutable_pflags();
			pflags->set_flags(playerflags.FlagsS);
			pflags->set_winp(playerflags.WinP);
			pflags->set_opcoeff(playerflags.opCoeff);
			pflags->set_flagsmoneylit(playerflags.flagsMoneyLit);
			pflags->set_curmoneyget(playerflags.CurMoneyGet);
			pflags->set_gmopflags(playerflags.GmOpFlags);
			pflags->set_recvcoincnt(playerflags.RecvCoinCnt);
			pflags->set_sendcoincnt(playerflags.SendCoinCnt);
			pflags->set_withdrawcnt(playerflags.WithDrawCnt);
			pflags->set_rechargecnt(playerflags.RechargeCnt);
			pflags->set_flagsy(playerflags.FlagsY);
			pflags->set_flagsx(playerflags.FlagsX);
			pflags->set_safebag(playerflags.SafeBag);
#endif // UsingLog
		}
		else
		{
			player->set_status(eUserState_Wait);
			player->add_wait();
		}
	}
	// 洗牌
#if 0
	m_GameCardMgr.Wash_Card();
#else
	// @ 默认取牌
	//m_GameCardMgr.Wash_CardEx2();
#endif
	m_MapPlayerCard.clear();
	// 选择庄家
	std::random_shuffle(m_VecActive.begin(),m_VecActive.end()); 
	if(m_VecActive.size() < PLAY_MIN_COUNT)
	{
		assert(0);
	}
	// 赢的下一个玩家做庄 赢家为庄
	uint32_t & WinerID = m_nLastWiner;
	uint32_t & bankerid = m_uBankerID;
	auto winer_it = m_MapTablePlayer.find(WinerID);
	if(winer_it == m_MapTablePlayer.end())
	{
		WinerID = 0;
	}
	if(WinerID!=0)
	{
		//	bankerid = GetNextPlayerByID(WinerID);
		bankerid = WinerID;
		WinerID = 0;
	}
	else
	{
		bankerid = m_VecActive.back();
		WinerID = 0;
	}
	// first
	m_nFirstID = GetNextPlayerByID(bankerid);
	// 设置参数
	m_nGameRound++;
	m_lCurJetton = base;
	// deal msg
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{ // 参与游戏用户
		auto &user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		
		int64_t nUserGold = user.p_asset;

		auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_game_start,zjh_protocols::e_mst_l2c_game_start);
		sendmsg->set_bankerid(bankerid);
		sendmsg->set_basegold(base);
		sendmsg->set_usergold(nUserGold);
		sendmsg->set_pool_gold(m_lTotalSliver);
		// state
		for (auto it2=m_MapTablePlayer.begin();it2!=m_MapTablePlayer.end();it2++)
		{
			auto &user = it2->second;
			sendmsg->add_state(user.p_active);
			sendmsg->add_stateid(user.p_id);
		}
		// card info
		auto cardinfo = sendmsg->mutable_cardinfo();
		if (user.p_active == FALSE)
		{
			cardinfo->clear_pokers();
			cardinfo->clear_pokertype();
			cardinfo->clear_pokerstatus();
		}
		else
		{
			if (player->is_robot())
			{
				cardinfo->clear_pokers();
				cardinfo->clear_pokertype();
				cardinfo->clear_pokerstatus();
			}
			else
			{
				cardinfo->mutable_pokers()->Reserve(MAX_CARDS_HAND);
				for (int j = 0; j < MAX_CARDS_HAND; j++)
				{
					cardinfo->add_pokers(0x00);
				}
				cardinfo->set_pokertype(0);
				cardinfo->set_pokerstatus(eCardState_An);
			}
		}
		if (player->is_robot())
		{
			robot_mgr::instance().recv_packet(player->get_pid(), sendmsg->packet_id(), sendmsg);
		}
		else
		{
			player->send_msg_to_client(sendmsg);
		}
	}
	// 计算发牌时间
	float fChipTime = GoldFlower_BaseInfo::GetSingleton()->GetData("ChipTime")->mValue/10.0;
	float fCardTime = GoldFlower_BaseInfo::GetSingleton()->GetData("CardTime")->mValue/10.0;
	float fPauseTime = GoldFlower_BaseInfo::GetSingleton()->GetData("PauseTime")->mValue/10.0;

	m_duration = fChipTime + m_VecActive.size()*MAX_CARDS_HAND*fCardTime+fPauseTime;
	// @取牌调控
// 	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
// 	{ // 
// 		auto user = it->second;
// 		auto player = user.p_playerptr;
// 		if (player == nullptr) continue;
// 		uint32_t playerid = player->get_pid();
// 		GameCard card;
// 		memset(&card, 0, sizeof(card));
// 		if (!m_GameCardMgr.GetUserCard(card, eCardlvl_Normal))
// 		{
// 			m_GameCardMgr.GetUserCard(card, eCardLvL_Random);
// 		}
// 		m_MapPlayerCard.insert(std::make_pair(playerid, card));
// 	}
	// @ 
	uint32_t killerid = PreSysRegulatoryPolicy();

	// @机器人获取其他玩家牌数据
	this->SendGameInfo2Robot(killerid);

	return true;
}

void zjh_space::logic_table::SendGameInfo2Robot(uint32_t killer)
{
	auto sendgameinfo = PACKET_CREATE(packetl2c_robot_gameinfo, e_mst_l2c_game_info);
	for (auto itcard = m_MapPlayerCard.begin(); itcard != m_MapPlayerCard.end(); itcard++)
	{
		auto id = itcard->first;
		auto cardinfo = itcard->second;
		auto playercards = sendgameinfo->add_playercard();
		playercards->set_playerid(id);
		auto msgcardinfo = playercards->mutable_cardinfo();
		for (int j = 0; j < MAX_CARDS_HAND; j++)
		{
			msgcardinfo->add_pokers(cardinfo.nCard[j]);
		}
		msgcardinfo->set_pokertype(cardinfo.nModel);
		if (id == killer)
		{
			msgcardinfo->set_pokerstatus(110);
		}
		else
		{
			msgcardinfo->clear_pokerstatus();
		}
	}
	//
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{ // 
		auto &user = it->second;
		auto player = user.p_playerptr;
		if (player == nullptr) continue;
		if (player->is_robot())
		{
			robot_mgr::instance().recv_packet(player->get_pid(), sendgameinfo->packet_id(), sendgameinfo);
		}
	}
}

bool logic_table::onGameNoticeStart()
{
	SLOG_CRITICAL<<"------------onGameNoticeStart------------";
	int32_t nGameState = get_status();
	if(nGameState != eGameState_FaPai)
	{
		SLOG_ERROR<<"onGameNoticeStart game state err: "<<nGameState;
		return false;
	}
	
	// check banker
	uint32_t& firstid = m_nFirstID;
	SLOG_CRITICAL << boost::format("%1%:%2% firstid:%3%") % __FUNCTION__%__LINE__%firstid;
	auto it = m_MapTablePlayer.find(firstid);
	if(it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"onGameNoticeStart find next user err: "<<firstid;
		DisbandPlayer();
		return false;
	}
	set_status(eGameState_Play);
	m_nCurOperatorID = firstid;
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_notice_start, zjh_protocols::e_mst_l2c_notice_start);
	sendmsg->set_playerid(firstid);
	broadcast_msg_to_client2(sendmsg);

	m_duration = m_OperaTime;

	return true;
}

uint32_t zjh_space::logic_table::GetNextPlayerByID(uint32_t playerid)
{
	SLOG_CRITICAL<<"GetNextPlayerByID playerid:"<<playerid;
	uint32_t nextid = 0;
	int32_t seat = get_seat_byid(playerid);
	int32_t nextseat = INVALID_CHAIR;
	int32_t fseat = seat;
	do 
	{
		if(nextseat == seat) break;
#if 0
		nextseat = (fseat-1+GAME_PLAYER)%GAME_PLAYER;
#else
		nextseat = (fseat+1)%GAME_PLAYER;
#endif
		auto itfind = std::find(m_VecChairMgr.begin(),m_VecChairMgr.end(),nextseat);
		if(itfind==m_VecChairMgr.end())
		{
			uint32_t findid = get_id_byseat(nextseat);
			auto it2 = std::find(m_VecPlaying.begin(),m_VecPlaying.end(),findid);
			if(it2!=m_VecPlaying.end()) break;
		}
		fseat = nextseat;
	} while (TRUE);

	nextid = get_id_byseat(nextseat);
	SLOG_CRITICAL<<"GetNextPlayerByID nextid:"<<nextid;
	return nextid;
}


// bool zjh_space::logic_table::isVaildSeat(uint16_t wChair)
// {
// 	if(wChair>=0 && wChair<GAME_PLAYER)
// 		return true;
// 	return false;
// }

bool zjh_space::logic_table::onGameGiveUp(uint32_t playerid,bool bTimeOver/*=false*/)
{
	/*
	* 弃牌在游戏中任何时段均可以操作
	*/
	SLOG_CRITICAL<<boost::format("%1%:%2% playerid:%3%")%__FUNCTION__%__LINE__%playerid;
	// check player
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"onGameGiveUp player err";
		return false;
	}
	auto &user =  player_it->second;
	auto player = user.p_playerptr;
	if(player && player->is_robot())
	{
		SLOG_CRITICAL<<boost::format("%1%:%2% Robot-id:%3%")%__FUNCTION__%__LINE__%playerid;
	}
	// check game state
	int32_t nGameState= get_status();
	if( nGameState !=eGameState_Play)
	{
		SLOG_ERROR<<boost::format("%1%:%2% game state err:%3%")%__FUNCTION__%__LINE__%nGameState;
		return false;
	}
	// check state
	auto playing_it = std::find(m_VecPlaying.begin(),m_VecPlaying.end(),playerid);
	if(playing_it==m_VecPlaying.end()) 
	{
		SLOG_ERROR<<"onGameGiveUp user state err";
		return false;
	}
	// clear
	m_VecPlaying.erase(playing_it);
	// clear
	auto allin_it = std::find(std::begin(m_VecAllin),std::end(m_VecAllin),playerid);
	if(allin_it != m_VecAllin.end())
	{
		m_VecAllin.erase(allin_it);
	}
	// set
	user.p_playing = FALSE;
	user.p_cardflag |= eCardState_Qi;
	if (player) player->set_status(eUserState_dead);
	//
#ifdef UsingLog
	tagOptInfo OptInfo;
	OptInfo.nOpt = eOperator_GiveUp;
	OptInfo.round = m_nGameRound;
	OptInfo.opttime = time_helper::instance().get_cur_time();
	save_gamelog(playerid, OptInfo);
#endif // UsingLog
	//
	if(bTimeOver==true)
	{
		SLOG_DEBUG<<"Time Over:"<<playerid;
		user.p_noaction += 1;
	}

	//@判断当前弃牌用户如果是当前用户,则重置牌局定时器为0;
	if (playerid == m_nCurOperatorID)
	{
		m_duration = 0;
	}
	
	if(m_VecPlaying.size()==1)
	{
		auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_operator_qi, zjh_protocols::e_mst_l2c_operator_qi_result);
		sendmsg->set_playerid(playerid);
		sendmsg->clear_nextid();
		sendmsg->clear_round();
		broadcast_msg_to_client2(sendmsg);
		// 幸存者
		uint32_t nextid = m_VecPlaying.back();
		onGameEnd(nextid,eGameEndType_OnlyOne);
	}
	else
	{
		auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_operator_qi, zjh_protocols::e_mst_l2c_operator_qi_result);
		sendmsg->set_playerid(playerid);
		if(m_nCurOperatorID == playerid)
		{
			uint32_t nextid = GetNextPlayerByID(playerid);
			if(nextid==0 )
			{
				SLOG_ERROR<<"onGameGiveUp find next user err: ";
				DisbandPlayer();
				return false;
			}
			m_nCurOperatorID = nextid;
			// 弃牌的下一家为起始用户
			if (playerid == m_nFirstID)
			{
				m_nFirstID = GetNextPlayerByID(playerid);
			}
			else
			{
				// 
				isNewRound();
			}
			
			
			sendmsg->set_nextid(nextid);
			sendmsg->set_round(m_nGameRound);
			//
			broadcast_msg_to_client2(sendmsg);
			//
			m_duration = m_OperaTime;
			//
			// 检测此玩家是否达到孤掷一注
			checkOutofBound(nextid);
			// 检测是否达到房间上限--强制全比
			checkOutofRange();
		}
		else
		{
			// 弃牌的下一家为起始用户
			if (playerid == m_nFirstID)
			{
				m_nFirstID = GetNextPlayerByID(playerid);
			}
			sendmsg->clear_nextid();
			sendmsg->clear_round();
			//
			broadcast_msg_to_client2(sendmsg);
		}
	}

	return true;
}

bool zjh_space::logic_table::onGameCheck(uint32_t playerid)
{
	/*
	*看牌在游戏中任何时段均可以操作
	*/
	SLOG_CRITICAL<<boost::format("%1%:%2% playerid:%3%")%__FUNCTION__%__LINE__%playerid;
	// check player
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"onGameCheck player err";
		return false;
	}
	auto &user =  player_it->second;
	auto player = user.p_playerptr;
	if(player && player->is_robot())
	{
		SLOG_CRITICAL<<boost::format("%1%:%2% Robot-id:%3%")%__FUNCTION__%__LINE__%playerid;
	}
	// check game state
	int32_t nGameState= get_status();
	if( nGameState !=eGameState_Play)
	{
		SLOG_ERROR<<boost::format("%1%:%2% game state err:%3%")%__FUNCTION__%__LINE__%nGameState;
		return false;
	}
	// check state
	auto playing_it = std::find(m_VecPlaying.begin(),m_VecPlaying.end(),playerid);
	if(playing_it==m_VecPlaying.end()) 
	{
		SLOG_ERROR<<"onGameCheck user state err";
		return false;
	}
	// check look
	if (m_nGameRound < m_room->get_data()->mLookCondition)
	{
		SLOG_ERROR << boost::format("onGameCheck round:%1%") % m_nGameRound;
		return false;
	}
	//
	int32_t &cflag = user.p_cardflag;
	if (cflag != eCardState_An)
	{
		SLOG_ERROR<<"onGameCheck card status err :"<<cflag;
		return false;
	}
	// set card flag
	cflag = eCardState_Ming;
	// set no action
	CleanNoAction(playerid);
	//
	m_duration = 0;
	// get card
	GameCard card;
	auto fresult = m_MapPlayerCard.find(playerid);
	if (fresult != m_MapPlayerCard.end())
	{
		card = fresult->second;
	}
	//
	//bool bGiveUp = SysRegulatoryCheckCard(playerid);
	//////////////////////////////////////////////////////////////////////////
#ifdef UsingLog
	tagOptInfo OptInfo;
	OptInfo.nOpt = eOperator_Check;
	OptInfo.round = m_nGameRound;
	OptInfo.opttime = time_helper::instance().get_cur_time();
	save_gamelog(playerid, OptInfo);
#endif // UsingLog
	//
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if(player!=nullptr)
		{
			auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_operator_kan, zjh_protocols::e_mst_l2c_operator_kan_result);
			sendmsg->set_playerid(playerid);
			auto cardinfo = sendmsg->mutable_cardinfo();
			if(player->get_pid()==playerid)
			{
				cardinfo->mutable_pokers()->Reserve(MAX_CARDS_HAND);
				for (int j=0;j<MAX_CARDS_HAND;j++)
				{

					cardinfo->add_pokers(card.nCard[j]);
					SLOG_CRITICAL << boost::format("[card]uid:%d,data:%02x,%02x,%02x") \
						%playerid%card.nCard[0]%card.nCard[1]%card.nCard[2];
				}
				int nCardModel = card.nModel;
				cardinfo->set_pokertype(nCardModel);
				cardinfo->set_pokerstatus(cflag);
			}
			else
			{
				sendmsg->clear_cardinfo();
				//	cardinfo->clear_pokers();
				//	cardinfo->clear_pokertype();
			}

			if(player->is_robot())
			{
				robot_mgr::instance().recv_packet(player->get_pid(),sendmsg->packet_id(),sendmsg);
			}
			else
			{
				player->send_msg_to_client(sendmsg);
			}

		}
	}
	
	// 设定时器
	m_duration = m_OperaTime;
	//
	if(playerid==m_nCurOperatorID)
	{
		// 检测此玩家是否达到孤掷一注
		checkOutofBound(playerid);
	}
	
	
	return true;
}

bool zjh_space::logic_table::onGameFlow(uint32_t playerid,int64_t lGold)
{
	SLOG_CRITICAL<<boost::format("%1%:%2% playerid:%3%,gold:%4%")%__FUNCTION__%__LINE__%playerid%lGold;
	if(playerid!=m_nCurOperatorID)
	{
		SLOG_ERROR<<"onGameFlow playerid err";
		return false;
	}
	// check player
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"onGameFlow player err";
		return false;
	}
	auto &user =  player_it->second;
	auto player = user.p_playerptr;
	if(player && player->is_robot())
	{
		SLOG_CRITICAL<<boost::format("%1%:%2% Robot-id:%3%")%__FUNCTION__%__LINE__%playerid;
	}
	// check game state
	int32_t nGameState= get_status();
	if( nGameState !=eGameState_Play)
	{
		SLOG_ERROR<<boost::format("%1%:%2% game state err:%3%")%__FUNCTION__%__LINE__%nGameState;
		return false;
	}
	// check state
	auto playing_it = std::find(m_VecPlaying.begin(),m_VecPlaying.end(),playerid);
	if(playing_it==m_VecPlaying.end()) 
	{
		SLOG_ERROR<<"onGameFlow user state err";
		return false;
	}
	//
	int32_t cflag = user.p_cardflag;
	int64_t & selfgold = user.p_asset;
	int64_t & expend = user.p_expend;
	// 
	// 参数检查
	int64_t lNeedGold = m_lCurJetton;
	if(cflag==eCardState_Ming)
	{
		lNeedGold *= 2;
	}
	if(lGold!=lNeedGold)
	{
		SLOG_ERROR<<"onGameFlow gold err";
		return false;
	}
	// 金币验证
	if (selfgold < lNeedGold )
	{
		SLOG_CRITICAL<<boost::format("selfgold:%1%,need:%2%")%selfgold%lNeedGold;
		return false;
	}
	// 梭哈
	if(m_nShowHandCnt!=0)
	{
		SLOG_ERROR<<"onGameFlow show-hand state";
		return false;
	}
	// del 10/1/2017 
// 	if(m_VecAllin.size() > 0)
// 	{
// 		SLOG_ERROR<<"onGameFlow all-in state";
// 		return false;
// 	}
	// set no action
	CleanNoAction(playerid);
	//
	m_duration = 0;
	// 设置参数
	expend += lNeedGold;
	m_lTotalSliver += lNeedGold;
	selfgold -= lNeedGold;
	//
#ifdef UsingLog
	tagOptInfo OptInfo;
	OptInfo.nOpt = eOperator_Flow;
	OptInfo.vargold = lNeedGold;
	OptInfo.round = m_nGameRound;
	OptInfo.opttime = time_helper::instance().get_cur_time();
	save_gamelog(playerid, OptInfo);
#endif // UsingLog
	//
	uint32_t nextid = GetNextPlayerByID(playerid);
	if(nextid==0)
	{
		SLOG_ERROR<<"onGameFlow find next user err: ";
		DisbandPlayer();
		return false;
	}
	m_nCurOperatorID = nextid;
	// 
	isNewRound();

	auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_operator_gen, zjh_protocols::e_mst_l2c_operator_gen_result);
	sendmsg->set_playerid(playerid);
	sendmsg->set_player_gold(selfgold);
	sendmsg->set_operator_gold(lNeedGold);
	sendmsg->set_pool_gold(m_lTotalSliver);
	sendmsg->set_nextid(nextid);
	sendmsg->set_round(m_nGameRound);
	broadcast_msg_to_client2(sendmsg);

	// 设定时器
	m_duration = m_OperaTime;
	// 检测此玩家是否达到孤掷一注
	checkOutofBound(nextid);
	// 检测是否达到房间上限--强制全比
	checkOutofRange();

	return true;
}

bool zjh_space::logic_table::onGameAdd(uint32_t playerid,int64_t lGold)
{
	SLOG_CRITICAL<<boost::format("%1%:%2% playerid:%3%,gold:%4%")%__FUNCTION__%__LINE__%playerid%lGold;
	if(playerid!=m_nCurOperatorID)
	{
		SLOG_ERROR<<"onGameAdd playerid err";
		return false;
	}
	// check player
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"onGameAdd player err";
		return false;
	}
	auto &user =  player_it->second;
	auto player = user.p_playerptr;
	if(player && player->is_robot())
	{
		SLOG_CRITICAL<<boost::format("%1%:%2% Robot-id:%3%")%__FUNCTION__%__LINE__%playerid;
	}
	// check game state
	int32_t nGameState= get_status();
	if( nGameState !=eGameState_Play)
	{
		SLOG_ERROR<<boost::format("%1%:%2% game state err:%3%")%__FUNCTION__%__LINE__%nGameState;
		return false;
	}
	// check state
	auto playing_it = std::find(m_VecPlaying.begin(),m_VecPlaying.end(),playerid);
	if(playing_it==m_VecPlaying.end()) 
	{
		SLOG_ERROR<<"onGameAdd user state err";
		return false;
	}
	//
	int32_t cflag = user.p_cardflag;
	int64_t & selfgold = user.p_asset;
	int64_t & expend = user.p_expend;
	// 金币验证
	if(!isVaildGold(lGold))
	{
		SLOG_ERROR<<"onGameAdd invaild-gold: "<<lGold;
		return false;
	}
	if (lGold > m_room->get_data()->mMaxChip)
	{
		SLOG_ERROR<<"onGameAdd up-limit";
		return false;
	}
	int64_t lNeedGold = lGold;
	// 金币验证
	if(cflag==eCardState_Ming)
	{
		lNeedGold = lGold * 2;
	}
	if (selfgold < lNeedGold )
	{
		SLOG_CRITICAL<<boost::format("selfgold:%1%,need:%2%")%selfgold%lNeedGold;
		return false;
	}
	if(m_nShowHandCnt!=0)
	{
		SLOG_ERROR<<"onGameAdd show-hand state";
		return false;
	}
	// del 10/1/2017 :
// 	if(m_VecAllin.size() > 0)
// 	{
// 		SLOG_ERROR<<"onGameAdd all-in state";
// 		return false;
// 	}
	// set no action
	CleanNoAction(playerid);
	// 加注成功
	m_duration = 0;
	m_lCurJetton = lGold;
	// 设置参数
	expend += lNeedGold;
	m_lTotalSliver += lNeedGold;
	selfgold -= lNeedGold;
	//
#ifdef UsingLog
	tagOptInfo OptInfo;
	OptInfo.nOpt = eOperator_Add;
	OptInfo.vargold = lNeedGold;
	OptInfo.round = m_nGameRound;
	OptInfo.opttime = time_helper::instance().get_cur_time();
	save_gamelog(playerid, OptInfo);
#endif // UsingLog
	// 
	uint32_t nextid = GetNextPlayerByID(playerid);
	if(nextid==0)
	{
		SLOG_ERROR<<"onGameAdd find next user err: ";
		DisbandPlayer();
		return false;
	}
	m_nCurOperatorID = nextid;

	isNewRound();
	//
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_operator_jia, zjh_protocols::e_mst_l2c_operator_jia_result);
	sendmsg->set_playerid(playerid);
	sendmsg->set_player_gold(selfgold);
	sendmsg->set_operator_gold(lNeedGold);
	sendmsg->set_cur_jetton(m_lCurJetton);
	sendmsg->set_pool_gold(m_lTotalSliver);
	sendmsg->set_nextid(nextid);
	sendmsg->set_round(m_nGameRound);
	broadcast_msg_to_client2(sendmsg);

	// 设定时器
	m_duration = m_OperaTime;
	// 检测此玩家是否达到孤掷一注
	checkOutofBound(nextid);
	// 检测是否达到房间上限--强制全比
	checkOutofRange();

	return true;
}

bool zjh_space::logic_table::onGameAllPK(int nCount)
{
	SLOG_CRITICAL<<boost::format("%1%:%2%")%__FUNCTION__%__LINE__;

	uint32_t nWinerID = CompareWinnerEx();
	if (nWinerID==0) return false;
	m_nLastWiner = nWinerID;
	SLOG_CRITICAL<<boost::format("onGameAllPK Winer:%1%")%nWinerID;
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_all_pk, zjh_protocols::e_mst_l2c_operator_allpk);
	sendmsg->set_windid(nWinerID);
	broadcast_msg_to_client2(sendmsg);
	// set operator flag
	SetCompareNexus(m_VecPlaying);
	// GameOver
	int nEndType = eGameEndType_AllPK;
	if(nCount==2)
		nEndType = eGameEndType_Compare;
	onGameEnd(nWinerID,nEndType);

	return true;
}

bool zjh_space::logic_table::onGameCompare(uint32_t reqid,uint32_t resid)
{
	SLOG_CRITICAL<<boost::format("%1%:%2% reqid:%3%,resid:%4%")%__FUNCTION__%__LINE__%reqid%resid;
	// 用户验证
	if(reqid!=m_nCurOperatorID)
	{
		SLOG_ERROR<<"onGameCompare playerid err";
		return false;
	}
	// check player
	auto req_it = m_MapTablePlayer.find(reqid);
	if(req_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"onGameCompare require player err";
		return false;
	}
	auto&user = req_it->second;
	auto player = user.p_playerptr;
	if(player && player->is_robot())
	{
		SLOG_CRITICAL<<boost::format("%1%:%2% Robot-id:%3%")%__FUNCTION__%__LINE__%reqid;
	}
	auto res_it = m_MapTablePlayer.find(resid);
	if(res_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"onGameCompare response player err";
		return false;
	}
	if(reqid==resid) return true;
	// check game state
	int32_t nGameState= get_status();
	if( nGameState !=eGameState_Play)
	{
		SLOG_ERROR<<boost::format("%1%:%2% game state err:%3%")%__FUNCTION__%__LINE__%nGameState;
		return false;
	}
	// check state
	auto playingreq_it = std::find(m_VecPlaying.begin(),m_VecPlaying.end(),reqid);
	if(playingreq_it==m_VecPlaying.end()) 
	{
		SLOG_ERROR<<"onGameCompare require state err";
		return false;
	}
	auto playingres_it = std::find(m_VecPlaying.begin(),m_VecPlaying.end(),resid);
	if(playingres_it==m_VecPlaying.end()) 
	{
		SLOG_ERROR<<"onGameCompare response state err";
		return false;
	}
	//
	int32_t cflag = user.p_cardflag;
	int64_t & selfgold = user.p_asset;
	int64_t & expend = user.p_expend;

	int64_t lNeedGold = m_lCurJetton;
	// 金币验证
	if(cflag==eCardState_Ming)
	{
		lNeedGold *= 2;
	}
	else
	{
		lNeedGold *= 1;
	}
	if(selfgold<lNeedGold)
	{
		SLOG_ERROR<<boost::format("onGameCompare selfgold:%1%,need:%2%")%selfgold%lNeedGold;;
		return false;
	}
	if(m_nGameRound < m_room->get_data()->mPkCondition)
	{
		SLOG_ERROR<<boost::format("onGameCompare pk round:%1%")%m_nGameRound;
		return false;
	}
	if(m_nShowHandCnt!=0)
	{
		SLOG_ERROR<<"onGameCompare show-hand state";
		return false;
	}
	// del 10/1/2017 : 选人比牌必须保证足够金币
// 	if(m_VecAllin.size() > 0)
// 	{
// 		SLOG_ERROR<<"onGameCompare all-in state";
// 		return false;
// 	}
	// set no action
	CleanNoAction(reqid);
	//
	m_duration = 0;
	// 设置参数
	expend += lNeedGold;
	m_lTotalSliver += lNeedGold;
	selfgold -= lNeedGold;
	//
#ifdef UsingLog
	tagOptInfo OptInfo;
	OptInfo.nOpt = eOperator_Compare;
	OptInfo.vargold = lNeedGold;
	OptInfo.targetID = resid;
	OptInfo.round = m_nGameRound;
	OptInfo.opttime = time_helper::instance().get_cur_time();
	save_gamelog(reqid, OptInfo);
#endif // UsingLog
	//
	uint32_t nWinerID = CompareWinner(resid,reqid);
	uint32_t nLostID = 0;
	if(nWinerID==resid)
	{
		nLostID = reqid;
	}
	else
	{
		nLostID = resid;
	}
	m_nLastWiner = nWinerID;
	SLOG_CRITICAL<<boost::format("onGameCompare Winer:%1%")%nWinerID;
	// set operator flag
	SetCompareNexus(reqid,resid);
	// 设置输
	auto lstplayer_it = m_MapTablePlayer.find(nLostID);
	if(lstplayer_it != m_MapTablePlayer.end())
	{
		auto &user = lstplayer_it->second;
		user.p_cardflag |= eCardState_Shu;
		user.p_playing = FALSE;
		auto player = user.p_playerptr;
		if(player)
		{
			player->set_status(eUserState_dead);
			// @等待比牌动画结束
			m_checkdelay = Compare_Action;
			m_uCompareID = nLostID;
		}
	}
	// 清理输
	auto lst_it = std::find(m_VecPlaying.begin(),m_VecPlaying.end(),nLostID);
	if(lst_it != m_VecPlaying.end())
	{
		m_VecPlaying.erase(lst_it);
	}
	// send compare result
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_operator_bi, zjh_protocols::e_mst_l2c_operator_bi_result);
	sendmsg->set_requestid(reqid);
	sendmsg->set_player_gold(selfgold);
	sendmsg->set_operator_gold(lNeedGold);
	sendmsg->set_compareid(resid);
	sendmsg->set_winid(nWinerID);
	sendmsg->set_pool_gold(m_lTotalSliver);
	if(m_VecPlaying.size()==1)
	{
		sendmsg->clear_nextid();
		sendmsg->clear_round();
	}
	else
	{
		uint32_t nextid = GetNextPlayerByID(reqid);
		if(nextid==0)
		{
			SLOG_ERROR<<"onGameCompare find next user err: ";
			DisbandPlayer();
			return false;
		}
		m_nCurOperatorID = nextid;
		// change first
		if(nLostID==m_nFirstID)
		{
			m_nFirstID = GetNextPlayerByID(nLostID);
		}
		isNewRound();
		sendmsg->set_nextid(nextid);
		sendmsg->set_round(m_nGameRound);
	}
	broadcast_msg_to_client2(sendmsg);

	// 
	if(m_VecPlaying.size()==1)
	{
		// GameOver
		onGameEnd(nWinerID,eGameEndType_Compare);
	}
	else
	{
		m_duration = TIME_INTERVAL + m_OperaTime;
		// 检测此玩家是否达到孤掷一注
		checkOutofBound(m_nCurOperatorID);
		// 检测是否达到房间上限--强制全比
		checkOutofRange();
	}

	return true;
}

bool zjh_space::logic_table::onGameCompareEx(uint32_t playerid)
{
	// 金币不足比牌---孤注一掷
	SLOG_CRITICAL<<boost::format("%1%:%2% playerid:%3%")%__FUNCTION__%__LINE__%playerid;
	if(playerid!=m_nCurOperatorID)
	{
		SLOG_ERROR<<"onGameCompareEx playerid err";
		return false;
	}
	// check player
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"onGameCompareEx player err";
		return false;
	}
	auto&user = player_it->second;
	auto player = user.p_playerptr;
	if(player && player->is_robot())
	{
		SLOG_CRITICAL<<boost::format("%1%:%2% Robot-id:%3%")%__FUNCTION__%__LINE__%playerid;
	}
	// check game state
	int32_t nGameState= get_status();
	if( nGameState !=eGameState_Play)
	{
		SLOG_ERROR<<boost::format("%1%:%2% game state err:%3%")%__FUNCTION__%__LINE__%nGameState;
		return false;
	}
	// check state
	auto playing_it = std::find(m_VecPlaying.begin(),m_VecPlaying.end(),playerid);
	if(playing_it==m_VecPlaying.end()) 
	{
		SLOG_ERROR<<"onGameCompareEx user state err";
		return false;
	}
	//add clear 1/10/2017
	auto fit = std::find(m_VecAllin.begin(), m_VecAllin.end(), playerid);
	if (fit != m_VecAllin.end())
	{	
		m_VecAllin.erase(fit);
	}
	//
	int32_t cflag = user.p_cardflag;
	int64_t & selfgold = user.p_asset;
	int64_t & expend = user.p_expend;
	// 
	if(selfgold <= 0)
	{
		SLOG_ERROR<<boost::format("onGameCompareEx gold-invalid playerid:%1%")%playerid;
		return false;
	}
	// set no action
	CleanNoAction(playerid);

	m_duration = 0;

	int64_t self_gold = selfgold;
	m_lTotalSliver += selfgold;
	expend += selfgold;
	selfgold -= selfgold;
	//
#ifdef UsingLog
	tagOptInfo OptInfo;
	OptInfo.nOpt = eOperator_Allin;
	OptInfo.vargold = selfgold;
	OptInfo.round = m_nGameRound;
	OptInfo.opttime = time_helper::instance().get_cur_time();
	save_gamelog(playerid, OptInfo);
#endif // UsingLog
	//
	uint32_t nWinerID = CompareWinnerEx();
	if (nWinerID==0) return false;
	m_nLastWiner = nWinerID;
	SLOG_CRITICAL<<boost::format("onGameCompareEx Winer:%1%")%nWinerID;
	// set operator flag 
	//SetCompareNexus(m_VecPlaying);
	for (auto id : m_VecPlaying)
	{
		SetCompareNexus(playerid,id);
	}

	if(nWinerID==playerid)
	{ // 此时游戏结束
		auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_operator_allin, zjh_protocols::e_mst_l2c_operator_allin);
		sendmsg->set_playerid(playerid);
		sendmsg->set_operator_gold(/*selfgold*/self_gold);
		sendmsg->set_pool_gold(m_lTotalSliver);
		sendmsg->set_bwin(true);
		sendmsg->clear_nextid();
		sendmsg->clear_round();
		broadcast_msg_to_client2(sendmsg);

		// 
		onGameEnd(nWinerID,eGameEndType_AllIN);
	}
	else
	{ // 剩余玩家超过2个
		//
		m_VecPlaying.erase(playing_it);
		// set
		auto &user =  player_it->second;
		user.p_playing = FALSE;
		user.p_cardflag |= eCardState_Qi;
		auto player = user.p_playerptr;
		if (player)
		{
			player->set_status(eUserState_dead);
			// @等待比牌动画结束
			m_checkdelay = Compare_Action;
			m_uCompareID = playerid;
		}
		if(m_VecPlaying.size()==1)
		{// 剩余玩家
			auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_operator_allin, zjh_protocols::e_mst_l2c_operator_allin);
			sendmsg->set_playerid(playerid);
			sendmsg->set_operator_gold(/*selfgold*/self_gold);
			sendmsg->set_pool_gold(m_lTotalSliver);
			sendmsg->set_bwin(false);
			sendmsg->clear_nextid();
			sendmsg->clear_round();
			broadcast_msg_to_client2(sendmsg);

			// 
			onGameEnd(nWinerID,eGameEndType_AllIN);
			return true;
		}

		uint32_t nextid = GetNextPlayerByID(playerid);
		if(nextid==0)
		{
			SLOG_ERROR<<"onGameCompareEx find next user err: ";
			DisbandPlayer();
			return false;
		}
		m_nCurOperatorID = nextid;
		// change first
		if(playerid==m_nFirstID)
		{
			m_nFirstID = GetNextPlayerByID(playerid);
		}
		isNewRound();
		auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_operator_allin, zjh_protocols::e_mst_l2c_operator_allin);
		sendmsg->set_playerid(playerid);
		sendmsg->set_operator_gold(/*selfgold*/self_gold);
		sendmsg->set_pool_gold(m_lTotalSliver);
		sendmsg->set_bwin(false);
		sendmsg->set_nextid(nextid);
		sendmsg->set_round(m_nGameRound);
		broadcast_msg_to_client2(sendmsg);

		// 设定时器
		m_duration = m_OperaTime;
		// 检测此玩家是否达到孤掷一注
		checkOutofBound(nextid);
		// 检测是否达到房间上限--强制全比
		checkOutofRange();
	}

	return true;

}

bool zjh_space::logic_table::onGameShowHand(uint32_t playerid)
{
	SLOG_CRITICAL<<boost::format("%1%:%2% playerid:%3%")%__FUNCTION__%__LINE__%playerid;
	if(playerid!=m_nCurOperatorID)
	{
		SLOG_ERROR<<"onGameShowHand playerid err";
		return false;
	}
	// check player
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"onGameShowHand player err";
		return false;
	}
	auto&user = player_it->second;
	auto player = user.p_playerptr;
	if(player && player->is_robot())
	{
		SLOG_CRITICAL<<boost::format("%1%:%2% Robot-id:%3%")%__FUNCTION__%__LINE__%playerid;
	}
	// check game state
	int32_t nGameState= get_status();
	if( nGameState !=eGameState_Play)
	{
		SLOG_ERROR<<boost::format("%1%:%2% game state err:%3%")%__FUNCTION__%__LINE__%nGameState;
		return false;
	}
	// check state
	auto playing_it = std::find(m_VecPlaying.begin(),m_VecPlaying.end(),playerid);
	if(playing_it==m_VecPlaying.end()) 
	{
		SLOG_ERROR<<"onGameShowHand user state err";
		return false;
	}
	if(m_VecPlaying.size()!=2)
	{
		SLOG_ERROR<<"onGameShowHand user playing count err";
		return false;
	}
	//
	int32_t cflag = user.p_cardflag;
	int64_t & selfgold = user.p_asset;
	int64_t & expend = user.p_expend;
	// 
	if(selfgold<=0)
	{
		SLOG_ERROR<<boost::format("onGameShowHand gold-invalid playerid:%1%")%playerid;
		return false;
	}
	// del 9/29/2017 : 金币不足但可以梭哈
// 	if(m_VecAllin.size() > 0)
// 	{
// 		SLOG_ERROR<<"onGameShowHand all-in state";
// 		return false;
// 	}
	// set no action
	CleanNoAction(playerid);

	m_duration = 0;

	if (m_nShowHandCnt==0)
	{
		m_nShowHandCnt++;
		uint32_t nextid = GetNextPlayerByID(playerid);
		if(nextid==0) return false;
		//
#ifdef UsingLog
		tagOptInfo OptInfo;
		OptInfo.nOpt = eOperator_ShowHand;
		OptInfo.targetID = nextid;
		OptInfo.opttime = time_helper::instance().get_cur_time();
		save_gamelog(playerid, OptInfo);
#endif // UsingLog
		//
		m_nCurOperatorID = nextid;
		// 设置参数
		m_lInventPool = m_lTotalSliver + selfgold;

		auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_ask_operator_showhand_result, zjh_protocols::e_mst_l2c_ask_showhand_result);
		sendmsg->set_playerid(playerid);
		sendmsg->set_nextid(nextid);
		sendmsg->set_operator_gold(selfgold);
		sendmsg->set_pool_gold(m_lInventPool);
		broadcast_msg_to_client2(sendmsg);

		// 设定时器
		m_duration = m_OperaTime;

	}
	else if(m_nShowHandCnt==1)
	{
#ifdef UsingLog
		tagOptInfo OptInfo;
		OptInfo.nOpt = eOperator_ShowHand;
		OptInfo.opttime = time_helper::instance().get_cur_time();
		save_gamelog(playerid, OptInfo);
#endif // UsingLog
		m_nShowHandCnt++;
		uint32_t nextid = GetNextPlayerByID(playerid);
		if(nextid==0) return false;
		m_lInventPool = m_lTotalSliver + selfgold;
		//////////////////////////////////////////////////////////////////////////
		// 计算梭哈玩家实际投入值
		m_lEqualize = 0;
		uint32_t lesssid = 0;
		int64_t lLessGold = GetLessGold(lesssid);
		assert(lesssid != 0);

		auto less_it = m_MapTablePlayer.find(lesssid);
		if (less_it == m_MapTablePlayer.end())
		{
			assert(0);
			return false;
		}
		auto &lessusser = less_it->second;
		// 需要补偿
		if (lessusser.p_cardflag == eCardState_Ming)
		{
			if (lLessGold % 2 != 0)
			{
				m_lEqualize = 1;
				lLessGold -= 1;
			}
			// 
			lessusser.p_asset -= m_lEqualize;
			lessusser.p_expend += m_lEqualize;
		}
		//
		if (lessusser.p_cardflag == eCardState_Ming)
		{
			m_lCurJetton = lLessGold / 2;
		}
		else
		{
			m_lCurJetton = lLessGold;
		}
		// 计算梭哈玩家实际支出 实际投入量
		for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
		{
			auto &user = it->second;
			if (user.p_playing == FALSE) continue;
			if (user.p_cardflag == eCardState_Ming)
			{
				user.p_asset -= m_lCurJetton * 2;
				user.p_expend += m_lCurJetton * 2;
				m_lTotalSliver += m_lCurJetton * 2;
			}
			else
			{
				user.p_asset -= m_lCurJetton * 1;
				user.p_expend += m_lCurJetton * 1;
				m_lTotalSliver += m_lCurJetton * 1;
			}
		}
		//////////////////////////////////////////////////////////////////////////
		uint32_t nWinerID = CompareWinner(nextid,playerid);
		m_nLastWiner = nWinerID;
		uint32_t nLostID = 0;
		if(nWinerID==nextid)
		{
			nLostID = playerid;
		}
		else
		{
			nLostID = nextid;
		}
		// set operator flags
		SetCompareNexus(nextid,playerid);
		// 设置输
// 		auto lstplayer_it = m_MapTablePlayer.find(nLostID);
// 		if(lstplayer_it != m_MapTablePlayer.end())
// 		{
// 			auto &user = lstplayer_it->second;
// 			user.p_cardflag = eCardState_Shu;
// 			user.p_playing = FALSE;
// 		}
// 		// 清理输
// 		auto lst_it = std::find(m_VecPlaying.begin(),m_VecPlaying.end(),nLostID);
// 		if(lst_it != m_VecPlaying.end())
// 		{
// 			m_VecPlaying.erase(lst_it);
// 		}
		auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_operator_showhand, zjh_protocols::e_mst_l2c_operator_showhand_result);
		sendmsg->set_requestid(nextid);
		sendmsg->set_respondid(playerid);
		sendmsg->set_respond_gold(selfgold);
		sendmsg->set_pool_gold(m_lInventPool);
		sendmsg->set_winid(nWinerID);
		broadcast_msg_to_client2(sendmsg);
		// 
		onGameEnd(nWinerID,eGameEndType_ShowHand);
	}
	else
	{
		assert(0);
	}
	return true;
}

bool zjh_space::logic_table::onGameAllIn(uint32_t playerid)
{
	SLOG_CRITICAL<<boost::format("%1%:%2% playerid:%3%")%__FUNCTION__%__LINE__%playerid;
	if(playerid!=m_nCurOperatorID)
	{
		SLOG_ERROR<<"onGameAllIn playerid err";
		return false;
	}
	// check player
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"onGameAllIn player err";
		return false;
	}
	auto&user = player_it->second;
	auto player = user.p_playerptr;
	if(player && player->is_robot())
	{
		SLOG_CRITICAL<<boost::format("%1%:%2% Robot-id:%3%")%__FUNCTION__%__LINE__%playerid;
	}
	// check game state
	int32_t nGameState= get_status();
	if( nGameState !=eGameState_Play)
	{
		SLOG_ERROR<<boost::format("%1%:%2% game state err:%3%")%__FUNCTION__%__LINE__%nGameState;
		return false;
	}
	// check state
	auto playing_it = std::find(m_VecPlaying.begin(),m_VecPlaying.end(),playerid);
	if(playing_it==m_VecPlaying.end()) 
	{
		SLOG_ERROR<<"onGameAllIn user state err";
		return false;
	}
	//
	int32_t cflag = user.p_cardflag;
	int64_t & selfgold = user.p_asset;
	int64_t & expend = user.p_expend;
	// 
	if(selfgold < 0)
	{
		SLOG_ERROR<<boost::format("onGameAllIn gold-invalid playerid:%1%")%playerid;
		return false;
	}
	if(m_VecAllin.size() == 0)
	{
		SLOG_ERROR<<boost::format("onGameAllIn opera-invalid");
		return false;
	}
	auto fit = std::find(m_VecAllin.begin(),m_VecAllin.end(),playerid);
	if(fit == m_VecAllin.end())
	{
		SLOG_ERROR<<boost::format("onGameAllIn playerid-invalid :%1%")%playerid;
		return false;
	}
	m_VecAllin.erase(fit);
	// set no action
	CleanNoAction(playerid);

	m_duration = 0;

	int64_t self_gold = selfgold;
	m_lTotalSliver += selfgold;
	expend += selfgold;
	selfgold -= selfgold;
#ifdef UsingLog
	tagOptInfo OptInfo;
	OptInfo.nOpt = eOperator_Allin;
	OptInfo.vargold = selfgold;
	OptInfo.round = m_nGameRound;
	OptInfo.opttime = time_helper::instance().get_cur_time();
	save_gamelog(playerid, OptInfo);
#endif // UsingLog
	//
	uint32_t nWinerID = CompareWinnerEx();
	if (nWinerID==0) return false;
	m_nLastWiner = nWinerID;
	SLOG_CRITICAL<<boost::format("onGameAllIn Winer:%1%")%nWinerID;
	// set operator flag 
	//SetCompareNexus(m_VecPlaying);
	for (auto id : m_VecPlaying)
	{
		SetCompareNexus(playerid,id);
	}

	if(nWinerID==playerid)
	{ // 此时游戏结束
		auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_operator_allin, zjh_protocols::e_mst_l2c_operator_allin);
		sendmsg->set_playerid(playerid);
		sendmsg->set_operator_gold(/*selfgold*/self_gold);
		sendmsg->set_pool_gold(m_lTotalSliver);
		sendmsg->set_bwin(true);
		sendmsg->clear_nextid();
		sendmsg->clear_round();
		broadcast_msg_to_client2(sendmsg);

		// 
		onGameEnd(nWinerID,eGameEndType_AllIN);
	}
	else
	{ // 剩余玩家超过2个
		//
		m_VecPlaying.erase(playing_it);
		// set
		auto &user =  player_it->second;
		user.p_playing = FALSE;
		user.p_cardflag |= eCardState_Qi;
		auto player = user.p_playerptr;
		if (player)
		{
			player->set_status(eUserState_dead);
			// @等待比牌动画结束
			m_checkdelay = Compare_Action;
			m_uCompareID = playerid;
		}
		if(m_VecPlaying.size()==1)
		{// 剩余玩家
			auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_operator_allin, zjh_protocols::e_mst_l2c_operator_allin);
			sendmsg->set_playerid(playerid);
			sendmsg->set_operator_gold(/*selfgold*/self_gold);
			sendmsg->set_pool_gold(m_lTotalSliver);
			sendmsg->set_bwin(false);
			sendmsg->clear_nextid();
			sendmsg->clear_round();
			broadcast_msg_to_client2(sendmsg);

			// 
			onGameEnd(nWinerID,eGameEndType_AllIN);
			return true;
		}

		uint32_t nextid = GetNextPlayerByID(playerid);
		if(nextid==0)
		{
			SLOG_ERROR<<"onGameAllIn find next user err: ";
			DisbandPlayer();
			return false;
		}
		m_nCurOperatorID = nextid;
		// change first
		if(playerid==m_nFirstID)
		{
			m_nFirstID = GetNextPlayerByID(playerid);
		}
		isNewRound();
		auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_operator_allin, zjh_protocols::e_mst_l2c_operator_allin);
		sendmsg->set_playerid(playerid);
		sendmsg->set_operator_gold(/*selfgold*/self_gold);
		sendmsg->set_pool_gold(m_lTotalSliver);
		sendmsg->set_bwin(false);
 		sendmsg->set_nextid(nextid);
 		sendmsg->set_round(m_nGameRound);
		broadcast_msg_to_client2(sendmsg);

		// 设定时器
		m_duration = m_OperaTime;
		// 检测此玩家是否达到孤掷一注
		checkOutofBound(nextid);
		// 检测是否达到房间上限--强制全比
		checkOutofRange();
	}

	return true;
}

bool zjh_space::logic_table::onGameEnd(uint32_t playerid,int nEndType)
{
	SLOG_CRITICAL<<boost::format("%1%:%2% winid:%3%,nEndType:%4%")%__FUNCTION__%__LINE__%playerid%nEndType;
	// 计算玩家输赢
	set_status(eGameState_End);
	if(nEndType==eGameEndType_OnlyOne)
	{
		m_duration = TIME_COMPARE + TIME_LESS;
	}
	else
	{
		m_duration = m_ResultTime + TIME_LESS;
	}
	// check player
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"onGameEnd player err";
		DisbandPlayer();
		return false;
	}
	m_nLastWiner = playerid;
	SLOG_CRITICAL<<boost::format("[GameResult] Pool:%1%")%m_lTotalSliver;
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		if(user.p_active==FALSE) continue;
		int64_t& lUserExpend = user.p_expend;
		uint32_t uid = user.p_id;
		// 消耗
		user.p_result -= lUserExpend;
		// 进账
		if(uid == playerid)
		{
			user.p_result += m_lTotalSliver;

			user.p_asset += m_lTotalSliver;
		}
		lUserExpend = 0;
		SLOG_CRITICAL<<boost::format("[GameResult] %1%:usergold:%2%,result:%3%")%uid%user.p_asset%user.p_result;
	}
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_game_result, zjh_protocols::e_mst_l2c_game_result);
	sendmsg->set_endtype(nEndType);
	sendmsg->set_winid(playerid);
	int nNewSize = m_VecActive.size();
	sendmsg->mutable_resultinfo()->Reserve(nNewSize);
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto user = it->second;
		LPlayerPtr& player = user.p_playerptr;
		if(user.p_active==FALSE || player==nullptr) continue;

		auto resultinfo = sendmsg->add_resultinfo();
		uint32_t uid = user.p_id;
		int64_t resultgold = user.p_result;

		int32_t nTax = 0;
		int32_t nRate = 0;
		auto p_gamehandler = game_engine::instance().get_handler();
		if(p_gamehandler && m_room)
		{
			nRate = p_gamehandler->GetRoomCommissionRate(m_room->get_id());
		}
		if(resultgold > 0 && nRate > 0)
		{
			nTax = std::ceil(resultgold * nRate / 100.0);
		}
		resultgold -= nTax;

#if 0
		int64_t selfgold = user.p_asset;
#else
		int64_t selfgold = player->get_gold() + resultgold;
#endif
		resultinfo->set_playerid(uid);
		resultinfo->set_player_gold(selfgold);
		resultinfo->set_player_result(resultgold);
		// save
		player->write_property(resultgold,nTax,m_cbKillType);
		SLOG_CRITICAL<<boost::format("[Save DB] %1%:usergold:%2%,result:%3%,tax:%4%")%uid%selfgold%resultgold%nTax;
#ifdef UsingLog
		auto count = mGameLog.pinfo_size();
		for (int i = 0; i < count; i++)
		{
			auto pinfo = mGameLog.mutable_pinfo(i);
			auto pid = pinfo->pid();
			if (pid == uid)
			{
				pinfo->set_goldend(selfgold);
				pinfo->set_commission(nTax);
				pinfo->set_vargold(resultgold);
			}
		}
#endif // UsingLog
	}
	broadcast_msg_to_client2(sendmsg);

#ifdef UsingCling
	// @附加队列用户数据
	for (auto it = m_MapExtraPlayer.begin(); it != m_MapExtraPlayer.end(); it++)
	{
		auto extrainfo = it->second;
		auto extraplayer = extrainfo.p_playerptr;
		if (extraplayer != nullptr)
		{
			extraplayer->send_msg_to_client(sendmsg);
		}
	}
#endif // UsingCling
	// card info
	onGameEndCardInfo();

	// 桌子复位---等待客户端动画结算
//	repositGame();

#ifdef UsingLog
	// check is all robot
	bool bAllRobot = true;
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto user = it->second;
		LPlayerPtr& player = user.p_playerptr;
		if (player == nullptr) continue;
		if (user.p_active == TRUE && player->is_robot() == false)
		{
			bAllRobot = false;
			break;
		}
	}
	if (bAllRobot) return true;
	auto count = mGameLog.pinfo_size();
	for (int i = 0; i < count; i++)
	{
		auto pinfo = mGameLog.mutable_pinfo(i);
		auto pid = pinfo->pid();
		// card 
		char szCardinfo[3] = { 0 };
		char cbCheck = 0;
		auto cardresult = m_MapPlayerCard.find(pid);
		if (cardresult != m_MapPlayerCard.end())
		{
			GameCard card;
			card = cardresult->second;
			for (int i=0;i<MAX_CARDS_HAND;i++)
			{
				szCardinfo[i] = card.nCard[i];
			}
			cbCheck = card.bCheck;
		}
		pinfo->set_cardhand(szCardinfo,MAX_CARDS_HAND);
		// opt
		std::vector<tagOptInfo> vecopt;
		auto optresult = mOptLog.find(pid);
		if (optresult != mOptLog.end())
		{
			vecopt = optresult->second;
			for (auto opt : vecopt)
			{
				auto popt = pinfo->add_optarray();
				popt->set_opt(opt.nOpt);
				popt->set_vargold(opt.vargold);
				popt->set_gameround(opt.round);
				popt->set_targetid(opt.targetID);
				popt->clear_targetcard();
				popt->clear_gamestate();
				popt->set_gametime(opt.opttime);
			}
		}
		// vartime
		pinfo->set_vartime(cbCheck);
	}
	mGameLog.set_endtime(time_helper::instance().get_cur_time());
	std::string strLog;
	mGameLog.SerializeToString(&strLog);
	game_engine::instance().get_handler()->sendLogInfo(strLog, GAME_ID);
#endif // UsingLog

	return true;
}

void zjh_space::logic_table::onGameEndCardInfo()
{
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto user = it->second;
		LPlayerPtr& player = user.p_playerptr;
		if(user.p_active==FALSE || player==nullptr) continue;
		uint32_t uid = user.p_id;
		auto cardmsg = PACKET_CREATE(zjh_protocols::packetl2c_open_card, zjh_protocols::e_mst_l2c_open_card);
		Vec_UserID allow;
		allow.clear();
		auto nexus_it = m_MapCompareNexus.find(uid);
		if(nexus_it != m_MapCompareNexus.end())
		{
			Vec_UserID &vecnexus = nexus_it->second;
			// add self
			vecnexus.push_back(uid);
			allow = vecnexus;
		}
		else
		{
			Vec_UserID vecnexus;
			// add self
			vecnexus.push_back(uid);
			allow = vecnexus;
		}
		int nNewSize = allow.size();
		cardmsg->mutable_opencard()->Reserve(nNewSize);
		for(auto vec_it=allow.begin();vec_it!=allow.end();vec_it++)
		{
			auto opencard = cardmsg->add_opencard();
			uint32_t nexusid = (*vec_it);
			opencard->set_playerid(nexusid);
			// find card
			GameCard card;
			auto fresult = m_MapPlayerCard.find(nexusid);
			if (fresult != m_MapPlayerCard.end())
			{
				card = fresult->second;
			}
			// add card
			auto cardinfo = opencard->mutable_cardinfo();
			cardinfo->mutable_pokers()->Reserve(MAX_CARDS_HAND);
			for (int j=0;j<MAX_CARDS_HAND;j++)
			{
				cardinfo->add_pokers(card.nCard[j]);
			}
			cardinfo->set_pokertype(card.nModel);
			cardinfo->clear_pokerstatus();
		}
		player->send_msg_to_client(cardmsg);
	}
}



bool logic_table::onGameAllowToSee(uint32_t playerid)
{
	SLOG_CRITICAL<<boost::format("%1%:%2% playerid:%3%")%__FUNCTION__%__LINE__%playerid;
	// check player
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"onGameAllowToSee player err";
		return false;
	}
	auto &user =  player_it->second;
	auto player = user.p_playerptr;
	// check game state
	int32_t nGameState= get_status();
	if(nGameState!=eGameState_End)
	{
		if(player && player->is_robot())
		{
			SLOG_CRITICAL<<boost::format("%1%:%2% Robot-id:%3%,game state err:%4%")%__FUNCTION__%__LINE__%playerid%nGameState;
			return true;
		}
		else
		{
			SLOG_CRITICAL<<boost::format("%1%:%2% game state err:%3%")%__FUNCTION__%__LINE__%nGameState;
			return false;
		}
	}
	// check user state
	auto active_it = std::find(m_VecActive.begin(),m_VecActive.end(),playerid);
	if(active_it==m_VecActive.end()) 
	{
		SLOG_ERROR<<"onGameAllowToSee user state err:"<<m_VecActive.size();
		return false;
	}
	GameCard card;
	auto fresult = m_MapPlayerCard.find(playerid);
	if (fresult != m_MapPlayerCard.end())
	{
		card = fresult->second;
	}
	else
	{
		return false;
	}
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_let_see_result, zjh_protocols::e_mst_l2c_letsee_result);
	sendmsg->set_playerid(playerid);
	auto cardinfo = sendmsg->mutable_cardinfo();
	cardinfo->mutable_pokers()->Reserve(MAX_CARDS_HAND);
	for (int j=0;j<MAX_CARDS_HAND;j++)
	{
		cardinfo->add_pokers(card.nCard[j]);
	}
	int nCardModel = card.nModel;
	cardinfo->set_pokertype(nCardModel);
	cardinfo->clear_pokerstatus();

	broadcast_msg_to_client2(sendmsg);

	return true;
}


void zjh_space::logic_table::CleanOutPlayer()
{
	for(auto user : m_MapTablePlayer)
	{
		auto player = user.second.p_playerptr;
		uint32_t playerid = user.second.p_id;
		int32_t noaction = user.second.p_noaction;
		int64_t lGold = user.second.p_asset;
		if(player && !player->is_robot())
		{// e_msg_cleanout_def
			// wait
			int32_t nReason = 0;
			if(player->get_wait()>=3)
			{
				nReason = 3;
			}
			else if(noaction >= 2)
			{
				nReason = 4;
			}
			else if(lGold<m_room->get_data()->mGoldMinCondition)
			{
				nReason = 5;
			}

			if(nReason > 0)
			{
				auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_clean_out, zjh_protocols::e_mst_l2c_clean_out);
				sendmsg->set_reason((zjh_protocols::e_msg_cleanout_def)nReason);
				sendmsg->set_sync_gold(lGold);
				player->send_msg_to_client(sendmsg);

				player->reqPlayer_leaveGame();
			}
		}
	}
}

void zjh_space::logic_table::CleanDisconnect()
{
	// 清理断线
	for (auto user : m_MapTablePlayer)
	{
		auto player = user.second.p_playerptr;
		if (player->get_state() == e_player_state::e_ps_disconnect)
		{
			SLOG_WARNING << boost::format("CleanDisconnect");
			// clear table
			player->reqPlayer_leaveGame();
		}
	}
}
void zjh_space::logic_table::CleanNoAction(uint32_t playerid)
{
	for(auto& user : m_MapTablePlayer)
	{
		uint32_t uid = user.second.p_id;
		if(uid == playerid)
		{
			user.second.p_noaction = 0;
		}
	}
}
void zjh_space::logic_table::onGameCleanOut(uint32_t playerid,int32_t nReason)
{
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_clean_out, zjh_protocols::e_mst_l2c_clean_out);
	sendmsg->set_reason((zjh_protocols::e_msg_cleanout_def)nReason);
	broadcast_msg_to_client2(sendmsg);
}

bool zjh_space::logic_table::checkGameStart()
{
	int nReadyCount = GetActiveUserCount();
	if(nReadyCount>=PLAY_MIN_COUNT)
	{
		return true;
	}
	return false;
}

bool zjh_space::logic_table::checkStartSpare()
{
	int nPlayerCount = GetAllUserCount();
	int nReadyCount = GetActiveUserCount();

	if(nReadyCount>=PLAY_MIN_COUNT && nPlayerCount>nReadyCount)
	{
		return true;
	}
	return false;
}

bool zjh_space::logic_table::checkOutofRange()
{
	/* 群魔乱舞
	* 1、人数超过2人
	* 2、达到轮数上限
	* 3、人数==2 强制比牌-不消耗用户筹码
	*/
	// 检测达到上限 --- 全比
	SLOG_CRITICAL<<boost::format("%1%:%2% ,Round:%3% ")%__FUNCTION__%__LINE__%m_nGameRound;
	if (m_nGameRound >= m_room->get_data()->mMaxRound)
	{
		// 达到最大轮数 游戏结束
		return onGameAllPK(m_VecPlaying.size());
	}

	return true;
}

bool zjh_space::logic_table::checkOutofBound(uint32_t playerid)
{
	/* 孤注一掷：
	* 1、人数超过2人
	* 2、金币不足以跟注
	* 3、人数==2  为请求比牌
	*/
	SLOG_CRITICAL<<boost::format("%1%:%2% ,Round:%3% , playerid:%4%")%__FUNCTION__%__LINE__%m_nGameRound%playerid;
	// check player
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"checkOutofBound player err";
		return false;
	}
	if(m_nGameRound < m_room->get_data()->mPkCondition) return false;
	
	auto&user = player_it->second;
	int32_t cflag = user.p_cardflag;
	int64_t & selfgold = user.p_asset;
	int64_t & expend = user.p_expend;
	int64_t lNeedGold = m_lCurJetton;
	if(cflag==eCardState_Ming)
	{
		lNeedGold *= 2;
	}
	else
	{
		lNeedGold *= 1;
	}
	if (selfgold < lNeedGold /*&& m_bPlayingCnt>2*/)
	{
		// 金币不足 通知孤注一掷
		LPlayerPtr player = player_it->second.p_playerptr;
		if(player)
		{
			auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_notice_allin, zjh_protocols::e_mst_l2c_notice_allin);
			sendmsg->set_playerid(playerid);
			if(!player->is_robot())
			{
				player->send_msg_to_client(sendmsg);
			}
			else
			{
				robot_mgr::instance().recv_packet(player->get_pid(),sendmsg->packet_id(),sendmsg);
			}

			m_VecAllin.push_back(playerid);
		}
	}
// 	else if(m_nUserGold[seat] < lNeedGold && m_bPlayingCnt==2)
// 	{ // 服务器比牌-不消耗金币
// 
// 
// 	}
	return true;
}

uint32_t zjh_space::logic_table::CompareWinner(uint32_t reqid,uint32_t resid)
{
	SLOG_CRITICAL<<boost::format("%1%:%2%")%__FUNCTION__%__LINE__;
#if 0  // @比牌大小管控
	// 比牌之前
	reverse_resultEx(reqid,resid);
#endif
	//
	GameCard reqcard,rescard;
	auto fresult = m_MapPlayerCard.find(reqid);
	if (fresult != m_MapPlayerCard.end())
	{
		reqcard = fresult->second;
	}
	auto fresult2 = m_MapPlayerCard.find(resid);
	if (fresult2 != m_MapPlayerCard.end())
	{
		rescard = fresult2->second;
	}

	if (reqcard < rescard)
	{
		return resid;
	}
	else
	{
		return reqid;
	}
// 	if(reqcard.nIndex > rescard.nIndex)
// 		return reqid;
// 	else
// 		return resid;
}

uint32_t zjh_space::logic_table::CompareWinner()
{
	SLOG_CRITICAL<<boost::format("%1%:%2%")%__FUNCTION__%__LINE__;
	// 比牌之前
	reverse_result();

	int32_t nMaxIndx = 0;
	uint32_t nMaxPlayerid = 0;
	for (uint32_t i = 0; i < m_VecPlaying.size(); i++)
	{
		uint32_t uid = m_VecPlaying[i];
		auto it = m_MapPlayerCard.find(uid);
		if(it==m_MapPlayerCard.end())
		{
			assert(0);
		}
		int32_t nCardIndx = it->second.nIndex;
		if(nCardIndx > nMaxIndx)
		{
			nMaxIndx = nCardIndx;
			nMaxPlayerid = uid;
		}
	}
	return nMaxPlayerid;
}

uint32_t zjh_space::logic_table::CompareWinnerEx()
{
	SLOG_CRITICAL<<boost::format("%1%:%2%")%__FUNCTION__%__LINE__;
	uint32_t nCount = m_VecPlaying.size();
	if(nCount==0)
	{
		assert(0);
	}
	uint32_t nMaxPlayerid = m_VecPlaying[0];
	for (uint32_t i = 1; i < nCount; i++)
	{
		uint32_t uid = m_VecPlaying[i];
		nMaxPlayerid = CompareWinner(nMaxPlayerid,uid);
	}

	return nMaxPlayerid;
}



bool zjh_space::logic_table::isNewRound()
{
	SLOG_CRITICAL<<boost::format("isNewRound cur:%1%,round:%2%")%m_nCurOperatorID%m_nGameRound;
	if(m_nFirstID==m_nCurOperatorID)
	{
		m_nGameRound++;
		return true;
	}
	return false;
}

bool zjh_space::logic_table::isVaildGold(int64_t lGold)
{
	for (std::vector<int>::iterator it=m_VecAddJetton.begin();it!=m_VecAddJetton.end();it++)
	{
		if (lGold==(*it))
		{
			return true;
		}
	}
	return false;
}


int64_t zjh_space::logic_table::GetLessGold(uint32_t& lessID)
{
	lessID = 0;
	int64_t lLessGold = 0;
	Vector_pair vecpair ;
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto user = it->second;
		if(user.p_playing==TRUE)
		{
			PAIR pairs(user.p_id,user.p_asset);
			vecpair.push_back(pairs);
		}
	}
	std::sort(vecpair.begin(),vecpair.end(),[](const PAIR& lhs, const PAIR& rhs) {
		return lhs.second > rhs.second;
	});
	if(vecpair.size()==0)
	{
		assert(0);
	}
	PAIR key_value = vecpair.back();
	lessID = key_value.first;
	lLessGold = key_value.second;
	return lLessGold;
}

void zjh_space::logic_table::SetCompareNexus(uint32_t playerid,uint32_t nexusid)
{
	// except self 
	if(playerid==nexusid) return;
	auto it = m_MapCompareNexus.find(playerid);
	if(it==m_MapCompareNexus.end())
	{
		Vec_UserID vecnexus;
		vecnexus.push_back(nexusid);
		m_MapCompareNexus.insert(std::make_pair(playerid,vecnexus));
	}
	else
	{
		Vec_UserID & vecnexus = it->second;
		vecnexus.push_back(nexusid);
	}

	auto nexusit = m_MapCompareNexus.find(nexusid);
	if(nexusit==m_MapCompareNexus.end())
	{
		Vec_UserID vecnexus;
		vecnexus.push_back(playerid);
		m_MapCompareNexus.insert(std::make_pair(nexusid,vecnexus));
	}
	else
	{
		Vec_UserID & vecnexus = nexusit->second;
		vecnexus.push_back(playerid);
	}
}

void zjh_space::logic_table::SetCompareNexus(Vec_UserID &playerids)
{
	for (auto outring_it=playerids.begin();outring_it != playerids.end();outring_it++)
	{
		uint32_t outring_uid = (*outring_it);
		for (auto inring_it=playerids.begin();inring_it != playerids.end();inring_it++)
		{
			SetCompareNexus(outring_uid,(*inring_it));
		}
	}
}





//////////////////////////////////////////////////////////////////////////
void zjh_space::logic_table::robot_heartbeat(double elapsed)
{
#if 0
	if (m_robotcfg.mIsOpen == 0) return;
#else
	if (m_room == nullptr) return;
	if (m_room->getRobotSwitch() == 2)
	{ // 
	  // @正在对局中等到游戏结束再释放
		if (get_status() == eGameState_Play) return;
		
		for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
		{
			auto &user = it->second;
			if (user.p_leave == true) continue;
			auto player = user.p_playerptr;
			if (player == nullptr) continue;
			auto uid = user.p_id;
			// @排除在比牌动画过程中离开
			if (player->is_robot() && player->can_leave_table() /*&& m_uCompareID != uid*/)
			{
				release_robot(uid);
			}
		}
		return;
	}
#endif
	
	int minCount = m_robotcfg.mRobotMinCount;
	int maxCount = m_robotcfg.mRobotMaxCount;
	/*static*/ int requireCount = global_random::instance().rand_int(minCount, maxCount);
	
	m_robot_elapsed -= elapsed;
	if (m_robot_elapsed <= 0)
	{
		m_robot_elapsed = global_random::instance().rand_double(m_robotcfg.mRobotMinEntry, m_robotcfg.mRobotMaxEntry);
		// @2018-3-12
		int32_t nRobotLimt = GAME_PLAYER;
		int32_t nManLimt = GAME_PLAYER;
		auto pRoom = get_room();
		if (pRoom)
		{
			auto roomcfg = pRoom->get_roomcfg();
			nRobotLimt = roomcfg->mTableRobotCount;
			nManLimt = roomcfg->mTableManCount;
		}
		int player_count = 0;
		int robot_count = 0;
		for (auto user : m_MapTablePlayer)
		{
			LPlayerPtr& player = user.second.p_playerptr;
			if(player!=nullptr)
			{
				if(player->is_robot())
					robot_count++;
				else
					player_count++;
			}
		}
		bool bKeepOne = false;
		int all_count = player_count + robot_count;
		if (all_count == GAME_PLAYER)
		{
			if (player_count >= nManLimt)
			{
				// @真实玩家已达要求
			}
			else if (robot_count > 1)
			{// @释放机器人
				if (!bKeepOne) bKeepOne = true;
			}
			else
			{

			}
		}
		else if( player_count > 0)
		{
			if (player_count >= nManLimt)
			{// @请求跟随机器人
				request_robot();
			}
			else if(all_count+1==GAME_PLAYER)
			{
				// @需要留位置
			}
			else
			{// @请求跟随机器人
				request_robot();
			}
		}
		else if(robot_count < nRobotLimt && robot_count < requireCount)
		{
			// @请求跟随机器人
			request_robot();
		}
		else
		{

		}
		// 清理
		if (bKeepOne)
		{
			for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
			{
				auto &user = it->second;
				if (user.p_leave == true) continue;
				auto player = user.p_playerptr;
				if (player == nullptr) continue;
				auto uid = user.p_id;
				if (player->is_robot() && player->can_leave_table())
				{
					release_robot(uid);
					break;
				}
			}
		}
	}
}

void zjh_space::logic_table::request_robot()
{
	// Robot
	static int minVIP = m_robotcfg.mRobotMinVip;
	static int maxVIP = m_robotcfg.mRobotMaxVip;
	int32_t vip_level = global_random::instance().rand_int(minVIP, maxVIP);	
	GOLD_TYPE gold_min = m_robotcfg.mRobotMinTake;
	GOLD_TYPE gold_max = m_robotcfg.mRobotMaxTake;

	GOLD_TYPE enter_gold = global_random::instance().rand_int(gold_min,gold_max);
	int32_t rid = m_room->get_id();
	int32_t tid = get_id();
//	int tag = (rid<<4)|tid;
	int tag = rid + tid * 100;
	SLOG_CRITICAL<<boost::format("request_robot::rid:%1% tid:%2%,tag:%3%")%rid%tid%tag;
	game_engine::instance().request_robot(tag,enter_gold, vip_level);

}

void zjh_space::logic_table::release_robot(int32_t playerid)
{
	// 
	SLOG_CRITICAL<<boost::format("release_robot:playerid:%1%")%playerid;
	// setting robot leave
	auto it = m_MapTablePlayer.find(playerid);
	if(it == m_MapTablePlayer.end()) return;
	auto & user = it->second;
	if (user.p_leave == true) return;
	user.p_leave = true;
	SLOG_CRITICAL << boost::format("release_robot:line:%1%") % __LINE__;
	bc_leave_seat(playerid);
	m_room->get_lobby()->robot_leave(playerid);
	game_engine::instance().release_robot(playerid);
}

int32_t zjh_space::logic_table::release_robot_seat()
{
	// @只能释放可以离开游戏的机器人 随机一个
	int32_t seat = INVALID_CHAIR;
	SLOG_CRITICAL << boost::format("%1%:%2%") % __FUNCTION__%__LINE__;
	int32_t nRobotCount = 0;
	struct tagRobotSeat
	{
		uint32_t robotid;
		int32_t  robotseat;

		tagRobotSeat() :robotid(0), robotseat(0) {}
	};
	std::vector<tagRobotSeat> ceder;
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto uid = user.p_id;
		if (player && player->is_robot())
		{
			nRobotCount++;
			bool bRet = player->can_leave_table();
			if (bRet)
			{
				tagRobotSeat rseat;
				rseat.robotid = uid;
				rseat.robotseat = player->get_seat();
				ceder.push_back(rseat);
			}
		}
	}
	std::random_device rd;
	std::mt19937_64 g(rd());
	std::shuffle(ceder.begin(), ceder.end(), g);
	if (ceder.size() > 0)
	{
		tagRobotSeat frseat = ceder.back();
		uint32_t ruid = frseat.robotid;
		auto player_it = m_MapTablePlayer.find(ruid);
		if (player_it != m_MapTablePlayer.end())
		{
			auto player = player_it->second.p_playerptr;
			if (player && player->is_robot())
			{
				player->leave_table();
				release_robot(ruid);
				seat = frseat.robotseat;
			}
		}
	}
	return seat;

}

bool zjh_space::logic_table::can_release_robot()
{
	SLOG_CRITICAL << boost::format("%1%:%2%") % __FUNCTION__%__LINE__;
	bool bRet = false;
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto user = it->second;
		auto player = user.p_playerptr;
		auto playerid = user.p_id;
		if (player && player->is_robot() && player->can_leave_table() /*&& m_uCompareID != playerid*/)
		{
			bRet = true;
			break;
		}
	}
	return bRet;
}

void zjh_space::logic_table::reverse_result(uint32_t reqid,uint32_t resid)
{
	SLOG_CRITICAL<<boost::format("%1%:%2%")%__FUNCTION__%__LINE__;
	int32_t nRobotCount = robot_counter();
	if(nRobotCount==0) return;
	uint32_t robot_req = 0,robot_res = 0;
	robot_req = robot_id(reqid);
	robot_res = robot_id(resid);
	if(robot_req==0 && robot_res==0) return;
	SLOG_CRITICAL<<boost::format("reverse_result:RobotCount:%1%,%2%,%3%")%nRobotCount%robot_req%robot_res;
	int32_t rate = robot_rate();
	int nRandom = global_random::instance().rand_int(1,100);
	if(nRandom <= rate)
	{
		if (nRobotCount>1)
		{// 此时机器是可以输的
			if(robot_req!=0)
			{
				robot_switch(robot_req,global_random::instance().rand_int(1,100));
			}
			if(robot_res!=0)
			{
				robot_switch(robot_res,global_random::instance().rand_int(1,100));
			}
		}
		else if(nRobotCount==1)
		{
			// 此时机器人是不能输的
			if(robot_req!=0)
			{
				robot_switch(robot_req);
			}
			if(robot_res!=0)
			{
				robot_switch(robot_res);
			}
		}
		else
		{
			// 此时无意义
		}
	}
	else
	{
		// 此时机器是可输可赢
// 		if(robot_req!=0)
// 		{
// 			robot_switch(robot_res,global_random::instance().rand_int(1,100));
// 		}
// 		if(robot_res!=0)
// 		{
// 			robot_switch(robot_res,global_random::instance().rand_int(1,100));
// 		}
	}
}



void zjh_space::logic_table::reverse_resultEx(uint32_t reqid,uint32_t resid)
{
	int32_t nRobotCount = robot_counter(reqid,resid);
	SLOG_CRITICAL<<boost::format("%1%:%2%,RobotCount:%3%")%__FUNCTION__%__LINE__%nRobotCount;
	if(nRobotCount==0)
	{
		// 玩家比牌
	}
	if(nRobotCount==1)
	{
		uint32_t playerid = 0,robotid = 0;
		uint32_t robot_req = 0,robot_res = 0;
		robot_req = robot_id(reqid);
		robot_res = robot_id(resid);
		if(robot_req==0 && robot_res==0) return;
		if(robot_req==0)
		{
			playerid = reqid;
			robotid = resid;
		}
		else
		{
			playerid = resid;
			robotid = reqid;
		}
#if 0
		SLOG_CRITICAL<<boost::format("[Switch]Robot:%d,Player:%d")%robotid%playerid;
		auto it = m_MapTablePlayer.find(playerid);
		if(it == m_MapTablePlayer.end()) return ;
		auto user = it->second;
		auto player = user.p_playerptr;
		tagPlayerCtrlAttr attr;
		player->get_player_ctrl(attr,m_lTotalSliver,user.p_expend);
		double fRandom = global_random::instance().rand_01();
		double fWinPercent = attr.fwinPercent;
		SLOG_CRITICAL<<boost::format("[Switch]Robot:%d,Player:%d,PlayerWin:%f")%robotid%playerid%fWinPercent;
		SLOG_CRITICAL<<boost::format("Random:%f,playerid:%d,Tag:%d,Win:%f")%fRandom%playerid%attr.nTag%fWinPercent;
		if(fRandom <= fWinPercent)
		{ // 玩家赢

		}
		else
		{// 机器人赢
			robot_switch(robotid);
		}
#else
		SysRegulatoryCompare(playerid,robotid);
#endif
	}
	else
	{
		// 机器人比牌
	}
}

void zjh_space::logic_table::reverse_result()
{
	SLOG_CRITICAL<<boost::format("%1%:%2%")%__FUNCTION__%__LINE__;
	int32_t nRobotCount = robot_counter();
	if(nRobotCount==0) return;
	SLOG_CRITICAL<<boost::format("reverse_result:RobotCount:%1%")%nRobotCount;
	int32_t rate = robot_rate();
	int nRandom = global_random::instance().rand_int(1,100);
	if (nRandom <= rate)
	{
		for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
		{ // 活着的机器人
			auto user = it->second;
			uint32_t uid = user.p_id;
			auto player = user.p_playerptr;
			if(user.p_playing==FALSE) continue;
			if( player && player->is_robot())
			{
				if(nRobotCount==1)
					robot_switch(uid);
				else
					robot_switch(uid,global_random::instance().rand_int(1,100));
			}	
		}
	}
}


void zjh_space::logic_table::robot_switch(uint32_t uid)
{
	auto fresult = m_MapPlayerCard.find(uid);
	if (fresult != m_MapPlayerCard.end())
	{
		auto& card = fresult->second;
		if (card.bCheck == 0)
		{
			if (m_GameCardMgr.GetUserCard(card, eCardLvL_Large))
			{
				card.bCheck = 1;
			}
		}
	}
}

void zjh_space::logic_table::robot_switch(uint32_t robotid, uint32_t playerid)
{
	GameCard card;
	auto fresult = m_MapPlayerCard.find(playerid);
	if (fresult != m_MapPlayerCard.end())
	{
		card = fresult->second;
	}
	auto robotresult = m_MapPlayerCard.find(robotid);
	if (robotresult == m_MapPlayerCard.end()) return;
	auto& robotcard = robotresult->second;
	if (robotcard < card) return;
	if (robotcard.bCheck != 0) return;
	if (m_GameCardMgr.GetUserCard(card, eCardlvl::eCardLvL_Small))
	{ // 设置小牌给机器人
		robotcard = card;
		robotcard.bCheck = 2;
	}
}

int32_t zjh_space::logic_table::robot_counter()
{
	int32_t nCount = 0;
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{ // 活着的机器人
		auto user = it->second;
		auto player = user.p_playerptr;
		if(user.p_playing==FALSE || player==nullptr) continue;
		if(player->is_robot())
		{
			nCount++;
		}
	}
	return nCount;
}

int32_t zjh_space::logic_table::robot_counter(uint32_t reqid,uint32_t resid)
{
	int32_t nCount = 0;
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{ // 活着的机器人
		auto user = it->second;
		auto player = user.p_playerptr;
		if(user.p_playing==FALSE || player==nullptr) continue;
		auto uid = user.p_id;
		if(player->is_robot())
		{
			if(uid == reqid || uid==resid) nCount++;
		}
	}
	return nCount;
}

uint32_t zjh_space::logic_table::robot_id(uint32_t uid)
{
	uint32_t robotid = 0;
	auto it = m_MapTablePlayer.find(uid);
	if(it == m_MapTablePlayer.end()) return robotid;
	auto player = it->second.p_playerptr;
	if(player && player->is_robot())
	{
		robotid = uid;
	}

	return robotid;
}

int32_t zjh_space::logic_table::robot_rate()
{
	int32_t nRate = 0;
	int32_t nLabel = GetMaxLabel();
	int32_t nRound = m_nGameRound;
	if(nLabel==Label_UserC)
	{
		nRate = 100;
	}
	else if(nLabel==Label_UserB)
	{
		nRate = (80-nRound*4);
	}
	else if(nLabel==Label_UserA)
	{
		nRate = (44-nRound*4);
	}
	else
	{
		nRate = 50;
	}

	return nRate;
}

int32_t zjh_space::logic_table::GetLabelByValue(int64_t nValue)
{
	int64_t base = m_room->get_data()->mBaseCondition;
// 	if(nValue<=200*base)
// 	{
// 		return Label_UserA;
// 	}
// 	else if(nValue<=300*base)
// 	{
// 		return Label_UserB;
// 	}
// 	else
	{
		return Label_UserC;
	}
}

void zjh_space::logic_table::CalcUserLabel()
{
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto & user = it->second;
		if(user.p_playing==FALSE) continue;
		uint32_t uid = user.p_id;
		auto& player = user.p_playerptr;
		if(player)
		{
			int32_t nTag= player->get_player_tag();
			if(nTag==Tag_Supper)
			{
				user.p_label = Label_Supper;
			}
			else if(nTag==Tag_Robot)
			{
				user.p_label = Label_Robot;
			}
			else
			{
				int64_t nLabelValue = nTag + m_lTotalSliver - user.p_expend;
				user.p_label = GetLabelByValue(nLabelValue);
			}
		}
	}
}

int32_t zjh_space::logic_table::GetMaxLabel()
{
	CalcUserLabel();
	for(auto user: m_MapTablePlayer)
	{
		if(user.second.p_label == Label_UserC)
			return Label_UserC;
	}
	for(auto user: m_MapTablePlayer)
	{
		if(user.second.p_label == Label_UserB)
			return Label_UserB;
	}
	for(auto user: m_MapTablePlayer)
	{
		if(user.second.p_label == Label_UserA)
			return Label_UserA;
	}

	return Label_Robot;
}


void zjh_space::logic_table::ReadCardRate()
{
	m_VecCardRate.clear();
	GoldFlower_CardCFG::GetSingleton()->Reload();
	const boost::unordered_map<int, GoldFlower_CardCFGData>& list = GoldFlower_CardCFG::GetSingleton()->GetMapData();
	for (auto it = list.begin(); it != list.end(); ++it)
	{
		if(it->second.mRoomID == m_room->get_id())
		{
			m_VecCardRate = it->second.mCardModeRate;
		}
	}
}

void zjh_space::logic_table::DisbandPlayer()
{
	SLOG_CRITICAL<<"------------DissBand All------------";
	m_duration = 0.0;
	set_status(eGameState_Free);
	// 游戏数据初始化
	m_uBankerID = 0;

	m_nCurOperatorID = 0;
	m_nLastWiner = 0;
	m_lCurJetton = 0;

	m_lTotalSliver = 0;
	m_lInventPool = 0;

	m_nFirstID = 0;
	m_MapCompareNexus.clear();
	m_VecActive.clear();
	m_VecPlaying.clear();

	m_nGameRound = 0;

	m_nEndType = eGameEndType_null;

	m_nShowHandCnt = 0;
	m_VecAllin.clear();

	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		if (user.p_leave == true) continue;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		auto uid = user.p_id;

		if(player->is_robot())
		{
			SLOG_CRITICAL << boost::format("release_robot:line:%1%") % __LINE__;
			release_robot(uid);
		}
		else
		{
			auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_clean_out, zjh_protocols::e_mst_l2c_clean_out);
			sendmsg->set_reason((zjh_protocols::e_msg_cleanout_def)1);
			sendmsg->set_sync_gold(player->get_gold());
			player->send_msg_to_client(sendmsg);
		}

	}

}

bool zjh_space::logic_table::onPlayerRunaway(uint32_t playerid)
{
	SLOG_CRITICAL<<boost::format("%1%:%2%,playerid:%3%")%__FUNCTION__%__LINE__%playerid;
	
	// playing
	auto it = std::find(m_VecPlaying.begin(),m_VecPlaying.end(),playerid);
	if(it != m_VecPlaying.end())
	{
		m_VecPlaying.erase(it);
	}
	// check game over
// 	int32_t nCount = 0;
// 	uint32_t winer = 0;
// 	for(auto user : m_MapTablePlayer)
// 	{
// 		auto tableusr = user.second;
// 		if(tableusr.p_playing == TRUE)
// 		{
// 			winer = tableusr.p_id;
// 			nCount++;
// 		}
// 	}
	int32_t nCount = m_VecPlaying.size();
	if(nCount>=PLAY_MIN_COUNT)
	{
		// 游戏继续
	}
	else if(nCount==1)
	{
		// 游戏结束
		uint32_t winer = m_VecPlaying.back();
		if(winer!=0)
		{
			onGameEnd(winer,eGameEndType_OnlyOne);
		}
		else
		{
			repositGame();
		}
	}
	else
	{
		// 桌子没人
		repositGame();
	}
	return true;
}

void zjh_space::logic_table::broadcast_msglist_to_client()
{
	if (!m_msglist.empty())
	{
		Vec_UserID userpids;
		Vec_UserID robotpids;
		for (auto user : m_MapTablePlayer)
		{
			LPlayerPtr& player = user.second.p_playerptr;
			if (player != nullptr )
			{
				if(!player->is_robot())
				{
					userpids.push_back(player->get_pid());
				}
				else
				{
					robotpids.push_back(player->get_pid());
				}
			}
		}
		if(userpids.size() > 0)
		{
			game_engine::instance().get_handler()->broadcast_msglist_to_client(userpids, m_msglist);
		}

		if(robotpids.size() > 0)
		{
			robot_mgr::instance().recv_bc_packet(robotpids, m_msglist);
		}

		m_msglist.clear();
	}
}

void zjh_space::logic_table::OnEnterBackground(uint32_t playerid)
{
	SLOG_CRITICAL << boost::format("%1% :%2%") % __FUNCTION__%playerid;
#if 0
	auto player_it = m_MapTablePlayer.find(playerid);
	if (player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR << boost::format("%1%: Can't find this user:%2%") % __FUNCTION__%playerid;
	}
	else
	{
		auto &user = player_it->second;
		user.p_background = true;
		auto player = user.p_playerptr;
		// 空闲状态立即清除
		if (player && player->can_leave_table())
		{ 
			auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_clean_out, zjh_protocols::e_mst_l2c_clean_out);
			sendmsg->set_reason(zjh_protocols::e_msg_cleanout_def::e_cleanout_enter_background);
			sendmsg->set_sync_gold(player->get_gold());
			player->send_msg_to_client(sendmsg);

			//  
			player->reqPlayer_leaveGame();	
		}

	}
#endif

}

void zjh_space::logic_table::OnDealEnterBackground()
{
// 	std::map<uint32_t, TablePlayer>  tMapTablePlayer = m_MapTablePlayer;
// 	for (auto user : tMapTablePlayer)
// 	{
// 		auto player = user.second.p_playerptr;
// 		uint32_t playerid = user.second.p_id;
// 		bool bbackground = user.second.p_background;
// 		if (player && bbackground)
// 		{
// 			SLOG_CRITICAL << boost::format("logic_table::clean enter background >>>> :%1%") % playerid;
// 			// clear table
// 			player->leave_table();
// 		}
// 	}
}

void zjh_space::logic_table::CleanKickedPlayer()
{
	// 游戏结束踢出玩家
	for (auto &user : m_MapTablePlayer)
	{
		auto& tuser = user.second;
		auto player = tuser.p_playerptr;
		if(player==nullptr) continue;
		if(player->is_robot()) continue;
		auto &state = tuser.p_leave;
		if(state==true) continue;
		uint32_t playerid = user.second.p_id;
		int reason = 0;
		if (get_room()->getservice() == 0)
		{
			// @ 踢出用户：系统维护
			reason = 8;
			state = true;
		}
		else if (tuser.p_kick == true)
		{
			/*
			* @ 踢出用户 离开World服务
			*/
			reason = 7;
			tuser.p_kick = false;
			state = true;
		}
		else if (player->get_serverflag() == 1)
		{
			state = true;
			player->reqPlayer_leaveGame(3);
		}
		if (reason==7 || reason==8)
		{
			auto sendmsg = PACKET_CREATE(zjh_protocols::packetl2c_clean_out, zjh_protocols::e_mst_l2c_clean_out);
			sendmsg->set_reason((zjh_protocols::e_msg_cleanout_def)reason);
			sendmsg->set_sync_gold(player->get_gold());
			player->send_msg_to_client(sendmsg);

			player->reqPlayer_leaveGame();
		}
	}
}

void logic_table::save_gamelog(uint32_t playerid, tagOptInfo  OptInfo)
{
	auto findopt = mOptLog.find(playerid);
	if (findopt != mOptLog.end())
	{
		std::vector<tagOptInfo> &vecoptinfo = findopt->second;
		vecoptinfo.push_back(OptInfo);
	}
	else
	{
		std::vector<tagOptInfo> vecoptinfo;
		vecoptinfo.push_back(OptInfo);
		mOptLog.insert(std::make_pair(playerid, vecoptinfo));
	}
}

uint32_t zjh_space::logic_table::GetCurLargestPlayer(bool bAll /*= true*/)
{
	typedef std::pair<uint32_t, GameCard> CardPair;
	std::vector<CardPair> vec_id_card;
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if (player == nullptr) continue;
		if (user.p_leave == true) continue;
		if (user.p_active == FALSE) continue;
		if (user.p_playing == FALSE) continue;
		// kick robot
		if(bAll==false && player->is_robot()) continue;
		auto playerid = user.p_id;
		// card
		GameCard card;
		auto fresult = m_MapPlayerCard.find(playerid);
		if (fresult != m_MapPlayerCard.end())
		{
			card = fresult->second;
			vec_id_card.push_back(std::make_pair(playerid, card));
		}
	}
	// 判断最大的玩家
	std::sort(vec_id_card.begin(), vec_id_card.end(), [](const CardPair& lhs, const CardPair& rhs) {
		return lhs.second.nIndex < rhs.second.nIndex; });
	CardPair max_cardpair = vec_id_card.back();
	SLOG_CRITICAL << boost::format("%1%:%2%, max_cardpair:%3%") % __FUNCTION__%__LINE__%vec_id_card.size();

	return max_cardpair.first;
}

int64_t zjh_space::logic_table::GetAllRobotExpend()
{
	int64_t nRExpend = 0;
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if (player == nullptr) continue;
		if (user.p_leave == true) continue;
		if (user.p_active == FALSE) continue;
		if (player->is_robot())
		{
			nRExpend += user.p_expend;
		}
	}
	return nRExpend;
}

int32_t logic_table::cling_robotid()
{
	/* @ 依附机器人
	*  @ 一个位置只能存在一个真实玩家
	*  @ 在本轮游戏结束后才能进入
	*  @ 允许重复附加但只能存在一个
	*/
	int32_t seat = INVALID_CHAIR;
	uint32_t robotid = 0;
	SLOG_CRITICAL << boost::format("%1%:%2%") % __FUNCTION__%__LINE__;
	Vec_UserID ceder;
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto uid = user.p_id;
		auto idx = user.p_idx;
		if (player && player->is_robot())
		{
			auto findseat = m_MapExtraPlayer.find(idx);
			if (findseat == m_MapExtraPlayer.end())
			{
				ceder.push_back(uid);
			}
		}
	}
	std::random_device rd;
	std::mt19937_64 g(rd());
	std::shuffle(ceder.begin(), ceder.end(), g);
	if (ceder.size() > 0)
	{
		uint32_t ruid = ceder.back();
		auto player_it = m_MapTablePlayer.find(ruid);
		if (player_it != m_MapTablePlayer.end())
		{
			auto& user = player_it->second;
			auto player = user.p_playerptr;
			if (player)
			{
				robotid = ruid;
				seat = user.p_idx;
				// @ 需要清理此机器人
				user.p_kick = true;
			}
		}
	}
	return seat;
}

void logic_table::joinnextround()
{
	// @清理机器人
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if (player == nullptr) continue;
		auto &state = user.p_leave;
		if (state == true) continue;
		auto uid = user.p_id;
		auto seat = user.p_idx;
		if (player->is_robot() && user.p_kick)
		{
			SLOG_CRITICAL << boost::format("joinnextround :%1%") % uid;
			release_robot(uid);
			state = true;
			// @椅子归还
			m_VecChairMgr.push_back(seat);
		}
	}
	// @加入对局中
	for (auto it = m_MapExtraPlayer.begin(); it != m_MapExtraPlayer.end(); it++)
	{
		auto table_player = it->second;
		// @检查椅子使用情况
		auto seat = table_player.p_idx;
		auto findseat = std::find(m_VecChairMgr.begin(), m_VecChairMgr.end(), seat);
		if (findseat != m_VecChairMgr.end())
		{
			m_MapTablePlayer.insert(std::make_pair(table_player.p_id, table_player));

			inc_dec_count();
			bc_enter_seat(table_player);
			// @椅子已经被使用
			m_VecChairMgr.erase(findseat);
		}
		else
		{
			assert(0);
		}
	}

	m_MapExtraPlayer.clear();
}

bool logic_table::SysRegulatoryCompare(uint32_t playerid,uint32_t robotid)
{
	SLOG_CRITICAL << boost::format("%1%:%2%") % __FUNCTION__%__LINE__;
	//  ADD : TagC Killer
	if (CounterTagC() > 0)
	{
		// 换大牌
		robot_switch(robotid);
		SLOG_CRITICAL << boost::format(" TagC Killer:line:%1%") % __LINE__;
		return true;
	}
	// ADD : Small Card 
	SysRegulatory4Model(playerid);

	bool bSwitch = false;
	uint32_t playermax = GetCurLargestPlayer();
	auto findmax = m_MapTablePlayer.find(playermax);
	if (findmax == m_MapTablePlayer.end()) return bSwitch;
	SLOG_CRITICAL << boost::format("%1%:%2%") % __FUNCTION__%__LINE__;
	auto usermax = findmax->second;
	auto maxtag = usermax.p_tag;
	auto maxpercent = usermax.p_percent * 10000;
	std::random_device rd;
	std::mt19937_64 g(rd());
	std::uniform_int_distribution<> dis(0, 10000);
	if (maxtag == Tag_UserC)
	{
		if (dis(g) <= maxpercent)
		{
			bSwitch = false;
		}
		else
		{
			bSwitch = true;
			robot_switch(robotid);
		}
		SLOG_CRITICAL << boost::format(" [Assert]:TagC Killer:line:%1%") % __LINE__;
	}
	else if(maxtag == Tag_UserB)
	{
		SLOG_CRITICAL << boost::format("================================");
		// 计算系统支出比例
		int64_t nRobotExpend = GetAllRobotExpend();
		float coeffi = 0;
		if (get_room()->getkiller() == 10)
		{
			coeffi = get_room()->getCutRound()*1.0/10000;
			SLOG_CRITICAL << boost::format("%1%:%2% Kill:%3%") % __FUNCTION__%__LINE__%coeffi;
		}
		else
		{
			coeffi = m_robotcfg.mRobotCoeffi*1.0;
			SLOG_CRITICAL << boost::format("%1%:%2% UnKill:%3%") % __FUNCTION__%__LINE__%coeffi;
		}
		int32_t percent = nRobotExpend*coeffi / m_lTotalSliver * 10000;
		SLOG_CRITICAL << boost::format("%1%:%2% nRobotExpend:%3%,TotalSliver:%4%") % __FUNCTION__%__LINE__%nRobotExpend%m_lTotalSliver;
		auto disRet = dis(g);
		if( disRet <= percent)
		{ // 换大牌
			bSwitch = true;
			robot_switch(robotid);
		}
		else
		{
			bSwitch = false;
		}
		SLOG_CRITICAL << boost::format("%1%:%2%[Largest]disRet:%3% ,percent:%4% ,Switch:%5%") % __FUNCTION__%__LINE__%disRet%percent%bSwitch;
		SLOG_CRITICAL << boost::format("================================");
	}
	else if(maxtag == Tag_Robot)
	{
		return SysRegulatoryCompareEx(playerid, robotid);
	}
	
	return bSwitch;
}

bool zjh_space::logic_table::SysRegulatoryCompareEx(uint32_t playerid, uint32_t robotid)
{
	SLOG_CRITICAL << boost::format("%1%:%2%") % __FUNCTION__%__LINE__;
	bool bSwitch = false;
	std::random_device rd;
	std::mt19937_64 g(rd());
	std::uniform_int_distribution<> dis(0, 10000);
	// 
	uint32_t playermax = GetCurLargestPlayer(false);
	auto findmax = m_MapTablePlayer.find(playermax);
	if (findmax == m_MapTablePlayer.end()) return bSwitch;
	SLOG_CRITICAL << boost::format("%1%:%2%") % __FUNCTION__%__LINE__;
	auto usermax = findmax->second;
	auto maxtag = usermax.p_tag;
	auto maxpercent = usermax.p_percent * 10000;
	if (maxtag == Tag_UserC)
	{
		if (dis(g) <= maxpercent)
		{
			bSwitch = true;
			robot_switch(robotid, playerid);
		}
		else
		{
			bSwitch = false;
		}
		SLOG_CRITICAL << boost::format(" [Assert]:TagC Killer:line:%1%") % __LINE__;
	}
	else
	{
		SLOG_CRITICAL << boost::format("================================");
		int64_t nRobotExpend = GetAllRobotExpend();
		float coeffi = 0;
		if (get_room()->getkiller() == 10)
		{
			coeffi = get_room()->getCutRound()*1.0 / 10000;
			SLOG_CRITICAL << boost::format("%1%:%2% Kill:%3%") % __FUNCTION__%__LINE__%coeffi;
		}
		else
		{
			coeffi = m_robotcfg.mRobotCoeffi*1.0;
			SLOG_CRITICAL << boost::format("%1%:%2% UnKill:%3%") % __FUNCTION__%__LINE__%coeffi;
		}
		int32_t percent = nRobotExpend*coeffi / m_lTotalSliver * 10000;
		SLOG_CRITICAL << boost::format("%1%:%2% nRobotExpend:%3%,TotalSliver:%4%") % __FUNCTION__%__LINE__%nRobotExpend%m_lTotalSliver;
		auto disRet = dis(g);
		if ( disRet <= percent)
		{
			bSwitch = false;
		}
		else
		{ // 换小牌
			bSwitch = true;
			robot_switch(robotid, playerid);
		}
		SLOG_CRITICAL << boost::format("%1%:%2%[Small]disRet:%3% ,percent:%4% ,Switch:%5%") % __FUNCTION__%__LINE__%disRet%percent%bSwitch;
		SLOG_CRITICAL << boost::format("================================");
	}

// 	auto findplayer = m_MapTablePlayer.find(playerid);
// 	if (findplayer == m_MapTablePlayer.end()) return bSwitch;
// 	auto cmpuser = findplayer->second;
// 	auto playertag = cmpuser.p_tag;
// 	auto playerpercent = cmpuser.p_percent * 10000;
// 	if (playertag == Tag_UserC)
// 	{
// 		if (dis(g) <= playerpercent)
// 		{
// 			bSwitch = true;
// 			robot_switch(robotid, playerid);
// 		}
// 		else
// 		{
// 			bSwitch = false;
// 		}
// 	}
// 	else
// 	{
// 		int64_t nRobotExpend = GetAllRobotExpend();
// 		float coeffi = 0;
// 		if (get_room()->getkiller() == 10)
// 		{
// 			coeffi = get_room()->getCutRound()*1.0 / 10000;
// 		}
// 		else
// 		{
// 			coeffi = m_robotcfg.mRobotCoeffi*1.0;
// 		}
// 		int32_t percent = nRobotExpend*coeffi / m_lTotalSliver * 10000;
// 		if (dis(g) <= percent)
// 		{
// 			bSwitch = false;
// 		}
// 		else
// 		{ // 换小牌
// 			bSwitch = true;
// 			robot_switch(robotid, playerid);
// 		}
// 	}

	return bSwitch;
}

bool logic_table::SysRegulatoryCheckCard(uint32_t robotid)
{
	bool bGiveUp = false;
	int32_t nCountA = 0, nCountB = 0, nCountC = 0;
	int32_t nCountR = 0, nCountOther = 0;
	// player info
	auto player_it = m_MapTablePlayer.find(robotid);
	if (player_it == m_MapTablePlayer.end()) return bGiveUp;
	auto player = player_it->second.p_playerptr;
	if (player && player->is_robot() == false) return bGiveUp;
	// 
	uint32_t playermax = GetCurLargestPlayer();
	auto findmax = m_MapTablePlayer.find(playermax);
	if (findmax == m_MapTablePlayer.end()) return bGiveUp;
	// card info
	GameCard card;
	auto fresult = m_MapPlayerCard.find(robotid);
	if (fresult != m_MapPlayerCard.end())
	{
		card = fresult->second;
	}
	//
	bool bSmall = m_GameCardMgr.SmallofBaseCard(card);
	if (bSmall == true)
	{
		if (playermax == robotid)
		{// 当前最大的牌
			bGiveUp = false;
		}
		else
		{// 不是当前最大的牌
			if (robot_counter() > 1)
			{
				// 弃牌
				bGiveUp = true;
			}
			else
			{
				auto usermax = findmax->second;
				auto maxtag = usermax.p_tag;
				auto maxpercent = usermax.p_percent * 10000;
				std::random_device rd;
				std::mt19937_64 g(rd());
				std::uniform_int_distribution<> dis(0, 10000);
				if (maxtag == Tag_UserC)
				{// 此时如果有C玩家 如果C是最大的牌则根据概率换牌或者弃牌
					if (dis(g) <= maxpercent)
					{// 弃牌
						bGiveUp = true;
					}
					else
					{// 换牌
						robot_switch(robotid);
						bGiveUp = false;
					}
				}
				else if (maxtag == Tag_UserB)
				{
					bGiveUp = false;
				}
				else
				{
					// 弃牌
					bGiveUp = true;
				}
			}
		}
	}
	else
	{
		bGiveUp = false;
	}
	return bGiveUp;
}

int32_t zjh_space::logic_table::CounterTagC()
{
	SLOG_CRITICAL << boost::format("%1%:%2%") % __FUNCTION__%__LINE__;
	int32_t nCounter = 0;
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if (player == nullptr) continue;
		if (user.p_leave == true) continue;
		if (user.p_active == FALSE) continue;
		if (user.p_playing == FALSE) continue;
		if (player->is_robot()) continue;
		if (user.p_tag == Tag_UserC)
		{
			nCounter++;
		}
	}
	SLOG_CRITICAL << boost::format("%1%:%2% ,nCounter:%3% ") % __FUNCTION__%__LINE__%nCounter;
	return nCounter;

}

void zjh_space::logic_table::SysKillerPolicy()
{
	std::random_device rd;
	std::mt19937_64 g(rd());
	// @ 0：default 1：kill 2: donate
	int killopt = get_room()->getkiller();
	int cutround = get_room()->getCutRound();
	cutround = std::abs(cutround);
	if (cutround == 0)
	{
		killopt = 11;
	}
	// 
	if (killopt == 10)
	{ // start
		if (m_cbGameRound == cutround)
		{
			m_cbKillRound = 0;
			m_cbGameRound = 0;
		}
		// kill point
		if (m_cbKillRound == 0)
		{
			std::uniform_int_distribution<> dis(1, cutround);
			m_cbKillRound = dis(g);
		}
		m_cbGameRound++;
	}
	else if (killopt == 11)
	{ // stop
		m_cbKillRound = 0;
	}
}

uint32_t zjh_space::logic_table::PreSysRegulatoryPolicy()
{
	uint32_t killerid = 0;
	std::random_device rd;
	std::mt19937_64 g(rd());

	SysKillerPolicy();

	// @ 0：default 1：kill 2: donate
	int32_t cbKiller = 0;
	int killopt = get_room()->getkiller();
	int cutround = get_room()->getCutRound();
	// @ check kill
	if (killopt == 10)
	{
		if (m_cbKillRound != 0 && m_cbGameRound == m_cbKillRound)
		{
			if (cutround > 0)
			{
				cbKiller = 1;
			}
			else if (cutround < 0)
			{
				cbKiller = 2;
			}
		}
	}
	else if (killopt == 11)
	{
		cbKiller = 0;
	}
	SLOG_CRITICAL << boost::format("%1%:%2%:[Kill-State]:%3%") % __FUNCTION__%__LINE__%cbKiller;
	m_cbKillType = cbKiller;

	Vec_UserID vecrobot;
	Vec_UserID vecother;
	Vec_UserID vecfind;
	for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto playerid = user.p_id;
		if (player == nullptr) continue;
		if (user.p_active == FALSE) continue;
		if (player->is_robot())
		{
			vecrobot.push_back(playerid);
		}
		else
		{
			vecother.push_back(playerid);
		}
	}
	if (cbKiller == 1)
	{
		// @系统赢
		m_MapPlayerCard.clear();
		m_GameCardMgr.Wash_CardControl();
		std::shuffle(vecrobot.begin(), vecrobot.end(), g);
		std::shuffle(vecother.begin(), vecother.end(), g);
		// @随机1个机器人赢
		for (auto id : vecrobot)
		{
			if (vecfind.size() >= 1)
			{
				vecother.push_back(id);
			}
			else
			{
				vecfind.push_back(id);
				killerid = id;
			}
		}
		std::copy(vecother.begin(), vecother.end(), std::back_inserter(vecfind));

	}
// 	else if (cbKiller == 2)
// 	{
// 		// @玩家赢
// 	}
	else
	{
		m_MapPlayerCard.clear();
		m_GameCardMgr.Wash_CardNomal();
		for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
		{
			auto &user = it->second;
			auto player = user.p_playerptr;
			if (player == nullptr) continue;
			if (user.p_active == FALSE) continue;
			uint32_t uid = user.p_id;
			vecfind.push_back(uid);
		}
	}
	// @ 查找当前最大的机器人
	killerid = 0;
	GameCard kCard;
	for (auto f_id : vecfind)
	{
		GameCard card;
		if (!m_GameCardMgr.GetUserCard(card))
		{
			m_GameCardMgr.GetUserCard(card, eCardLvL_Random);
			SLOG_WARNING << "[Warning]：Can't find card to : "<<f_id;
		}
		int32_t nModel = card.nModel;
		SLOG_CRITICAL << boost::format("[CHK] playerid:%d Model:%d Card:[%02x,%02x,%02x]") % f_id%nModel
			%card.nCard[0] % card.nCard[1] % card.nCard[2] ;
		m_MapPlayerCard.insert(std::make_pair(f_id, card));

		// @ 查找当前最大的机器人
		auto itfind = std::find(vecrobot.begin(), vecrobot.end(), f_id);
		if (itfind != vecrobot.end())
		{
			if (killerid == 0)
			{
				kCard = card;
				killerid = f_id;
			}
			else
			{
				if (kCard < card || kCard == card)
				{
					kCard = card;
					killerid = f_id;
				}
			}
		}
	}

	return killerid;
}

void zjh_space::logic_table::SysRegulatory4Model(uint32_t playerid)
{
	bool bSysReg = false;
	GameCard card;
	auto fresult = m_MapPlayerCard.find(playerid);
	if (fresult != m_MapPlayerCard.end())
	{
		card = fresult->second;
		if (m_GameCardMgr.SmallofSingleCard(card))
		{
			bSysReg = true;
		}
	}
	if (bSysReg)
	{
		auto player_it = m_MapTablePlayer.find(playerid);
		if (player_it != m_MapTablePlayer.end())
		{
			auto &user = player_it->second;
			user.p_percent = 0.2;
		}
	}
}