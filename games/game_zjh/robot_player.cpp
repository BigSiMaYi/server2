#include "stdafx.h"
#include "robot_player.h"
#include "logic_player.h"
#include "logic_table.h"
#include "logic_room.h"

#include "zjh_logic.pb.h"
#include <net/packet_manager.h>

#include "GoldFlower_BaseInfo.h"
#include "GoldFlower_RoomCFG.h"
#include "game_engine.h"
#include "GoldFlower_RobotGameCFG.h"
#include "GoldFlower_RobotActionCFG.h"


ZJH_SPACE_USING


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
	m_nExpend = 0;
	m_nGameRound = 0;
	m_nShowHandID = 0;

	m_nPkCondition = 0;
	m_nLookCondition = 0;
	m_bChecking = false;
	m_bIsGod = false;
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
	// 关闭亮牌功能
// 	if(m_fLetSee>0)
// 	{
// 		m_fLetSee -= elapsed;
// 	}
// 	if(m_fLetSee<0)
// 	{
// 		if(m_nGameStatus==eGameState_End)
// 		{
// 			double nRandom = global_random::instance().rand_double(1,100);
// 			if(nRandom<=12 && m_nUserStatus!=eUserState_Wait)
// 			{
// 				onSendLetSeeMsg();
// 			}
// 			else
// 			{
// 				// 只有一次机会
// 				m_fLetSee = 0;
// 			}
// 		}
// 		else
// 		{
// 
// 		}
// 	}
// 	char buf[256] = {0};
// 	sprintf(buf,":[%p]:%f",&m_duration,m_duration);
// 	SLOG_CRITICAL<<"heartbeat:"<<buf;
//	SLOG_CRITICAL<<global_random::instance().rand_int(TIME_LESS, 15); 
	if(m_duration<0)
	{
		if( m_nGameStatus==eGameState_Free )
		{
			if(m_nUserStatus == eUserState_free)
				onSendReadyMsg();
		}
		else if(m_nGameStatus==eGameState_Spare)
		{
		}
		else if(m_nGameStatus==eGameState_FaPai)
		{
		}
		else if(m_nGameStatus==eGameState_Play)
		{ 
			if (m_nUserStatus == eUserState_Wait) return;
			double nRandom = global_random::instance().rand_01();
			char buf[MAX_PATH] = {0};
			sprintf(buf,"nRandom:%f,curid=%d,selfid=%d",nRandom,m_nCurOperatorID,m_nSelfPlayerID);
			SLOG_DEBUG<<"heartbeat:"<<buf;
			if(m_nCurOperatorID != m_nSelfPlayerID)
			{ // 看牌--但是不检测弃牌
				if(CanCheck())
				{
					onSendCheckMsg();
				}
				else
				{
					// ADD  看牌弃牌
					if (CanGiveUp() && m_RobotUser.size() > 1)
					{
						double nGiveupRand = GetGiveUpRate();
						if(nRandom <= nGiveupRand)
						{
							onSendGiveUpMsg();
						}
						SLOG_DEBUG << boost::format("[U-Active-Period]Random:%1%,nGiveupRand:%2%") % nRandom%nGiveupRand;
					}
					m_duration = 0;
				}
			}
			else
			{ // 
				double nRand_Showhand = GetShowHandRate2Active();
				double nRand_Giveup = GetGiveUpRate();
				double nRand_Add = GetAddRate();
				double nRand_Compare = GetCompareRate();
		//		double nRand_Flow = GetFlowRate();
		//		double nRand_Check = GetCheckRate();
				//
				SLOG_DEBUG << boost::format("heartbeat:[HELP]Rate:showhand[%1%],giveup[%2%],add[%3%],cmp[%4%]")%nRand_Showhand \
					%nRand_Giveup%nRand_Add%nRand_Compare;
				SLOG_WARNING << boost::format("heartbeat:[HELP]GameInfo:Round[%1%],Cur-Chip[%2%],Max-Chip[%3%]")%m_nGameRound \
					%m_lCurJetton%m_nMaxChip;
				//////////////////////////////////////////////////////////////////////////
				// @空闲时间看牌事件延续到激活时间
				if (m_bChecking && CanCheck())
				{
					m_bChecking = false;
					onSendCheckMsg();
					return;
				}
				m_bChecking = false;
				// 梭哈请求优先处理
				if(m_bHasShowHand)
				{
					nRand_Showhand = GetShowHandRate2Passive();
					SLOG_DEBUG << boost::format("heartbeat:[HELP]Rate:Passive-showhand[%1%],nRandom[%2%]") % nRand_Showhand%nRandom;
					if(nRandom <= nRand_Showhand)
					{
						onSendShowHandMsg();
					}
					else
					{
						if(CanCheck())
						{
							onSendCheckMsg();
						}
						else if(CanGiveUp())
						{
							onSendGiveUpMsg();
						}
						else
						{
							SLOG_DEBUG<<boost::format("[Robot] Nothing TODO line:%1%")%__LINE__;
						}
					}
					return;
				}
				// 孤注一掷处理
				if(m_bHasAllin)
				{
					onSendAllInMsg();
					return;
				}

				int32_t nActionIdx = -1;
				// 概率单独计算
				if(nRandom <= nRand_Showhand)
				{
					nActionIdx = 0;
					onSendShowHandMsg();
				}
				else
				{
					nRandom = global_random::instance().rand_01();
					// @ 已看牌优先
					if (IsCheck())
					{
						if (nRandom <= nRand_Giveup)
						{
							if (CanGiveUp())
							{
								nActionIdx = 15;
								onSendGiveUpMsg();
								return;
							}
						}

					}
					// @ 调整操作优先级 比牌 优先 加注
					SLOG_DEBUG << "heartbeat:check cmp: " << nRandom;
					if (nRandom <= nRand_Compare)
					{
						SLOG_DEBUG << "heartbeat:compare";
						uint32_t nCompareID = FindCompareUser();
						SLOG_DEBUG << "heartbeat:compare id:" << nCompareID;
						if (nCompareID != 0 && nCompareID != m_nSelfPlayerID && IsValidPlayer(nCompareID))
						{
							nActionIdx = 2;
							int64_t lNeed = m_lCurJetton * NeedDouble() * 2;
							if (m_nSelfGold >= lNeed)
							{
								onSendCompareMsg(nCompareID);
							}
							else
							{
								onSendCompareExMsg();
							}

						}
						else
						{
							SLOG_WARNING << boost::format("[Robot] Nothing TODO line:%1%") % __LINE__;
						}
					}
					else
					{
						nRandom = global_random::instance().rand_01();
						SLOG_DEBUG << "heartbeat:check add: " << nRandom;
						if (nRandom <= nRand_Add)
						{
							SLOG_DEBUG << "heartbeat:add";
							int64_t lAddGold = GetAddGold();
							if (lAddGold > 0)
							{
								nActionIdx = 1;
								onSendAddMsg(lAddGold);
							}
							else
							{// 加注概率事件可能会失败
								if (CanFlow())
								{
									nActionIdx = 10;
									onSendFlowMsg();
								}
								else
								{
									// @ Add 10/2/2017
									SLOG_DEBUG << "heartbeat:compare4";
									uint32_t nCompareID = FindCompareUser();
									SLOG_DEBUG << "heartbeat:compare4 id:" << nCompareID;
									if (nCompareID != 0 && nCompareID != m_nSelfPlayerID && IsValidPlayer(nCompareID))
									{
										nActionIdx = 32;
										int64_t lNeed = m_lCurJetton * NeedDouble() * 2;
										if (m_nSelfGold >= lNeed)
										{
											onSendCompareMsg(nCompareID);
										}
										else
										{
											onSendCompareExMsg();
										}
									}
									else
									{
										SLOG_WARNING << boost::format("[Robot] Nothing TODO line:%1%") % __LINE__;
									}
								}
							}
						}
						else
						{
							nRandom = global_random::instance().rand_01();
							SLOG_DEBUG<<"heartbeat:check give-up: "<<nRandom;
							if(nRandom<=nRand_Giveup)
							{
								if (IsCheck())
								{
									if (CanGiveUp())
									{
										nActionIdx = 14;
										onSendGiveUpMsg();
									}
									else
									{
										SLOG_WARNING << boost::format("[Robot] Nothing TODO line:%1%") % __LINE__;
									}
								}
								else
								{
									if (CanCheck())
									{
										nActionIdx = 3;
										onSendCheckMsg();
									}
									else if (CanFlow())
									{
										nActionIdx = 13;
										onSendFlowMsg();
									}
									else if (CanGiveUp())
									{
										nActionIdx = 4;
										onSendGiveUpMsg();
									}
									else
									{
										SLOG_WARNING << boost::format("[Robot] Nothing TODO line:%1%") % __LINE__;
									}
								}
							}
							else
							{// 此时不能再也不能弃牌
								// 先跟注
								if(CanFlow())
 								{
 									nActionIdx = 5;
 									onSendFlowMsg();
 								}
								else if (CanCheck())
								{
									nActionIdx = 6;
									onSendCheckMsg();
								}
								else
								{
									// 再验证比牌
									SLOG_DEBUG << "heartbeat:compare2";
									uint32_t nCompareID = FindCompareUser();
									SLOG_DEBUG << "heartbeat:compare2 id:" << nCompareID;
									if (nCompareID != 0 && nCompareID != m_nSelfPlayerID && IsValidPlayer(nCompareID))
									{
										nActionIdx = 22;
										int64_t lNeed = m_lCurJetton * NeedDouble() * 2;
										if (m_nSelfGold >= lNeed)
										{
											onSendCompareMsg(nCompareID);
										}
										else
										{
											onSendCompareExMsg();
										}

									}
									else
									{
										SLOG_WARNING << boost::format("[Robot] Nothing TODO line:%1%") % __LINE__;
									}
								}
							}
						}
					}
				}
				SLOG_WARNING<<boost::format("[Robot]:action:%1%,line:%2%")%nActionIdx%__LINE__;
				if(nActionIdx==-1)
				{ // 优先跟注
					if(CanFlow())
					{
						onSendFlowMsg();
					}
					else if(CanCheck())
					{
						onSendCheckMsg();
					}
					else
					{
						SLOG_DEBUG << "heartbeat:compare3";
						uint32_t nCompareID = FindCompareUser();
						SLOG_DEBUG << "heartbeat:compare3 id:" << nCompareID;
						if (nCompareID != 0 && nCompareID != m_nSelfPlayerID && IsValidPlayer(nCompareID))
						{
							nActionIdx = 22;
							int64_t lNeed = m_lCurJetton * NeedDouble() * 2;
							if (m_nSelfGold >= lNeed)
							{
								onSendCompareMsg(nCompareID);
							}
							else
							{
								onSendCompareExMsg();
							}
						}
						else
						{ // 弃牌
							onSendGiveUpMsg();
						}
						
					}
				}
				//////////////////////////////////////////////////////////////////////////
			}
		}
		else if(m_nGameStatus==eGameState_End)
		{
			// 
			m_nGameStatus = eGameState_Free;
			m_nUserStatus = eUserState_free;

			m_duration = global_random::instance().rand_double(m_nReadyTimeMin, m_nReadyTimeMax);

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

uint32_t zjh_space::robot_player::get_pid()
{

	return m_nSelfPlayerID;
}

int64_t zjh_space::robot_player::get_gold()
{

	return m_nSelfGold;
}

int32_t zjh_space::robot_player::get_seat(uint32_t playerid)
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

uint32_t zjh_space::robot_player::get_id(int32_t seat)
{
	uint32_t uid = 0;
	auto it = m_MapTableUser.find(seat);
	if(it!=m_MapTableUser.end())
	{
		return it->second.p_id;
	}

	return uid;
}


void zjh_space::robot_player::initScene()
{
	// 
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetc2l_get_scene_info, zjh_protocols::e_mst_c2l_get_scene_info);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
}


void zjh_space::robot_player::init(LPlayerPtr player,int rid,int tid)
{
	// deprecated
	assert(player);
	m_player = player;
	m_roomid = rid;
	m_tableid = tid;

	m_nSelfPlayerID = player->get_pid();
	m_nSelfGold = player->get_gold();
	m_init = true;

	// 读取房间配置
 	auto cfg = GoldFlower_RoomCFG::GetSingleton()->GetData(rid);
	m_nBaseGold = cfg->mBaseCondition;
	m_nLookCondition = cfg->mLookCondition;
	m_nPkCondition = cfg->mPkCondition;
	m_nMaxChip = cfg->mMaxChip;
	m_VecAddJetton = cfg->mAddChipList;
	// TODO 设定游戏中不加入
	m_nGameStatus = eGameState_Free;
}

void zjh_space::robot_player::init(uint32_t uid,int64_t gold,int32_t rid,int32_t tid,int32_t seat)
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
	auto cfg = GoldFlower_RoomCFG::GetSingleton()->GetData(rid);
	m_nBaseGold = cfg->mBaseCondition;
	m_nLookCondition = cfg->mLookCondition;
	m_nPkCondition = cfg->mPkCondition;
	m_nMaxChip = cfg->mMaxChip;
	m_VecAddJetton = cfg->mAddChipList;
	// 读取机器人操作配置
	auto game_cfg = GoldFlower_RobotGameCFG::GetSingleton()->GetData(rid);
	m_nOperaTimeMin = game_cfg->mOperaTimeMin;
	m_nOperaTimeMax = game_cfg->mOperaTimeMax;
	m_nReadyTimeMin = game_cfg->mReadyTimeMin;
	m_nReadyTimeMax = game_cfg->mReadyTimeMax;
	m_nResultTime = game_cfg->mRobotResultTime;

	m_nCheckParam1 = game_cfg->mParam1;
	m_nCheckParam2 = game_cfg->mParam2;
	m_nCheckParam3 = game_cfg->mParam3;
	m_nCheckParam4 = game_cfg->mParam4;
	// @读取机器人操作参数
	m_MapRobotAction.clear();
	for (int i = 1; i <= MAX_ACTION; i++)
	{
		auto gamae_action = GoldFlower_RobotActionCFG::GetSingleton()->GetData(i);
		if (gamae_action == nullptr) break;
		tagRobotAction action;
		action.nModelId = i;
		action.card = gamae_action->mModelData;
		action.fAddParam = gamae_action->mAddRateParam;
		action.vecAddRate = gamae_action->mAddRateList;
		action.fCmpParam1 = gamae_action->mCmpParam1;
		action.fCmpParam2 = gamae_action->mCmpParam2;
		action.fCmpParam3 = gamae_action->mCmpParam3;
		action.fAllInParam1 = gamae_action->mAllInParam1;
		action.fAllInParam2 = gamae_action->mAllInParam2;
		action.fGiveUpParam = gamae_action->mGiveUpParam;

		m_MapRobotAction.insert(std::make_pair(action.nModelId, action));
	}
	

}


