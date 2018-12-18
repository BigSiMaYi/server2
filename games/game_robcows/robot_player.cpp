#include "stdafx.h"
#include "robot_player.h"
#include "logic_player.h"
#include "logic_table.h"
#include "logic_room.h"

#include "robcows_logic.pb.h"
#include <net/packet_manager.h>

#include "RobCows_BaseInfo.h"
#include "RobCows_RoomCFG.h"
#include "game_engine.h"
#include "RobCows_RobotGameCFG.h"
#include <random>


ROBCOWS_SPACE_USING


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
	if(m_duration>0)
	{
		m_duration -= elapsed;
	}

// 	char buf[256] = {0};
// 	sprintf(buf,":[%p]:%f",&m_duration,m_duration);
// 	SLOG_CRITICAL<<"heartbeat:"<<buf;
//	SLOG_CRITICAL<<global_random::instance().rand_int(TIME_LESS, 15); 
	if(m_duration<0)
	{
		if( m_nGameStatus==eGameState_Free )
		{
// 			if(m_nUserStatus == eUserState_free)
// 				onSendReadyMsg();
		}
		else if(m_nGameStatus==eGameState_FaPai)
		{
		}
		else if(m_nGameStatus==eGameState_Banker)
		{
			onSendRobBankerMsg();
		}
		else if(m_nGameStatus==eGameState_Bet)
		{
			onSendBetMsg();
		}
		else if(m_nGameStatus==eGameState_OpenCard)
		{
			onSendOpenCardMsg();
		}
		else if(m_nGameStatus==eGameState_Display)
		{

		}
		else if(m_nGameStatus==eGameState_End)
		{
			m_nGameStatus = eGameState_Free;
			m_nUserStatus = eUserState_free;

			m_duration = global_random::instance().rand_double(TIME_LESS1, m_nResultTime);
		}
		else
		{
			assert(0);
			m_duration = 0.0;
		}

	}
	//
// 	if(m_duration==0)
// 	{
// 		if(m_nGameStatus==eGameState_Free)
// 		{
// 			if(m_nUserStatus == eUserState_free)
// 				onSendReadyMsg();
// 		}
// 	}

}


uint32_t robcows_space::robot_player::get_pid()
{

	return m_nSelfPlayerID;
}

int64_t robcows_space::robot_player::get_gold()
{

	return m_nSelfGold;
}

int32_t robcows_space::robot_player::get_seat(uint32_t playerid)
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

uint32_t robcows_space::robot_player::get_id(int32_t seat)
{
	uint32_t uid = 0;
	auto it = m_MapTableUser.find(seat);
	if(it!=m_MapTableUser.end())
	{
		return it->second.p_id;
	}

	return uid;
}


void robcows_space::robot_player::initScene()
{
	// 
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetc2l_get_scene_info, robcows_protocols::e_mst_c2l_get_scene_info);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
}


void robcows_space::robot_player::init(uint32_t uid,int64_t gold,int32_t rid,int32_t tid,int32_t seat)
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
	auto cfg = RobCows_RoomCFG::GetSingleton()->GetData(rid);
	m_nBaseGold = cfg->mBaseCondition;
	m_nLookCondition = 0;
	m_nPkCondition = 0;
	m_nMaxChip = 0;
	m_VecRobRate = cfg->mRobBankerList;
	m_VecBetRate = cfg->mBetList;
	// 读取机器人操作配置
	auto game_cfg = RobCows_RobotGameCFG::GetSingleton()->GetData(rid);
	m_nReadyTime = game_cfg->mReadyTime;
	m_nBankerTime = game_cfg->mBankerTime;
	m_VecRandRob = game_cfg->mRobList;
	m_nBetTime = game_cfg->mBetTime;
	m_VecRandBet = game_cfg->mBetList;
	m_nOperaTime = game_cfg->mOperaTime;
	m_nResultTime = game_cfg->mResultTime;

	std::reverse(m_VecRobRate.begin(), m_VecRobRate.end());
	std::reverse(m_VecBetRate.begin(), m_VecBetRate.end());

}


