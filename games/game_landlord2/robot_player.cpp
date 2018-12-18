#include "stdafx.h"
#include "robot_player.h"
#include "logic_player.h"
#include "logic_table.h"
#include "logic_room.h"

#include "landlord3_logic.pb.h"
#include <net/packet_manager.h>

#include "Landlord3_BaseInfo.h"
#include "Landlord3_RoomCFG.h"
#include "game_engine.h"
#include "Landlord3_RobotGameCFG.h"


LANDLORD_SPACE_USING


robot_player::robot_player(void)
:m_player(nullptr),
m_init(false),
m_duration(0.0)
{

	m_nGameStatus = eGameState_Free;
	m_nUserStatus = eUserState_free;

	m_lCurJetton = 0;

	m_nCurOperatorID = 0;
	m_nSelfGold = 0;
	m_nGameRound = 0;
	
}

robot_player::~robot_player(void)
{
	
}

//////////////////////////////////////////////////////////////////////////
void robot_player::heartbeat( double elapsed )
{
	if(!m_init) return;
	std::srand(unsigned(std::time(0)));
	if(m_duration>=0)
	{
		m_duration -= elapsed;
	}
	if(m_fLetSee>0)
	{
		m_fLetSee -= elapsed;
	}
	if(m_fLetSee<0)
	{
	}

	if(m_duration<0)
	{
		if( m_nGameStatus==eGameState_Free )
		{
			if(m_nUserStatus == eUserState_free)
				onSendReadyMsg();
		}
		else if(m_nGameStatus==eGameState_FaPai)
		{
            auto lptable = m_player->get_table();
            if (!lptable)
            {
                return ;
            }
            onSendApplyBankerMsg();
            //机器人叫地主;
            int32_t robrate = 2;// m_analyser.CalcGetLandScore(user.p_idx);

            lptable->onEventJiao(m_player->get_pid(), robrate);
		}
		else if(m_nGameStatus==eGameState_Play)
		{ 
		}
		else if(m_nGameStatus==eGameState_End)
		{
			// 
			m_nGameStatus = eGameState_Free;
			m_nUserStatus = eUserState_free;
            m_duration = GetRobotTimes(eGameState_Free);
			//m_duration = global_random::instance().rand_double(TIME_LESS, m_nReadyTime);
		}
		else
		{
			assert(0);
			m_duration = 0.0;
		}
	}

// 	if(m_duration==0)
// 	{
// 		if(m_nGameStatus==eGameState_Free)
// 		{
// 			if(m_nUserStatus == eUserState_free)
// 				onSendReadyMsg();
// 		}
// 	}
}

bool landlord_space::robot_player::onEventEnd(uint32_t playerid, int32_t nEndType)
{
	m_nGameStatus = eGameState_End;
	return true;
}


float landlord_space::robot_player::GetRobotTimes(int8_t nStatus)
{
    auto robotcfg = Landlord3_RobotGameCFG::GetSingleton()->GetData(m_roomid);
    if (robotcfg == nullptr)
    {
        return 0;
    }
    float duration = 0.0;
    switch (nStatus)
    {
	case eGameState_Free:
		duration = global_random::instance().rand_int(3, 5);
    case eGameState_FaPai:
        break;
    case eGameState_Banker:
    {
        int min = robotcfg->mRobotReadyMinTime;
        int max = robotcfg->mRobotReadyMaxTime;
        int time = global_random::instance().rand_int(min, max);
        duration = time / 1000.0;
        break;
    }
    case eGameState_Play:
    {
        int min = robotcfg->mRobotOperaMinTime;
        int max = robotcfg->mRobotOperaMaxTime;
        int time = global_random::instance().rand_int(min, max);
        duration = time / 1000.0;
        break;
    }

    case eGameState_End:
        break;
    default:
        break;
    }
	return duration;
}

uint32_t landlord_space::robot_player::get_pid()
{

	return m_nSelfPlayerID;
}

int64_t landlord_space::robot_player::get_gold()
{

	return m_nSelfGold;
}

int32_t landlord_space::robot_player::get_seat(uint32_t playerid)
{
	for (int i=0;i<GAME_PLAYER;i++)
	{
		auto it = m_MapTableUser.find(i);
		if(it!=m_MapTableUser.end())
		{
			if(it->second.p_id==playerid)
			{
				return it->second.p_idx;
			}
		}
	}
	return INVALID_CHAIR;

}