void zjh_space::robot_player::initGame(int seat,uint32_t p_id,int32_t spare)
{
	if(p_id==m_nSelfPlayerID)
	{
		m_nSelfChairID = seat;
		// 准备
		if(spare == 0)
		{
			m_duration = global_random::instance().rand_double(m_nReadyTimeMin, m_nReadyTimeMax);
		}
		else
		{
			if(spare <= ROBOT_LESS)
			{
				m_duration = 0;
			}
			else
			{
				m_duration = global_random::instance().rand_double(ROBOT_LESS, spare);
			}
			
		}

		SLOG_DEBUG<<"initGame:"<<m_duration;

		m_nUserStatus = eUserState_free;
		m_nGameStatus = eGameState_Free;
	}
}


void zjh_space::robot_player::initGame_play(int seat,uint32_t p_id,int32_t status)
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


bool zjh_space::robot_player::onEventUserReady(uint32_t uid)
{
	if(uid==m_nSelfPlayerID)
	{
		m_nUserStatus = eUserState_ready;
	}

	return true;
}

bool zjh_space::robot_player::onEventGameStart(std::map<uint32_t,TableUser> &tableusr, 
											   int64_t usr_gold,int64_t base_gold,uint32_t bankerid,
											   int64_t lPool)
{
	m_MapTableUser.clear();
	m_nGameRound = 0;
	m_nCurOperatorID = 0;
	m_nFirstID = 0;
	m_fLetSee = 0;
	m_nShowHandID = 0;

	m_bHasAddAction = false;
	m_bHasShowHand = false;
	m_bHasAllin = false;
	m_bGiveUp = false;
	m_bCompareAction = false;
	

	m_AllUser.clear();
	m_CheckUser.clear();
	m_RobotUser.clear();
	m_CompareUser.clear();
	m_bHasAddAction = false;
	m_bChecking = false;
	m_bIsGod = false;
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
				tagPlayerCtrlAttr attr;
				bool bRet = player->get_player_ctrl(attr);
				int nTag = Tag_UserA;
				if(bRet)
				{
					nTag = attr.nTag;
				}
				usr.p_tag = nTag;
				usr.p_expend = base_gold;
				m_AllUser.push_back(uid);
				// Robot
				if(player->is_robot())
				{
					m_RobotUser.push_back(uid);
				}
				SLOG_DEBUG<<boost::format("robot_player:[playerid:%d,Tag:%d,Win:%f]")%uid%attr.nTag%attr.fwinPercent;
			}

		}
		else
		{ // 旁观等待

		}
	}

	// 设置参数
	m_nSelfGold = usr_gold;
	m_nBaseGold = base_gold;
	m_nExpend += base_gold;
	m_lCurJetton = m_nBaseGold;
	// @记录其他玩家牌信息
	m_MapSaveCard.clear();
	
	return true;
}