void robcows_space::robot_player::initGame(int seat,uint32_t p_id,int32_t spare)
{
	if(p_id==m_nSelfPlayerID)
	{
		m_nSelfChairID = seat;
		// 准备
		if(spare == 0)
		{
			m_duration = global_random::instance().rand_double(TIME_LESS2, m_nReadyTime);
		}
		else
		{
			if(spare <= TIME_LESS2)
			{
				m_duration = 0;
			}
			else
			{
				m_duration = global_random::instance().rand_double(TIME_LESS2, spare);
			}
			
		}

		SLOG_DEBUG<<"initGame:"<<m_duration;

		m_nUserStatus = eUserState_free;
		m_nGameStatus = eGameState_Free;
	}
	else
	{
		m_duration = 0;
	}

}


void robcows_space::robot_player::initGame_play(int seat,uint32_t p_id,int32_t status)
{
	if(p_id==m_nSelfPlayerID)
	{
		m_nSelfChairID = seat;
		
		m_nUserStatus = eUserState_Wait;
	

		m_duration = 0;
	}
	else
	{
		m_duration = 0;
	}
}


bool robcows_space::robot_player::onEventUserReady(uint32_t uid)
{
	if(uid==m_nSelfPlayerID)
	{
		m_nUserStatus = eUserState_ready;
	}

	return true;
}

bool robcows_space::robot_player::onEventGameStart(std::map<uint32_t,TableUser> &tableusr, 
											   int64_t usr_gold,int64_t base_gold,uint32_t bankerid,
											   int64_t lPool)
{
	m_MapTableUser.clear();
	m_nGameRound = 0;
	m_nCurOperatorID = 0;
	m_nFirstID = 0;



	m_AllUser.clear();
	m_CheckUser.clear();
	m_RobotUser.clear();
	m_CompareUser.clear();

	m_lPoolGold = lPool;

	m_nGameStatus = eGameState_FaPai;
	m_nUserStatus = eUserState_play;
	//
	m_MapTableUser = tableusr;
	// 初始化数据
	for (auto it = m_MapTableUser.begin();it!=m_MapTableUser.end();it++)
	{
		auto & usr = it->second;
		uint32_t uid = usr.p_id;
		int32_t nActive = usr.p_active;
		if(nActive==TRUE)
		{ // 参与了游戏
			// label
			auto& player = game_engine::instance().get_lobby().get_player(uid);
			if(player)
			{
				usr.p_tag = player->get_player_tag();
				usr.p_expend = base_gold;
				m_AllUser.push_back(uid);
				// Robot
				if(player->is_robot())
				{
					m_RobotUser.push_back(uid);
				}
			}

		}
		else
		{ // 旁观等待

		}
	}

	// 设置参数
	m_nSelfGold = usr_gold;
	m_nBaseGold = base_gold;
	m_lCurJetton = m_nBaseGold;
	
	return true;
}


bool robcows_space::robot_player::onEventNoticeStart(uint32_t uid)
{
	m_nCurOperatorID = uid;
	m_nFirstID = uid;
	m_nGameRound++;
	if(uid == m_nSelfPlayerID)
	{
		// 操作
		m_duration = global_random::instance().rand_double(TIME_LESS2, m_nOperaTime);
		SLOG_DEBUG<<"onEventNoticeStart:"<<m_duration;
	}
	else
	{
		m_duration = 0;
	}

	return true;
}


bool robcows_space::robot_player::CanAddJetton(int64_t lJetton)
{
	if(lJetton>m_nMaxChip) return false;
	else if (lJetton>m_nSelfGold) return false;
	else if (lJetton<=m_lCurJetton) return false;
	return true;
}

bool robcows_space::robot_player::CanCompare()
{
	if (m_nGameRound>=m_nPkCondition)
	{
		int64_t lNeed = m_lCurJetton * NeedDouble() * 2;
		if(m_nSelfGold >= lNeed)
		{
			return true;
		}
	}
	return false;
}

bool robcows_space::robot_player::IsCheck()
{
	auto it = std::find(m_CheckUser.begin(),m_CheckUser.end(),m_nSelfPlayerID);
	if(it != m_CheckUser.end())
		return true;
	return false;
}

bool robcows_space::robot_player::CanCheck()
{
	if(m_nGameRound>=m_nLookCondition)
	{
		return true;
	}
	return false;
}

bool robcows_space::robot_player::CanFlow()
{
	int64_t lNeed = m_lCurJetton * NeedDouble();
	if(m_nSelfGold >= lNeed)
	{
		return true;
	}
	return false;
}

