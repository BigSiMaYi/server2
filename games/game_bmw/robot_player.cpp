#include "stdafx.h"
#include "robot_player.h"
#include "logic_player.h"
#include "logic_table.h"
#include "logic_room.h"

#include "bmw_logic.pb.h"
#include <net/packet_manager.h>

#include "BMW_BaseInfo.h"
#include "BMW_RoomCFG.h"
#include "game_engine.h"



BMW_SPACE_USING

const int32_t robot_player::m_cbJettonArea[PLACE_AREA+1] = {0,
	ID_SMALL_VW,ID_SMALL_Benz, ID_SMALL_BMW,ID_SMALL_Porsche,
	ID_BIG_VW,	ID_BIG_Benz,   ID_BIG_BMW,	ID_BIG_Porsche};

robot_player::robot_player(void)
:m_player(nullptr),
m_init(false),
m_duration(0.0)
{

	m_nGameStatus = eGameState_Free;
	m_nUserStatus = eUserState_free;

	m_nSelfGold = 0;
	m_nExpend = 0;
	m_nGameRound = 0;

	m_ApplyList.clear();
	m_nApplyMin = 10;
	m_nApplyMax = 30;

}

robot_player::~robot_player(void)
{
	
}

//////////////////////////////////////////////////////////////////////////
void robot_player::heartbeat( double elapsed )
{
	if(!m_init) return;
	std::srand(unsigned(std::time(0)));
	if(m_fApply>0)
	{
		m_fApply -= elapsed;
	}
	if(m_fApply<0)
	{ // 达到上庄条件随机上庄
		if(m_nSelfGold >= m_lBankerCondition)
		{
			auto nApplySize = m_ApplyList.size();
			if(nApplySize<4)
			{
				auto search = std::find(m_ApplyList.begin(),m_ApplyList.end(),m_nSelfPlayerID);
				if(search == m_ApplyList.end())
				{
					double fRandom = global_random::instance().rand_double(1,100);
					if(fRandom<=48) onSendApplyBanker();
				}
			}
		}
	}
	if(m_duration>0)
	{
		m_duration -= elapsed;
	}
	if(m_duration<0)
	{
		if( m_nGameStatus==eGameState_Free )
		{
// 			if(m_nUserStatus == eUserState_free)
// 				onSendReadyMsg();
			onHeartApply();
		}
		else if(m_nGameStatus==eGameState_Place)
		{
			onHeartPlace();
		}
		else if(m_nGameStatus==eGameState_Play)
		{ 
			
		}
		else if(m_nGameStatus==eGameState_End)
		{
			// 
			m_nGameStatus = eGameState_Free;
			m_nUserStatus = eUserState_free;

		}
		else
		{
			assert(0);
			m_duration = 0.0;
		}

	}

}

uint32_t bmw_space::robot_player::get_pid()
{

	return m_nSelfPlayerID;
}

int64_t bmw_space::robot_player::get_gold()
{

	return m_nSelfGold;
}

int32_t bmw_space::robot_player::get_seat(uint32_t playerid)
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

uint32_t bmw_space::robot_player::get_id(int32_t seat)
{
	uint32_t uid = 0;
	auto it = m_MapTableUser.find(seat);
	if(it!=m_MapTableUser.end())
	{
		return it->second.p_id;
	}

	return uid;
}


void bmw_space::robot_player::initScene()
{
	// 
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetc2l_get_scene_info, bmw_protocols::e_mst_c2l_get_scene_info);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
}