bool zjh_space::robot_player::onGetGameInfo(uint32_t uid, GameCard &card)
{
	/* @初始获取牌型 
	*/
	if (uid == m_nSelfPlayerID)
	{
		m_GameLogic.CheckCardModel(card);
		// @ 比对相应牌值
		for (int i = 1; i <= MAX_ACTION; i++)
		{
			auto rit = m_MapRobotAction.find(i);
			if (rit == m_MapRobotAction.end()) continue;
			auto actioncard = rit->second.card;
			GameCard gamecard;
			for (int i = 0; i < MAX_CARDS_HAND; i++)
			{
				gamecard.nCard[i] = actioncard[i];
			}
			std::sort(gamecard.nCard.begin(), gamecard.nCard.end(), [](int32_t a, int32_t b) {
				return (a&MASK_VALUE) > (b&MASK_VALUE); });
			m_GameLogic.CheckCardModel(gamecard);
			if (card < gamecard || card == gamecard)
			{
				m_RobotAction = rit->second;
				break;
			}
		}
		// @ check
		if (m_RobotAction.nModelId == 0)
		{
			//SLOG_WARNING << boost::format("[Warning]:Can't find action card :(%1%,%2%,%3%)") % card.nCard[0] % card.nCard[1] % card.nCard[2];

			m_RobotAction = m_MapRobotAction[1];
		}
		// @ killer
		if (card.bCheck == 110)
		{
			m_bIsGod = true;
		}
		else
		{
			m_bIsGod = false;
		}
	}
	m_MapSaveCard.insert(std::make_pair(uid, card));

	
	return true;
}

bool zjh_space::robot_player::onEventNoticeStart(uint32_t uid)
{
	m_nGameStatus = eGameState_Play;
	m_nCurOperatorID = uid;
	m_nFirstID = uid;
	m_nGameRound++;
	if(uid == m_nSelfPlayerID)
	{
		// 操作
		m_duration = global_random::instance().rand_double(m_nOperaTimeMin, m_nOperaTimeMax);
		SLOG_DEBUG<<"onEventNoticeStart:"<<m_duration;
	}
	else
	{
		m_duration = 0;
	}

	return true;
}