bool robcows_space::robot_player::CanGiveUp()
{

	return true;
}

bool robcows_space::robot_player::CanShowHand()
{

	return true;
}

int robcows_space::robot_player::NeedDouble()
{
	auto it = std::find(m_CheckUser.begin(),m_CheckUser.end(),m_nSelfPlayerID);
	if(it != m_CheckUser.end())
	{
		return 2;
	}
	return 1;
}

void robcows_space::robot_player::onSendReadyMsg()
{
	m_duration = 0;
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetc2l_ask_ready, robcows_protocols::e_mst_c2l_ask_ready);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
	SLOG_CRITICAL<<boost::format("Robot:Send Ready");
}

void robcows_space::robot_player::onSendCheckMsg()
{
	
}

void robcows_space::robot_player::onSendFlowMsg()
{

}

void robcows_space::robot_player::onSendAddMsg(int64_t lAdd)
{
	
}

void robcows_space::robot_player::onSendShowHandMsg()
{

}

void robcows_space::robot_player::onSendGiveUpMsg()
{
	
}



uint32_t robcows_space::robot_player::FindCompareUser()
{
	uint32_t nCompareid = 0;
	if(nCompareid = FindNoSelf(m_CompareUser))
	{
		return nCompareid;
	}
	else if(nCompareid = FindNoSelf(m_CheckUser))
	{
		return nCompareid;
	}
	else if(nCompareid = FindNoSelf(m_AllUser))
	{
		return nCompareid;
	}
	//
	return nCompareid;
}

uint32_t robcows_space::robot_player::FindNoSelf(VectorUserID &vecusers)
{
	uint32_t nFindID = 0;
	VectorUserID findvec = vecusers;
	auto it = std::find(findvec.begin(),findvec.end(),m_nSelfPlayerID);
	if(it!=findvec.end())
	{
		findvec.erase(it);
	}
	if(findvec.size()>0)
	{
		std::random_shuffle(findvec.begin(),findvec.end());
		nFindID = findvec.back();
	}
	return nFindID;
}

void robcows_space::robot_player::onSendCompareMsg(uint32_t playerid)
{
	
}

void robcows_space::robot_player::onSendAllInMsg()
{

}

bool robcows_space::robot_player::onEventFlow(uint32_t uid,uint32_t nextid,
										  int64_t lNeedGold,
										  int32_t nRound,
										  int64_t lPool)
{
	return true;
}

bool robcows_space::robot_player::onEventCheck(uint32_t uid)
{
	// ok
	return false;
}

bool robcows_space::robot_player::onEventGiveUP(uint32_t uid,uint32_t nextid,int32_t nRound)
{
	// ok
	return true;
}

bool robcows_space::robot_player::onEventAdd(uint32_t uid,uint32_t nextid,
										 int64_t lCurGold,int64_t lNeedGold,
										 int32_t nRound,
										 int64_t lPool)
{
	// ok
	return true;
}

bool robcows_space::robot_player::onEventCompare(uint32_t reqid,int64_t lNeedGold, 
											 uint32_t resid,uint32_t winid, 
											 uint32_t nextid, 
											 int32_t nRound,int64_t lPool)
{
	//  ok
	return true;
}

bool robcows_space::robot_player::onEventNoticeAllIn(uint32_t uid)
{
	return true;
}

bool robcows_space::robot_player::onEventAllIn(uint32_t uid,bool bWin,int64_t lNeedGold,int64_t lPool,uint32_t nextid,int32_t nRound)
{
	return true;
}

bool robcows_space::robot_player::onEventShowHand(uint32_t uid,uint32_t nextid,int64_t lNeedGold,int64_t lPool)
{
	return true;
}

bool robcows_space::robot_player::onEventEnd(uint32_t playerid)
{
	// ok
	m_nGameStatus = eGameState_End;
	m_nUserStatus = eUserState_free;
	
	m_duration = global_random::instance().rand_double(m_nResultTime+TIME_LESS2, m_nResultTime+TIME_LESS2*2);

	SLOG_DEBUG<<"onEventEnd:"<<m_duration;
	// 清理

	return true;
}

//////////////////////////////////////////////////////////////////////////