void bmw_space::robot_player::init(uint32_t uid,int64_t gold,int32_t rid,int32_t tid,int32_t seat)
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
	auto cfg = BMW_RoomCFG::GetSingleton()->GetData(rid);
	m_VecJetton = cfg->mPlaceJetton;
	m_lBankerCondition = cfg->mBankerGold;
	// 
	auto rcfg = BMW_RobotCFG::GetSingleton()->GetData(rid);
	m_nApplyMin = rcfg->mBankerMinEntry/2;
	m_nApplyMax = rcfg->mBankerMaxEntry/2;
	// 读取机器人操作配置
	m_lRobotMinJetton = BMW_ResultCtrl::GetSingleton()->GetData("MinJetton")->mValue;
	m_lRobotMaxJetton = BMW_ResultCtrl::GetSingleton()->GetData("MaxJetton")->mValue;

	m_nRobotMinBetTimes = BMW_ResultCtrl::GetSingleton()->GetData("MinBetTimes")->mValue;
	m_nRobotMaxBetTimes = BMW_ResultCtrl::GetSingleton()->GetData("MaxBetTimes")->mValue;

	m_nRobotMinBankerTimes = BMW_ResultCtrl::GetSingleton()->GetData("MinBankerTimes")->mValue;
	m_nRobotMaxBankerTimes = BMW_ResultCtrl::GetSingleton()->GetData("MaxBankerTimes")->mValue;

	m_lRobotGiveUpMinWinScore = BMW_ResultCtrl::GetSingleton()->GetData("GiveUpMinWinScore")->mValue;
	m_lRobotGiveUpMaxWinScore = BMW_ResultCtrl::GetSingleton()->GetData("GiveUpMaxWinScore")->mValue;

	m_lRobotGiveUpMinLostScore = BMW_ResultCtrl::GetSingleton()->GetData("GiveUpMinLostScore")->mValue;
	m_lRobotGiveUpMaxLostScore = BMW_ResultCtrl::GetSingleton()->GetData("GiveUpMaxLostScore")->mValue;

	m_nRobotGiveUpWinTimes = BMW_ResultCtrl::GetSingleton()->GetData("GiveUpWinTimes")->mValue;
	m_nRobotGiveUpLostTimes = BMW_ResultCtrl::GetSingleton()->GetData("GiveUpLostTimes")->mValue;
	if(m_lRobotMinJetton > m_lRobotMaxJetton)
	{
		m_lRobotMaxJetton = m_lRobotMinJetton;
	}
	if(m_nRobotMinBetTimes > m_nRobotMaxBetTimes)
	{
		m_nRobotMaxBetTimes = m_nRobotMinBetTimes;
	}
	if(m_nRobotMinBankerTimes > m_nRobotMaxBankerTimes)
	{
		m_nRobotMaxBankerTimes = m_nRobotMinBankerTimes;
	}
	if(m_lRobotGiveUpMinWinScore > m_lRobotGiveUpMaxWinScore)
	{
		m_lRobotGiveUpMaxWinScore = m_lRobotGiveUpMinLostScore;
	}
	if(m_lRobotGiveUpMinLostScore > m_lRobotGiveUpMaxLostScore)
	{
		m_lRobotGiveUpMaxLostScore = m_lRobotGiveUpMinLostScore;
	}
}


void bmw_space::robot_player::initGame_free(tagBankerInfo bankerinfo,std::list<uint32_t>& applylist)
{
	m_nUserStatus = eUserState_free;
	m_nGameStatus = eGameState_Free;
	m_ApplyList = applylist;
	if(m_nSelfGold >= m_lBankerCondition)
	{
		//onSendApplyBanker();
		m_fApply = global_random::instance().rand_int(m_nApplyMin,m_nApplyMax);
	}

}

void bmw_space::robot_player::initGame_play(int32_t nGameState,int32_t nLeftTime,
											tagBankerInfo bankerinfo,
											tagAreaInfo areainfo,
											std::list<uint32_t>& applylist)
{
	m_nUserStatus = eUserState_Wait;
	m_nGameStatus = eGameState_Play;
	m_ApplyList = applylist;
	if(m_nSelfGold >= m_lBankerCondition)
	{
		//onSendApplyBanker();
		m_fApply = global_random::instance().rand_int(m_nApplyMin,m_nApplyMax);
	}

}

bool bmw_space::robot_player::onEventUserReady(uint32_t uid)
{
	if(uid==m_nSelfPlayerID)
	{
		m_nUserStatus = eUserState_ready;
	}

	return true;
}