int64_t zjh_space::robot_player::GetRandJetton()
{
	int64_t llJet = 0;

	int iRandValue = global_random::instance().rand_100();
	if (iRandValue < 34)
	{
		for (uint32_t i=0;i<m_VecAddJetton.size();i++)
		{
			if(CanAddJetton(m_VecAddJetton[i]))
			{
				llJet = m_VecAddJetton[i];
				break;
			}
		}
	}
	else
	{
		for (int i=m_VecAddJetton.size()-1;i>=0;i--)
		{
			if(CanAddJetton(m_VecAddJetton[i]))
			{
				llJet = m_VecAddJetton[i];
				break;
			}
		}
	}

	return llJet;
}

bool zjh_space::robot_player::CanAddJetton(int64_t lJetton)
{
	SLOG_DEBUG << boost::format("CanAdd:,cur:%1%,max:%2%,double:%3%") \
		%m_lCurJetton%m_nMaxChip%NeedDouble();
	SLOG_DEBUG << boost::format("CanAdd: gold:%1%")%m_nSelfGold;
	if(m_nGameStatus!=eGameState_Play) return false;
	if(lJetton>m_nMaxChip) return false;
	else if (lJetton * NeedDouble() > m_nSelfGold) return false;
	else if (lJetton<=m_lCurJetton) return false;
	return true;
}

bool zjh_space::robot_player::CanCompare()
{
	if(m_nGameStatus!=eGameState_Play) return false;
	if (m_nGameRound>=m_nPkCondition)
	{
#if 0
		int64_t lNeed = m_lCurJetton * NeedDouble() * 2;
		if(m_nSelfGold >= lNeed)
		{
			return true;
		}
#else
		if(m_nSelfGold > 0)
		{
			return true;
		}
#endif
	}
	return false;
}

bool zjh_space::robot_player::IsCompare()
{
	auto it = std::find(m_CompareUser.begin(),m_CompareUser.end(),m_nSelfPlayerID);
	if(it != m_CompareUser.end())
		return true;
	return false;
}


bool zjh_space::robot_player::IsValidPlayer(uint32_t uid)
{
	auto& player = game_engine::instance().get_lobby().get_player(uid);
	if(player)
	{
		return true;
	}
	SLOG_WARNING<<boost::format("Invalid Player: %d")%uid;
	return false;
}

bool zjh_space::robot_player::IsCheck()
{
	auto it = std::find(m_CheckUser.begin(),m_CheckUser.end(),m_nSelfPlayerID);
	if(it != m_CheckUser.end())
		return true;
	return false;
}

bool zjh_space::robot_player::CanCheck()
{
	if(m_nGameStatus!=eGameState_Play) return false;
	if(IsCheck()) return false;
	if(m_nGameRound>=m_nLookCondition)
	{
		return true;
	}
	return false;
}

bool zjh_space::robot_player::CanFlow()
{
	if(m_nGameStatus!=eGameState_Play) return false;
	int64_t lNeed = m_lCurJetton * NeedDouble();
	if(m_nSelfGold >= lNeed)
	{
		return true;
	}
	return false;
}

bool zjh_space::robot_player::IsActive()
{
	auto it = std::find(m_AllUser.begin(),m_AllUser.end(),m_nSelfPlayerID);
	if(it != m_AllUser.end())
		return true;
	return false;
}

bool zjh_space::robot_player::CanGiveUp()
{
	if(m_nGameStatus!=eGameState_Play) return false;

	return true;
}

bool zjh_space::robot_player::CanShowHand()
{

	return true;
}

int zjh_space::robot_player::NeedDouble()
{
	auto it = std::find(m_CheckUser.begin(),m_CheckUser.end(),m_nSelfPlayerID);
	if(it != m_CheckUser.end())
	{
		return 2;
	}
	return 1;
}

void zjh_space::robot_player::onSendReadyMsg()
{
	m_duration = 0;
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetc2l_ask_ready, zjh_protocols::e_mst_c2l_ask_ready);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
}

void zjh_space::robot_player::onSendCheckMsg()
{
	m_duration = 0;
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetc2l_ask_operator_kan, zjh_protocols::e_mst_c2l_ask_operator_kan);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
	SLOG_DEBUG<<"onSendCheckMsg:";
}

void zjh_space::robot_player::onSendFlowMsg()
{
	m_duration = 0;
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetc2l_ask_operator_gen, zjh_protocols::e_mst_c2l_ask_operator_gen);
	int64_t lNeedGold = m_lCurJetton*NeedDouble();
	sendmsg->set_operator_gold(lNeedGold);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);

	SLOG_DEBUG<<"onSendFlowMsg:"<<lNeedGold;
}

void zjh_space::robot_player::onSendAddMsg(int64_t lAdd)
{
	m_duration = 0;
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetc2l_ask_operator_jia, zjh_protocols::e_mst_c2l_ask_operator_jia);
	int64_t lNeedGold = lAdd*NeedDouble();
	sendmsg->set_operator_gold(lAdd);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);

	SLOG_DEBUG<<"onSendAddMsg:"<<lAdd;
}

void zjh_space::robot_player::onSendShowHandMsg()
{
	m_duration = 0;
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetc2l_ask_operator_showhand, zjh_protocols::e_mst_c2l_ask_showhand);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
	SLOG_DEBUG<<"onSendShowHandMsg:";
}

void zjh_space::robot_player::onSendGiveUpMsg()
{
	m_duration = 0;
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetc2l_ask_operator_qi, zjh_protocols::e_mst_c2l_ask_operator_qi);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
	SLOG_DEBUG<<"onSendGiveUpMsg:";
}


void zjh_space::robot_player::onSendLetSeeMsg()
{
	m_fLetSee = 0;
	if(m_bGiveUp)
	{
		SLOG_DEBUG<<"onSendLetSeeMsg: Give up can't send LetSee Msg";
	}
	else
	{
		auto sendmsg = PACKET_CREATE(zjh_protocols::packetc2l_let_see, zjh_protocols::e_mst_c2l_ask_letsee);
		robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
		SLOG_DEBUG<<"onSendLetSeeMsg: send LetSee Msg";
	}
	
}


uint32_t zjh_space::robot_player::FindCompareUser()
{
	// 
	SLOG_WARNING<<"1=============";
	uint32_t nCompareid = 0;
	for (auto it : m_CompareUser)
	{
		SLOG_WARNING<<boost::format("compare:%d")%it;
	}
	for (auto it : m_CheckUser)
	{
		SLOG_WARNING<<boost::format("check:%d")%it;
	}
	for (auto it : m_AllUser)
	{
		SLOG_WARNING<<boost::format("all:%d")%it;
	}
	SLOG_WARNING<<"2=============";
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
	SLOG_WARNING<<"3=============";
	//
	return nCompareid;
}

uint32_t zjh_space::robot_player::FindNoSelf(VectorUserID &vecusers)
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