int32_t robcows_space::robot_player::GetLabelByValue(int64_t nValue)
{
	if(nValue<=Tag_UserA)
	{
		return Label_UserA;
	}
	else if(nValue<=Tag_UserB)
	{
		return Label_UserB;
	}
	else
	{
		return Label_UserC;
	}
}

int32_t robcows_space::robot_player::GetMaxLabel()
{
	// 	int32_t nMaxLab = Tag_UserA;
	// 	for(auto tableusr: m_MapTableUser)
	// 	{
	// 		auto user = tableusr.second;
	// 		if(user.p_active==TRUE) 
	// 		{
	// 			int32_t nTag = user.p_tag;
	// 			if(nMaxLab <= nTag)
	// 			{
	// 				nMaxLab = nTag;
	// 			}
	// 		}
	// 	}
	// C 最大
	for(auto user: m_MapTableUser)
	{
		if(user.second.p_active==TRUE && user.second.p_tag == Tag_UserC) 
		{
			return Tag_UserC;
		}
	}
	// 没有C 找B
	for(auto user: m_MapTableUser)
	{
		if(user.second.p_active==TRUE && user.second.p_tag == Tag_UserB) 
		{
			return Tag_UserB;
		}
	}
	// 没有B 找A
	for(auto user: m_MapTableUser)
	{
		if(user.second.p_active==TRUE && user.second.p_tag == Tag_UserA) 
		{
			return Tag_UserA;
		}
	}
	return Tag_Robot;
}


int32_t robcows_space::robot_player::GetPlayerLabel(uint32_t playerid)
{
	int32_t nLabel = Tag_UserA;
	auto it = m_MapTableUser.find(playerid);
	if(it!=m_MapTableUser.end())
	{
		auto & user = it->second;
		auto& player = game_engine::instance().get_lobby().get_player(playerid);
		if(player)
		{
			tagPlayerCtrlAttr attr;
			bool bRet = player->get_player_ctrl(attr);
			user.p_tag = attr.nTag;
		}
	}
	return nLabel;
}

void robcows_space::robot_player::SetUserActive(uint32_t playerid,BOOL bActive/*=FALSE*/)
{
	auto it = m_MapTableUser.find(playerid);
	if(it!=m_MapTableUser.end())
	{
		auto& user = it->second;
		uint32_t uid = user.p_id;
		if(uid==playerid)
		{
			user.p_active = bActive;
			if(bActive==FALSE)
			{// del
				DeleteGiveUpUser(playerid);
			}
		}
	}

}

void robcows_space::robot_player::DeleteGiveUpUser(uint32_t playerid)
{
	// del
	auto all_it = std::find(m_AllUser.begin(),m_AllUser.end(),playerid);
	if(all_it != m_AllUser.end())
	{
		m_AllUser.erase(all_it);
	}
	// Robot
	auto rb_it = std::find(m_RobotUser.begin(),m_RobotUser.end(),playerid);
	if (rb_it != m_RobotUser.end())
	{
		m_RobotUser.erase(rb_it);
	}
	// Cheaker
	auto check_it = std::find(m_CheckUser.begin(),m_CheckUser.end(),playerid);
	if(check_it != m_CheckUser.end())
	{
		m_CheckUser.erase(check_it);
	}
	// compare
	auto compare_it = std::find(m_CompareUser.begin(),m_CompareUser.end(),playerid);
	if(compare_it != m_CompareUser.end())
	{
		m_CompareUser.erase(compare_it);
	}
}

void robcows_space::robot_player::AddUserExpend(uint32_t playerid,int64_t lGold)
{
	auto it = m_MapTableUser.find(playerid);
	if(it!=m_MapTableUser.end())
	{
		auto& user = it->second;
		user.p_expend += lGold;
	}
}


int64_t robcows_space::robot_player::GetAddGold()
{
	if(m_lCurJetton == m_VecAddJetton.back())
	{
		return 0;
	}
	int64_t nAddGold = 0;
	// m_VecAddRate
	int nRandom = global_random::instance().rand_int(1,100);
	int nSum = 0;
	for (uint32_t i=0;i<m_VecAddRate.size();i++)
	{
		nSum += m_VecAddRate[i];
		if(nRandom>=nSum)
		{
			nAddGold = m_VecAddJetton[i];
			break;
		}
	}
	if(CanAddJetton(nAddGold)==false)
	{
		nAddGold = 0;
	}
	return nAddGold;
}