void bmw_space::robot_player::onEventNoticeBanker(tagBankerInfo bankerinfo)
{
	// 游戏开始记录当前庄家
	uint32_t  uid = bankerinfo.uid;
	int64_t money = bankerinfo.money;
	int32_t	round = bankerinfo.round;
	int64_t	result= bankerinfo.result;
	SLOG_CRITICAL<<boost::format("[Robot] Now Banker info :uid:%d,money:%d,round:%d,result:%d") \
		%uid%money%round%result;

	memcpy(&m_NowBanker,&bankerinfo,sizeof(bankerinfo));

	
}


void bmw_space::robot_player::onEventBeginPlace()
{
	// 开始下注
	m_nGameStatus = eGameState_Place;
	//根据庄家金币数量，计算各区域的初始化可下分
	int64_t nBankerGold = m_NowBanker.money;
	uint32_t nBankerID = m_NowBanker.uid;
	int iOddsArray[PLACE_AREA+1] = {0,5,5,5,5,10,20,30,40};
	for(int i=1;i<=PLACE_AREA;i++)
	{
		m_lAreaLimitScore[i] = nBankerGold/(iOddsArray[i]);
	}
	m_bAddScore = (global_random::instance().rand_int(0,1) == 1);

	m_lUserMaxScore = m_nSelfGold;

	if (nBankerID!=m_nSelfPlayerID)
	{
		m_nCurrentBetTimes = m_nRobotMinBetTimes + 
			global_random::instance().rand_int(0,m_nRobotMaxBetTimes - m_nRobotMinBetTimes);

		if(m_nCurrentBetTimes > 0)
		{
			if (m_bAddScore)
			{   
				m_duration = global_random::instance().rand_int(2,4);
			}
			else
			{
				m_duration = global_random::instance().rand_int(3,5);
			}
		}
	}

}

void bmw_space::robot_player::onHeartPlace()
{
	m_duration = 0;
	if (m_nCurrentBetTimes <= 0) return ;
	if(m_nSelfPlayerID == m_NowBanker.uid) return;
	
#if 1
	std::vector<int32_t> VecIdx;
	for (int32_t i=ID_SMALL_VW;i<=ID_BIG_Porsche;i++)
	{
		VecIdx.push_back(i);
	}
	std::random_device rd;
	std::mt19937_64 g(rd());
	std::shuffle(VecIdx.begin(),VecIdx.end(),g);
	int cbAreaIndex = VecIdx.back();
	int64_t lJettonScore = 0;
	// 
	std::vector<int> tmpVecJetton; 
	for (auto chip : m_VecJetton)
	{
		if(chip<=m_lUserMaxScore && chip<=m_lAreaLimitScore[cbAreaIndex] 
		&& chip>=m_lRobotMinJetton && chip<=m_lRobotMaxJetton)
		{
			tmpVecJetton.push_back(chip);
		}
	}
	std::shuffle(tmpVecJetton.begin(),tmpVecJetton.end(),g);
	if(tmpVecJetton.size()>0)
	{
		lJettonScore = tmpVecJetton.back();
	}
	if(lJettonScore > 0)
	{
		onSendPlaceMsg(cbAreaIndex,lJettonScore);
	}
#else
	int cbAreaIndex = global_random::instance().rand_int(ID_SMALL_VW,ID_BIG_Porsche);
	int	nMinJettonIndex = 0,nMaxJettonIndex = JETTON_COUNT-1;
	int cbJettonArea = m_cbJettonArea[cbAreaIndex];
	int64_t lJettonScore = 0;
	if(m_VecJetton.size()!=JETTON_COUNT) 
	{
		assert(0);
	}
	for(int i=0;i<JETTON_COUNT;i++)
	{
		if(nMinJettonIndex == JETTON_COUNT && m_VecJetton[i] >= m_lRobotMinJetton)
		{
			nMinJettonIndex = i;
		}				
		if(m_VecJetton[i]>m_lUserMaxScore ||  m_VecJetton[i] > m_lAreaLimitScore[cbJettonArea] || m_VecJetton[i] > m_lRobotMaxJetton)
		{
			nMaxJettonIndex = i;
			break;
		}
	}
	if(nMaxJettonIndex > nMinJettonIndex)
	{
		int nIdex = nMinJettonIndex + global_random::instance().rand_int(0,nMaxJettonIndex - nMinJettonIndex);
		lJettonScore = m_VecJetton[nIdex];
	}

	if (lJettonScore>m_VecJetton[3])
	{
		std::vector<int> vecIndex;
		for(int i=0;i<4;i++)
		{
			vecIndex.push_back(i*2+2);
		}
		std::random_shuffle(vecIndex.begin(),vecIndex.end());
		for (int i=0;i<4;i++)
		{
			int _cbJettonArea = vecIndex.at(i);
			if (m_lAreaLimitScore[_cbJettonArea] > lJettonScore)
			{
				cbJettonArea = _cbJettonArea;
				break;
			}
		}
	}
	if(lJettonScore > 0)
	{
		onSendPlaceMsg(cbJettonArea,lJettonScore);
	}
#endif
	if(m_nCurrentBetTimes > 0)
	{
		if (m_bAddScore)
		{   
			m_duration = global_random::instance().rand_int(2,4);
		}
		else
		{
			m_duration = global_random::instance().rand_int(3,5);
		}
	}
}