void zjh_space::robot_player::onSendCompareMsg(uint32_t playerid)
{
	m_duration = 0;
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetc2l_ask_operator_bi, zjh_protocols::e_mst_c2l_ask_operator_bi);
	sendmsg->set_compareid(playerid);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
	SLOG_DEBUG<<"onSendCompareMsg:"<<playerid;
}


void zjh_space::robot_player::onSendCompareExMsg()
{
	m_duration = 0;
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetc2l_ask_operator_pk, zjh_protocols::e_mst_c2l_ask_operator_pk);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
	SLOG_DEBUG<<"onSendCompareExMsg:";
}

void zjh_space::robot_player::onSendAllInMsg()
{
	m_duration = 0;
	auto sendmsg = PACKET_CREATE(zjh_protocols::packetc2l_ask_operator_allin, zjh_protocols::e_mst_c2l_ask_operator_allin);
	robot_mgr::instance().send_packet(get_pid(), sendmsg->packet_id(), sendmsg);
	SLOG_DEBUG<<"onSendAllInMsg:";
}

bool zjh_space::robot_player::onEventFlow(uint32_t uid,uint32_t nextid,
										  int64_t lNeedGold,
										  int32_t nRound,
										  int64_t lPool)
{
	// ok
	m_bHasShowHand = false;
	m_bHasAllin = false;
	m_bCompareAction = false;
	m_lPoolGold = lPool;
	if(nextid!=0) m_nCurOperatorID = nextid;
	if (nRound > m_nGameRound)
	{
		m_bHasAddAction = false;
	}
	m_nGameRound = nRound;
	if(m_nSelfPlayerID==uid)
	{
		m_nSelfGold -= lNeedGold;
		m_nExpend += lNeedGold;
	}
	AddUserExpend(uid,lNeedGold);
	if(m_nSelfPlayerID==nextid)
	{
		if (m_bChecking  && m_duration > 0)
		{
			SLOG_DEBUG << "onEventFlow: waiting to check: " << m_duration;
		}
		else
		{
			double fTime = global_random::instance().rand_double(m_nOperaTimeMin, m_nOperaTimeMax);
			m_duration = fTime + GetFloatTime(fTime);
			SLOG_DEBUG << "onEventFlow:" << m_duration;
		}
		
	}
	else
	{
		m_duration = 0;
		
		// 非活动期间看牌
		m_duration = GetCheckTime();
		SLOG_DEBUG<<"onEventFlow--check :"<<m_duration;
	}

	return true;
}


bool zjh_space::robot_player::onEventCheck(uint32_t uid)
{
	// ok
// 	m_bHasAddAction = false;
// 	m_bHasShowHand = false;
// 	m_bHasAllin = false;

	m_CheckUser.push_back(uid);
	if(m_nSelfPlayerID==uid && uid==m_nCurOperatorID)
	{ // 自己才有接下来的操作

		double fTime = global_random::instance().rand_double(m_nOperaTimeMin, m_nOperaTimeMax);
		m_duration = fTime + GetFloatTime(fTime); 
		SLOG_DEBUG<<boost::format("onEventCheck A:%1%")%m_duration;
	}
	else if(uid==m_nSelfPlayerID && uid!=m_nCurOperatorID)
	{
		// ADD  Halen 2017-8-23
		//  非活动看牌-弃牌
		if( !IsCompare())
		{
			double fTime = global_random::instance().rand_double(m_nOperaTimeMin, m_nOperaTimeMax);
			m_duration = fTime + GetFloatTime(fTime); 
			SLOG_DEBUG<<boost::format("onEventCheck B:%1%")%m_duration;
		}
	}
	else
	{
	
	}
	
	return false;
}

bool zjh_space::robot_player::onEventCheck(uint32_t uid, CARDLEVEL clvl)
{
	// ok
// 	m_bHasAddAction = false;
// 	m_bHasShowHand = false;
// 	m_bHasAllin = false;

	m_CheckUser.push_back(uid);
	if (m_nSelfPlayerID == uid && uid == m_nCurOperatorID)
	{ // 自己才有接下来的操作

		double fTime = global_random::instance().rand_double(m_nOperaTimeMin, m_nOperaTimeMax);
		m_duration = fTime + GetFloatTime(fTime);
		SLOG_DEBUG << boost::format("onEventCheck A:%1%") % m_duration;
		
	}
	else if (uid == m_nSelfPlayerID && uid != m_nCurOperatorID)
	{
		// ADD  Halen 2017-8-23
		//  非活动看牌-弃牌
		if (!IsCompare())
		{
			double fTime = global_random::instance().rand_double(m_nOperaTimeMin, m_nOperaTimeMax);
			m_duration = fTime + GetFloatTime(fTime);
			SLOG_DEBUG << boost::format("onEventCheck B:%1%") % m_duration;
		}
	}
	else
	{

	}

	return false;
}

bool zjh_space::robot_player::onEventGiveUP(uint32_t uid,uint32_t nextid,int32_t nRound)
{
	// ok
	m_bHasShowHand = false;
	m_bHasAllin = false;
	m_bCompareAction = false;
	SetUserActive(uid,FALSE);
	//
	if(uid==m_nSelfPlayerID)
	{
		// 已经弃牌
		m_duration = 0;

		m_bGiveUp = true;
	}
	if(nextid!=0) m_nCurOperatorID = nextid;
	if (nRound > m_nGameRound)
	{
		m_bHasAddAction = false;
	}
	m_nGameRound = nRound;
	if(nextid == m_nSelfPlayerID)
	{
		if (m_bChecking  && m_duration > 0 )
		{
			SLOG_DEBUG << "onEventGiveUP:waiting to check: " << m_duration;
		}
		else
		{
			double fTime = global_random::instance().rand_double(m_nOperaTimeMin, m_nOperaTimeMax);
			m_duration = fTime + GetFloatTime(fTime);
			SLOG_DEBUG << "onEventGiveUP:" << m_duration;
		}
		
	}
	else if(m_nCurOperatorID==m_nSelfPlayerID)
	{
		//
	}
	else
	{
		m_duration = 0;

		// 非活动期间看牌
		m_duration = GetCheckTime();
		SLOG_DEBUG<<"onEventGiveUP--check :"<<m_duration;

	}

	return true;
}