double robcows_space::robot_player::GetFloatTime(double fTime)
{
	double fMin = 0,fMax = 0;
	double nRound = m_nGameRound;
	fMin = 2.5 * fTime * 2 / (nRound + 30);
	fMax = 2.5 * fTime * 5 / (nRound + 30);
	double fResult = global_random::instance().rand_double(fMin,fMax);
	SLOG_DEBUG << "GetFloatTime :"<<fResult;
	return fResult;
}

void robcows_space::robot_player::onTellSpare()
{
	// 游戏即将开始
	m_duration = 0;

}

void robcows_space::robot_player::onTellStart(std::map<uint32_t,TableUser> &tableusr)
{
	m_MapTableUser.clear();
	m_nBankerID = 0;
	m_nSelfGold = 0;
	m_nBankerRate = 0;

	m_nGameStatus = eGameState_FaPai;
	m_nUserStatus = eUserState_play;

	m_duration = 0;

	m_MapTableUser = tableusr;
	// 初始化数据
	for (auto it = m_MapTableUser.begin();it!=m_MapTableUser.end();it++)
	{
		auto & usr = it->second;
		uint32_t uid = usr.p_id;
		int32_t nActive = usr.p_active;
		if(nActive==TRUE)
		{ // 参与了游戏
			// label
			auto& player = game_engine::instance().get_lobby().get_player(uid);
			if(player)
			{
				if(uid==m_nSelfPlayerID)
				{
					m_nSelfGold = player->get_gold();
				}
				tagPlayerCtrlAttr attr;
				bool bRet = player->get_player_ctrl(attr);
				int32_t nTag = Tag_UserA;
				if(bRet)
				{
					nTag = attr.nTag;
				}
				usr.p_tag = nTag;
			}

		}
		else
		{ // 旁观等待

		}
	}
}


void robcows_space::robot_player::onTellRobBanker(std::map<uint32_t, TableUser> &tableusr)
{
	m_MapTableUser.clear();
	m_nBankerID = 0;
	m_nSelfGold = 0;
	m_nBankerRate = 0;

	m_nGameStatus = eGameState_FaPai;
	m_nUserStatus = eUserState_play;

	m_duration = 0;

	m_MapTableUser = tableusr;
	// 初始化数据
	for (auto it = m_MapTableUser.begin(); it != m_MapTableUser.end(); it++)
	{
		auto & usr = it->second;
		uint32_t uid = usr.p_id;
		int32_t nActive = usr.p_active;
		if (nActive == TRUE)
		{ // 参与了游戏
		  // label
			auto& player = game_engine::instance().get_lobby().get_player(uid);
			if (player)
			{
				if (uid == m_nSelfPlayerID)
				{
					m_nSelfGold = player->get_gold();
				}
				tagPlayerCtrlAttr attr;
				bool bRet = player->get_player_ctrl(attr);
				int32_t nTag = Tag_UserA;
				if (bRet)
				{
					nTag = attr.nTag;
				}
				usr.p_tag = nTag;
			}

		}
		else
		{ // 旁观等待

		}
	}
	/* 开始抢庄*/
	m_nGameStatus = eGameState_Banker;
	if(m_nUserStatus!=eUserState_play) return;
	m_duration = global_random::instance().rand_double(TIME_LESS2, m_nBankerTime);
}

void robcows_space::robot_player::onTellBetRate(uint32_t nBanker,int32_t nRobRate)
{
	/* 开始下注*/
	m_nGameStatus = eGameState_Bet;
	if(m_nUserStatus!=eUserState_play) return;
	m_nBankerID = nBanker;
	m_nBankerRate = nRobRate;
	if(m_nSelfPlayerID != nBanker)
	{
		m_duration = global_random::instance().rand_double(TIME_LESS2, m_nBetTime);
	}
	else
	{
		m_duration = 0;
	}
}

void robcows_space::robot_player::onTellOpenCard()
{
	/* 开始开牌*/
	m_nGameStatus = eGameState_OpenCard;
	if(m_nUserStatus!=eUserState_play) return;
	m_duration = global_random::instance().rand_double(TIME_LESS2, m_nOperaTime);

}