void bmw_space::robot_player::onHeartApply()
{
	uint32_t uid = m_NowBanker.uid;
	if(m_nSelfPlayerID == uid) return;
	
}

void bmw_space::robot_player::onEventPlace(uint32_t playerid,int32_t nArea,int64_t lGold)
{
	// 返回下注结果
	if(playerid==m_nSelfPlayerID)
	{
		m_lUserMaxScore -= lGold;

		m_nCurrentBetTimes--;
	}
}


void bmw_space::robot_player::onEventNoticeFull()
{
	m_nGameStatus = eGameState_Play;
}


void bmw_space::robot_player::onEventNoticeRun(int32_t nResult)
{

	m_nGameStatus = eGameState_Play;
}

void bmw_space::robot_player::onEventApplyBanker(uint32_t playerid,std::list<uint32_t>& applylist)
{
	// 返回上庄结果
	m_ApplyList.clear();
	m_ApplyList = applylist;

}

void bmw_space::robot_player::onEventUnApplyBanker(uint32_t playerid,std::list<uint32_t>& applylist)
{
	// 返回下庄结果
	m_ApplyList.clear();
	m_ApplyList = applylist;
}

bool bmw_space::robot_player::IsValidPlayer(uint32_t uid)
{
	auto& player = game_engine::instance().get_lobby().get_player(uid);
	if(player)
	{
		return true;
	}
	SLOG_WARNING<<boost::format("Invalid Player: %d")%uid;
	return false;
}

void bmw_space::robot_player::onSendReadyMsg()
{
	m_duration = 0;
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetc2l_ask_ready, bmw_protocols::e_mst_c2l_ask_ready);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
}

void bmw_space::robot_player::onSendPlaceMsg(int32_t nArea,int64_t lGold)
{
	m_duration = 0;
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetc2l_ask_place_bet, bmw_protocols::e_mst_c2l_ask_place_bet);
	sendmsg->set_betarea(nArea);
	sendmsg->set_betvalue(lGold);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
}

void bmw_space::robot_player::onSendApplyBanker()
{
	m_fApply = 0;
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetc2l_ask_apply_banker, bmw_protocols::e_mst_c2l_ask_apply_banker);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
}

void bmw_space::robot_player::onSendUnApplyBanker()
{
	m_duration = 0;
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetc2l_ask_unapply_banker, bmw_protocols::e_mst_c2l_ask_unapply_banker);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
}

bool bmw_space::robot_player::onEventEnd(uint32_t playerid,int64_t nResult)
{
	// ok
	m_nGameStatus = eGameState_End;
//	m_nUserStatus = eUserState_free;

	m_duration = global_random::instance().rand_double(TIME_COMPARE,TIME_COMPARE+TIME_LESS);
	// 清理

	return true;
}



double bmw_space::robot_player::GetFloatTime(double fTime)
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



//////////////////////////////////////////////////////////////////////////