bool zjh_space::robot_player::onEventAdd(uint32_t uid,uint32_t nextid,
										 int64_t lCurGold,int64_t lNeedGold,
										 int32_t nRound,
										 int64_t lPool)
{
	// ok
	m_bHasShowHand = false;
	m_bHasAllin = false;
	m_bCompareAction = false;
	m_lPoolGold = lPool;
	m_lCurJetton = lCurGold;
	if(nextid!=0) m_nCurOperatorID = nextid;
	if (nRound > m_nGameRound)
	{
		m_bHasAddAction = false;
	}
	m_nGameRound = nRound;
	if(m_nSelfPlayerID==uid)
	{
		 m_nSelfGold -= lNeedGold;
		 m_nExpend += lNeedGold;

		 // ADD 2017-9-4 
		 m_pairRound.first = nRound;
		 m_pairRound.second = 3;
	}
	// 每一轮机器人只加注一次
	auto itfind = std::find(m_RobotUser.begin(),m_RobotUser.end(),uid);
	if (itfind != m_RobotUser.end())
	{
		m_bHasAddAction = true;
	}
	AddUserExpend(uid,lNeedGold);

	if(m_nSelfPlayerID==nextid)
	{
		if (m_bChecking && m_duration > 0)
		{
			SLOG_DEBUG << "onEventAdd:waiting to check: " << m_duration;
		}
		else
		{
			double fTime = global_random::instance().rand_double(m_nOperaTimeMin, m_nOperaTimeMax);
			m_duration = fTime + GetFloatTime(fTime);
			SLOG_DEBUG << "onEventAdd:" << m_duration;
		}

	}
	else
	{
		m_duration = 0;

		// 非活动期间看牌
		m_duration = GetCheckTime();
		SLOG_DEBUG<<"onEventAdd--check :"<<m_duration;
	}
	return true;
}

bool zjh_space::robot_player::onEventCompare(uint32_t reqid,int64_t lNeedGold, 
											 uint32_t resid,uint32_t winid, 
											 uint32_t nextid, 
											 int32_t nRound,int64_t lPool)
{
	//  ok
	m_bHasShowHand = false;
	m_bHasAllin = false;
	m_bCompareAction = true;
	m_lPoolGold = lPool;
	if(nextid!=0) m_nCurOperatorID = nextid;
	if (nRound > m_nGameRound)
	{
		m_bHasAddAction = false;
	}
	m_nGameRound = nRound;
	if(m_nSelfPlayerID==reqid)
	{
		m_nSelfGold -= lNeedGold;
		m_nExpend += lNeedGold;
	}
	AddUserExpend(reqid,lNeedGold);
	// Fix-Bug : H 
 	//m_CompareUser.push_back(winid);
	AddComare(winid);

	if(winid==resid)
	{
		SetUserActive(reqid,FALSE);
	}
	else
	{
		SetUserActive(resid,FALSE);
	}
	if(m_nSelfPlayerID==nextid)
	{
		double fAction = 0;
		if(m_bCompareAction)
		{
			fAction = Compare_Action;
		}
		double fTime = global_random::instance().rand_double(m_nOperaTimeMin+fAction, m_nOperaTimeMax+fAction);
		m_duration = fTime + GetFloatTime(fTime) ; 
		SLOG_DEBUG<<"onEventCompare:"<<m_duration;
	}
	else
	{
		m_duration = 0;

		// Del 10/2/2017 比牌期间不允许看牌
// 		// 非活动期间看牌
// 		m_duration = GetCheckTime();
// 		SLOG_DEBUG<<"onEventCompare--check :"<<m_duration;
	}


	return true;
}

bool zjh_space::robot_player::onEventNoticeAllIn(uint32_t uid)
{
	/* 
	* 孤注一掷
	* 服务器通知孤注一掷：操作二选一（孤注一掷、看牌\弃牌）
	*/
	m_bHasAddAction = false;
	m_bHasShowHand = false;
	m_bHasAllin = false;

	if(uid == m_nSelfPlayerID)
	{
		m_nCurOperatorID = uid;
		// 必须孤注一掷
		m_bHasAllin =  true;
		double fAction = 0;
		if(m_bCompareAction)
		{
			fAction = Compare_Action;
		}
		double fTime = global_random::instance().rand_double(m_nOperaTimeMin+fAction, m_nOperaTimeMax+fAction);
		m_duration = fTime + GetFloatTime(fTime); 
		SLOG_DEBUG<<"onEventNoticeAllIn:"<<m_duration;
	}
	else
	{
		m_duration = 0;
		// TODO
		// 非活动期间看牌
	}

	return true;
}

bool zjh_space::robot_player::onEventAllIn(uint32_t uid,bool bWin,int64_t lNeedGold,int64_t lPool,uint32_t nextid,int32_t nRound)
{
	/* 
	* 孤注一掷-结果
	*/
	m_bHasAddAction = false;
	m_bHasShowHand = false;
	m_bHasAllin = false;
	m_bCompareAction = true;
	m_lPoolGold = lPool;
	if(nRound>0) m_nGameRound = nRound;
	SLOG_CRITICAL<<boost::format("onEventAllIn =======> uid:%d,nextid:%d")%uid%nextid;
	if(bWin)
	{
		// game over
	}
	else
	{
		if(m_nSelfPlayerID==uid)
		{
			m_nSelfGold -= lNeedGold;
			m_nExpend += lNeedGold;
		}

		AddUserExpend(uid,lNeedGold);
	//	m_CompareUser.push_back(uid);
		AddComare(uid);
		SetUserActive(uid,FALSE);

		if(nextid!=0)
		{
			m_nCurOperatorID = nextid;
			// 游戏继续
			if(m_nSelfPlayerID==nextid)
			{
				double fAction = 0;
				if(m_bCompareAction)
				{
					fAction = Compare_Action;
				}
				double fTime = global_random::instance().rand_double(m_nOperaTimeMin+fAction, m_nOperaTimeMax+fAction);
				m_duration = fTime + GetFloatTime(fTime); 
				SLOG_DEBUG<<"onEventAllIn:"<<m_duration;
			}
			else
			{
				m_duration = 0;
				if(m_nSelfPlayerID != uid)
				{
					// 非活动期间看牌
					m_duration = GetCheckTime();
					SLOG_DEBUG<<"onEventAllIn--check :"<<m_duration;
				}
			}
		}
	}
	
	return true;
}

bool zjh_space::robot_player::onEventShowHand(uint32_t uid,uint32_t nextid,int64_t lNeedGold,int64_t lPool)
{
	/* 
	*  梭哈
	*  只能是梭哈 或者 弃(看)
	*/
	m_bHasAddAction = false;
	m_bHasShowHand = false;
	m_bHasAllin = false;
	m_bCompareAction = false;
	m_lPoolGold = lPool;
	if(nextid!=0) m_nCurOperatorID = nextid;
	if(m_nSelfPlayerID==uid)
	{
		m_nSelfGold -= lNeedGold;
		m_nExpend += lNeedGold;
	}
	AddUserExpend(uid,lNeedGold);
	// @记录主动梭哈
	m_nShowHandID = uid;

	m_nCurOperatorID = nextid;
	if(nextid == m_nSelfPlayerID)
	{
		m_bHasShowHand = true;
		double fTime = global_random::instance().rand_double(m_nOperaTimeMin, m_nOperaTimeMax);
		m_duration = fTime + GetFloatTime(fTime); 
		SLOG_DEBUG<<"onEventShowHand:"<<m_duration;
	}
	else
	{
		m_duration = 0;
		// TODO
		// 非活动期间看牌
	}

	return true;
}