uint32_t landlord_space::robot_player::get_id(int32_t seat)
{
	uint32_t uid = 0;
	auto it = m_MapTableUser.find(seat);
	if(it!=m_MapTableUser.end())
	{
		return it->second.p_id;
	}

	return uid;
}


void landlord_space::robot_player::initScene()
{
	// 
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetc2l_get_scene_info, landlord3_protocols::e_mst_c2l_get_scene_info);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
}


void landlord_space::robot_player::init(uint32_t uid,int64_t gold,int32_t rid,int32_t tid,int32_t seat)
{
	m_roomid = rid;
	m_tableid = tid;

	m_nSelfPlayerID = uid;
	m_nSelfGold = gold;
	m_nSelfChairID = seat;

	m_nUserStatus = eUserState_free;
	m_nGameStatus = eGameState_Free;

	m_init = true;

	// 读取房间配置
	auto cfg = Landlord3_RoomCFG::GetSingleton()->GetData(rid);
	m_nBaseGold = cfg->mBaseCondition;
	m_nLookCondition = 0;
	m_nPkCondition = 0;
	m_nMaxChip =0;
//	m_VecAddJetton = cfg->mAddChipList;
	// 读取机器人操作配置
	//auto game_cfg = Landlord3_RobotGameCFG::GetSingleton()->GetData(rid);
	//m_nOperaTime = game_cfg->mRobotOperaTime;
	//m_nReadyTime = game_cfg->mRobotReadyTime;
	//m_nResultTime = game_cfg->mRobotResultTime;

}


void landlord_space::robot_player::initGame(int seat,uint32_t p_id,int32_t spare)
{
	if(p_id==m_nSelfPlayerID)
	{
		m_nSelfChairID = seat;
		// 准备
		if(spare == 0)
		{
            m_duration = GetRobotTimes(eGameState_Banker);
			//m_duration = global_random::instance().rand_double(ROBOT_LESS, m_nReadyTime);
		}
		else
		{
			if(spare <= ROBOT_LESS)
			{
				m_duration = 0;
			}
			else
			{
				m_duration = global_random::instance().rand_double(ROBOT_LESS, 3);
			}
			
		}
		SLOG_DEBUG<<"initGame:"<<m_duration;
		m_nUserStatus = eUserState_free;
		m_nGameStatus = eGameState_Free;
	}
}


void landlord_space::robot_player::initGame_play(int seat,uint32_t p_id,int32_t status)
{
	if(p_id==m_nSelfPlayerID)
	{
		m_nSelfChairID = seat;
		
		m_nUserStatus = eUserState_Wait;
		m_nGameStatus = eGameState_Play;

		m_duration = 0;
	}
	else
	{
		m_duration = 0;
	}
}


bool landlord_space::robot_player::onEventUserReady(uint32_t uid)
{
	if(uid==m_nSelfPlayerID)
	{
		m_nUserStatus = eUserState_ready;
	}

	return true;
}

bool landlord_space::robot_player::onEventGameStart(std::map<uint32_t,TableUser> &tableusr, 
											   int64_t usr_gold,int64_t base_gold,uint32_t bankerid,
											   int64_t lPool)
{
	SLOG_CRITICAL << "playerID=" << m_nSelfPlayerID <<" userstatus ="<< m_nUserStatus;
	return true;
}

void landlord_space::robot_player::onSendReadyMsg()
{
	m_duration = 0;
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetc2l_ask_ready, landlord3_protocols::e_mst_c2l_ask_ready);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
	m_nUserStatus = eUserState_ready;
}

void landlord_space::robot_player::onSendApplyBankerMsg()
{
    
}
//////////////////////////////////////////////////////////////////////////
double landlord_space::robot_player::GetFloatTime(double fTime)
{
	double fMin = 0,fMax = 0;
	double nRound = m_nGameRound;
	double fResult = 0;
	uint32_t nAll = m_AllUser.size();
#if 0
	fMin = 2.5 * fTime * 2 / (nRound + 30);
	fMax = 2.5 * fTime * 5 / (nRound + 30);
	fResult = global_random::instance().rand_double(fMin,fMax);
#else
	fResult = fTime * 6.0 * nAll / (nRound+30);
#endif
	SLOG_DEBUG << "GetFloatTime :"<<fResult;

	return fResult;
}