void robcows_space::robot_player::onTellGameResult()
{
	/* 游戏结算*/
	m_nGameStatus = eGameState_End;
	m_nUserStatus = eUserState_free;

	m_duration = global_random::instance().rand_double(m_nResultTime, m_nResultTime+m_nReadyTime);

}

int32_t robcows_space::robot_player::GetMaxRobRate()
{
	int32_t nMaxRate = 0;
	uint32_t nCount = m_VecRobRate.size();
	for (auto rob : m_VecRobRate)
	{
		if (m_nSelfGold >= m_nBaseGold * 30 * rob)
		{
			nMaxRate = rob;
			break;
		}
	}
	if (nMaxRate == 0)
	{
		nMaxRate = m_VecRobRate.back();
	}
	return nMaxRate;
}


int32_t robcows_space::robot_player::GetRobBankerRate()
{
	int32_t nRate = m_VecRobRate.back();
	int32_t nMaxRate = GetMaxRobRate();
	SLOG_CRITICAL<<boost::format("Robot:RobRate,MaxRate:%d, default:%d")% nMaxRate%nRate; 
	SLOG_CRITICAL<<boost::format("Robot:RobRate,Gold:%d, base:%d") % m_nSelfGold%m_nBaseGold;
	std::vector<int> tmp;
	for (auto rate : m_VecRandRob)
	{
		if (rate <= nMaxRate)
		{
			tmp.push_back(rate);
		}
	}
	if (tmp.size() > 0)
	{
		std::random_device rd;
		std::mt19937_64 g(rd());
		std::shuffle(tmp.begin(), tmp.end(), g);

		nRate = tmp.back();
	}
	return nRate;
}

void robcows_space::robot_player::onSendRobBankerMsg()
{
	m_duration = 0;
	if(m_nUserStatus == eUserState_Wait) return;
	int32_t nRobRate = GetRobBankerRate();
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetc2l_ask_robbank, robcows_protocols::e_mst_c2l_ask_robbanker);
	sendmsg->set_robrate(nRobRate);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);

	SLOG_CRITICAL<<boost::format("[CHK]Robot:rob-banker:%d")%nRobRate;
}

void robcows_space::robot_player::onSendBetMsg()
{
	m_duration = 0;
	if(m_nUserStatus == eUserState_Wait) return;
	int32_t nBetRate = GetBetRate();
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetc2l_ask_bet, robcows_protocols::e_mst_c2l_ask_bet);
	sendmsg->set_betrate(nBetRate);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
	SLOG_CRITICAL<<boost::format("[CHK]Robot:bet-rate:%d")%nBetRate;
}

void robcows_space::robot_player::onSendOpenCardMsg()
{
	m_duration = 0;
	if(m_nUserStatus != eUserState_play) return;
	auto sendmsg = PACKET_CREATE(robcows_protocols::packetc2l_ask_opencard, robcows_protocols::e_mst_c2l_ask_opencard);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
	SLOG_CRITICAL<<boost::format("Robot:open-card");
}

int32_t robcows_space::robot_player::GetMaxBetRate()
{
	int32_t nMaxRate = 0;
	uint32_t nCount = m_VecBetRate.size();
	for (auto bet : m_VecBetRate)
	{
		if (m_nSelfGold > m_nBaseGold*m_nBankerRate*bet)
		{
			nMaxRate = bet;
			break;
		}
	}
	if (nMaxRate == 0)
	{
		nMaxRate = m_VecBetRate.back();
	}
	return nMaxRate;
}


int32_t robcows_space::robot_player::GetBetRate()
{
	int32_t nRate = m_VecBetRate.back();
	int32_t nMaxRate = GetMaxBetRate();
	SLOG_CRITICAL << boost::format("Robot:BetRate,MaxRate:%d, default:%d") % nMaxRate%nRate;
	SLOG_CRITICAL << boost::format("Robot:BetRate,Gold:%d, base:%d") % m_nSelfGold%m_nBaseGold;
	std::vector<int> tmp;
	for (auto rate : m_VecRandBet)
	{
		if (rate <= nMaxRate)
		{
			tmp.push_back(rate);
		}
	}
	if (tmp.size() > 0)
	{
		std::random_device rd;
		std::mt19937_64 g(rd());
		std::shuffle(tmp.begin(), tmp.end(), g);

		nRate = tmp.back();
	}
	return nRate;
}