bool zjh_space::robot_player::onEventEnd(uint32_t playerid,int32_t nEndType)
{
	// ok
	m_nGameStatus = eGameState_End;
//	m_nUserStatus = eUserState_free;
	m_duration = 0;
	m_fLetSee = 0;
	if(nEndType==eGameEndType_OnlyOne)
	{
		m_duration = global_random::instance().rand_double(TIME_COMPARE,TIME_COMPARE+TIME_LESS);

		m_fLetSee = global_random::instance().rand_double(ROBOT_ACTION,TIME_COMPARE);
	}
	else
	{
		m_duration = global_random::instance().rand_double(m_nResultTime, m_nResultTime+TIME_LESS*2);
		// m_nResultTime = 9
		m_fLetSee = global_random::instance().rand_double(TIME_COMPARE+TIME_LESS,TIME_COMPARE+TIME_COMPARE);
	}

	SLOG_DEBUG<<boost::format("[onEventEnd] duration:%1%:nEndType:%2%,fLetSee:%3%")%m_duration%nEndType%m_fLetSee;
	// 清理

	return true;
}

//////////////////////////////////////////////////////////////////////////

int32_t zjh_space::robot_player::GetLabelByValue(int64_t nValue)
{
	return Tag_UserC;
}

void zjh_space::robot_player::CalcUserLabel()
{
	for (auto it = m_MapTableUser.begin();it!=m_MapTableUser.end();it++)
	{
		auto & user = it->second;
		if(user.p_active==FALSE) continue;
		uint32_t uid = user.p_id;
		auto& player = game_engine::instance().get_lobby().get_player(uid);
		if(player)
		{
			tagPlayerCtrlAttr attr;
			player->get_player_ctrl(attr,m_lPoolGold,user.p_expend);
			int32_t nTag= attr.nTag;
			user.p_tag = nTag;
		//	SLOG_DEBUG<<boost::format("UserLabel:[playerid:%d,Tag:%d,Win:%f]")%uid%attr.nTag%attr.fwinPercent;
		}
	}
}

int32_t zjh_space::robot_player::GetMaxLabel()
{
	CalcUserLabel();
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

void zjh_space::robot_player::SetUserActive(uint32_t playerid,BOOL bActive/*=FALSE*/)
{
	auto it = m_MapTableUser.find(playerid);
	if(it!=m_MapTableUser.end())
	{
		auto& user = it->second;
		user.p_active = bActive;
		if(bActive==FALSE)
		{// del
			DeleteGiveUpUser(playerid);
		}
	}

}

void zjh_space::robot_player::DeleteGiveUpUser(uint32_t playerid)
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

void zjh_space::robot_player::AddUserExpend(uint32_t playerid,int64_t lGold)
{
	auto it = m_MapTableUser.find(playerid);
	if(it!=m_MapTableUser.end())
	{
		auto& user = it->second;
		user.p_expend += lGold;
	}
}


void zjh_space::robot_player::AddComare(uint32_t playerid)
{
	auto compare_it = std::find(m_CompareUser.begin(),m_CompareUser.end(),playerid);
	if(compare_it == m_CompareUser.end())
	{
		m_CompareUser.push_back(playerid);
	}
}

double zjh_space::robot_player::GetShowHandRate2Active()
{
	int32_t nLabel = GetMaxLabel();
	double nRate = 0;
	if(m_AllUser.size()>2) return nRate;
	if(m_lCurJetton!=m_nMaxChip) return nRate;
	double nRound = m_nGameRound;
	double fParam1 = m_RobotAction.fAllInParam1;
	double fParam2 = m_RobotAction.fAllInParam2;
	if (fParam2 == 0)
	{
		return nRate;
	}
	nRate = fParam1 + nRound / fParam2;

	return nRate;
}


double zjh_space::robot_player::GetShowHandRate2Passive()
{
	int32_t nLabel = GetMaxLabel();
	double nRate = 0;
	if(m_AllUser.size()>2) return nRate;
	if(m_lCurJetton!=m_nMaxChip) return nRate;
	double nRound = m_nGameRound;
	/*
	1、对方已看牌，且牌比自己大
	2、对方未看牌，且牌比自己大
	3、对方牌比自己小
	*/
	if (m_nShowHandID == 0) return nRate;
	GameCard selfcard , othercard;
	auto it1 = m_MapSaveCard.find(m_nSelfPlayerID);
	if (it1 == m_MapSaveCard.end())
	{
		return nRate;
	}
	else
	{
		selfcard = it1->second;
	}
	m_GameLogic.CheckCardModel(selfcard);
	auto it2 = m_MapSaveCard.find(m_nShowHandID);
	if (it2 == m_MapSaveCard.end())
	{
		return nRate;
	}
	else
	{
		othercard = it2->second;
	}
	m_GameLogic.CheckCardModel(othercard);
	// @梭哈者是否看牌
	bool bCheck = false;
	auto it = std::find(m_CheckUser.begin(), m_CheckUser.end(), m_nShowHandID);
	if (it != m_CheckUser.end())  bCheck = true;
	// @梭哈双方都是机器人: 直接梭哈
	bool bRobot = false;
	auto rb_it = std::find(m_RobotUser.begin(), m_RobotUser.end(), m_nShowHandID);
	if (rb_it != m_RobotUser.end())  bRobot = true;
	// @机器人有比牌操作
	bool bCompare = false;
	auto compare_it = std::find(m_CompareUser.begin(), m_CompareUser.end(), m_nShowHandID);
	if (compare_it != m_CompareUser.end()) bCompare = true;
	if (bRobot && bCompare)
	{
		return 1.0;
	}
	if (selfcard<othercard && bCheck)
	{
		nRate = 0.5 * m_nExpend / (m_nSelfGold + m_nExpend*1.0);
	}
	else if (selfcard<othercard && bCheck == false)
	{
		nRate = 0.5 * m_nExpend / (m_nSelfGold + m_nExpend*1.0);
	}
	else
	{
		nRate = 1.0;
	}

	return nRate;
}

double zjh_space::robot_player::GetGiveUpRate()
{
	int32_t nLabel = GetMaxLabel();
	double nRound = m_nGameRound;
	double nRate = 0;
// 	// 参与比牌  
// 	auto result = std::find(m_CompareUser.begin(),m_CompareUser.end(),m_nSelfPlayerID);
// 	if(result!=m_CompareUser.end())
// 	{
// 		return nRate;
// 	}
// 	// ADD 2017-9-4 
// 	if(std::abs(m_pairRound.first-m_nGameRound) <= m_pairRound.second)
// 	{
// 		return nRate;
// 	}
// 	// 未参与比牌 
// 	uint32_t nRobotCount = m_RobotUser.size();
// 	SLOG_DEBUG<<boost::format("[GetGiveUpRate]RobotCount:%d,MaxLabel:%d")%nRobotCount%nLabel;

	// @机器人弃牌
	//if (m_bIsGod) return nRate;

	nRate = m_RobotAction.fGiveUpParam;
	if (nRate == 0 && m_bIsGod == false)
	{
		nRate = 1.0;
	}
	return nRate;
}

double zjh_space::robot_player::GetAddRate()
{
	int32_t nLabel = GetMaxLabel();
	double nRound = m_nGameRound;
	double nRate = 0;
	uint32_t nRobotCount = m_RobotUser.size();
	//SLOG_DEBUG<<boost::format("[GetAddRate]RobotCount:%d,MaxLabel:%d")%nRobotCount%nLabel;
	if(nRobotCount==0) return nRate;
	if(m_lCurJetton==m_nMaxChip) return nRate;
	if (m_bHasAddAction) return nRate;
	double nRandom = global_random::instance().rand_01();
// 	
// 	if (m_RobotAction.fGiveUpParam == 1.0)
// 	{
// 		return nRate;
// 	}
// 	else
	{
		nRate = m_RobotAction.fAddParam;
	}

	return nRate;
}

double zjh_space::robot_player::GetCompareRate()
{
	int32_t nLabel = GetMaxLabel();
	double nRound = m_nGameRound;
	double nRate = 0;
	uint32_t nRobotCount = m_RobotUser.size();
	SLOG_DEBUG<<boost::format("[GetCompareRate]RobotCount:%d,MaxLabel:%d")%nRobotCount%nLabel;
	if(nRobotCount==0) return nRate;
	if( !CanCompare()) return nRate;
	// @ 新增未看牌不比牌
	if (!IsCheck()) return nRate;
	uint32_t nCheckCount = m_CheckUser.size();
	uint32_t nRobotCheckCount = 0;
	for (auto id : m_CheckUser)
	{
		for (auto id2 : m_RobotUser)
		{
			if (id == id2)
			{
				nRobotCheckCount++;
			}
		}
	}
	if (nRobotCheckCount >= nCheckCount)
	{
		nRobotCheckCount = nCheckCount;
	}
	double fParam1 = m_RobotAction.fCmpParam1;
	double fParam2 = m_RobotAction.fCmpParam2;
	double fParam3 = m_RobotAction.fCmpParam3;
	if (m_RobotAction.fGiveUpParam == 1.0)
	{
		return nRate;
	}
	else
	{
		if (fParam2 <= 0)
		{
			nRate = 0;
		}
		else
		{
			nRate = (nRound - 1) * fParam1 * nRobotCount / fParam2 + (nCheckCount- nRobotCheckCount) * fParam3;
		}
	}
	
	return nRate;
}

double zjh_space::robot_player::GetFlowRate()
{
	int32_t nLabel = GetMaxLabel();
	double nRound = m_nGameRound;
	double nRate = 0;
	uint32_t nRobotCount = m_RobotUser.size();
	if(nRobotCount==0) return nRate;
	
	if(nRate<=0) nRate = 0;
	if(nRate>=100) nRate = 100;
	return nRate;
}

double zjh_space::robot_player::GetCheckTime()
{
	double duration = 0;
	double nRandom = global_random::instance().rand_01();
	double nRand_Check = GetCheckRate();
// 	char buf[MAX_PATH] = {0};
// 	sprintf(buf,"[Un-Active-Period]Rate: random[%f],check[%f]",nRandom,nRand_Check);
// 	SLOG_DEBUG<<buf;
	SLOG_DEBUG << boost::format("[U-Active-Period]Random:%1%,nCheckRand:%2%") % nRandom%nRand_Check;
	if(nRandom<=nRand_Check)
	{
		m_bChecking = true;
		duration = global_random::instance().rand_double(m_nOperaTimeMin, m_nReadyTimeMax);
	}
	else
	{
		m_bChecking = false;
		duration = 0;
	}

	return duration;
}

double zjh_space::robot_player::GetCheckRate()
{
	double nRate = 0;
	if(!CanCheck()) return nRate;
	auto it = std::find(m_AllUser.begin(),m_AllUser.end(),m_nSelfPlayerID);
	if(it==m_AllUser.end()) return nRate;
	
	double nRound = m_nGameRound;
	double nCurChip = m_lCurJetton;
	double	nMaxChip = m_nMaxChip;

	double naddRate = 0;
	uint32_t nCheckCount = m_CheckUser.size();
	uint32_t nAllCount = m_AllUser.size();
	if (nAllCount <= 1) return nRate;
	char buf[MAX_PATH] = {0};
	sprintf(buf,"[GetCheckRate] Rate: PlayerCount[%d], Round[%f],chip[%f],maxchip[%f],checkcount[%d]",
		nAllCount,nRound,nCurChip,nMaxChip,nCheckCount);
	SLOG_DEBUG<<buf;

	// @看牌概率计算
	nRate = m_nCheckParam1 + m_nCheckParam2*nCheckCount / (nAllCount - 1) +
		m_nCheckParam3*(nCurChip / nMaxChip - 0.1) + m_nCheckParam4 / nRound;
	SLOG_DEBUG<<"GetCheckRate Rate: "<<nRate;
	return nRate;
}

int64_t zjh_space::robot_player::GetAddGold()
{
	int64_t nAddGold = 0;
	//
	m_VecAddRate = m_RobotAction.vecAddRate;
	double fRandom = global_random::instance().rand_double(1,100);
	double fSum = 0;
	for (uint32_t i=0;i<m_VecAddRate.size();i++)
	{
		fSum += m_VecAddRate[i];
		if(fRandom<=fSum)
		{
			nAddGold = m_VecAddJetton[i];
			break;
		}
	}
	SLOG_DEBUG<<"GetAddGold add1: "<<nAddGold;
	if(CanAddJetton(nAddGold)==false)
	{
		nAddGold = 0;
		// Add 10/1/2017
		for (uint32_t i = 0; i < m_VecAddJetton.size(); i++)
		{
			int64_t lJetton = m_VecAddJetton[i];
			if (CanAddJetton(lJetton))
			{
				nAddGold = lJetton;
				break;
			}
		}
	}
	SLOG_DEBUG<<"GetAddGold add2: "<<nAddGold;
	return nAddGold;
}

double zjh_space::robot_player::GetFloatTime(double fTime)
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
	fResult = 0;
	return fResult;
}
