#pragma once
#include "robot_def.h"
#include "logic_def.h"
#include <i_game_phandler.h>
#include "i_game_player.h"
#include "GameLogic.h"


ROBCOWS_SPACE_BEGIN

typedef struct tagTableUser
{
	int32_t		p_state;	// 状态
	int32_t		p_active;
	int32_t		p_idx;		// 椅子
	uint32_t	p_id;		
	int32_t		p_label;	// 标签
	int32_t		p_tag;		// 标记
	int64_t		p_expend;	// 当局消耗
}TableUser;

class robot_player
	: public enable_obj_pool<robot_player>
{
public:
	robot_player(void);
	virtual ~robot_player(void);

	void heartbeat(double elapsed);

	void init(LPlayerPtr player,int rid,int tid);

	void init(uint32_t uid,int64_t gold,int32_t rid,int32_t tid,int32_t seat);

	uint32_t get_pid();

	int64_t get_gold();

	int32_t	get_seat(uint32_t playerid);

	uint32_t get_id(int32_t seat);
public:
	// 
	void initScene();
	// 
	void initGame(int seat,uint32_t p_id,int32_t spare);
	//
	void initGame_play(int seat,uint32_t p_id,int32_t status);
	//
	bool onEventUserReady(uint32_t uid);
	//
	bool onEventGameStart(std::map<uint32_t,TableUser> &tableusr,
		int64_t usr_gold,int64_t base_gold,uint32_t bankerid,int64_t lPool);
	//
	bool onEventNoticeStart(uint32_t uid);
	//
	bool onEventFlow(uint32_t uid,uint32_t nextid,int64_t lNeedGold,
		int32_t nRound,int64_t lPool);
	//
	bool onEventGiveUP(uint32_t uid,uint32_t nextid,int32_t nRound);
	//
	bool onEventAdd(uint32_t uid,uint32_t nextid,int64_t lCurGold,int64_t lNeedGold,int32_t nRound,int64_t lPool);
	//
	bool onEventCompare(uint32_t reqid,int64_t lNeedGold,
		uint32_t resid,uint32_t winid,
		uint32_t nextid,
		int32_t nRound,int64_t lPool);
	//
	bool onEventShowHand(uint32_t uid,uint32_t nextid,int64_t lNeedGold,int64_t lPool);
	//
	bool onEventCheck(uint32_t uid,CARDLEVEL clvl);
	//
	bool onEventCheck(uint32_t uid);
	//
	bool onEventNoticeAllIn(uint32_t uid);
	//
	bool onEventAllIn(uint32_t uid,bool bWin,int64_t lNeedGold,int64_t lPool,uint32_t nextid,int32_t nRound);
	//
	bool onEventEnd(uint32_t playerid);

	void onSendReadyMsg();
	void onSendCheckMsg();
	void onSendFlowMsg();
	void onSendAddMsg(int64_t lAdd);
	void onSendShowHandMsg();
	void onSendGiveUpMsg();
	uint32_t FindCompareUser();
	uint32_t FindNoSelf(VectorUserID &vecusers);
	void onSendCompareMsg(uint32_t playerid);
	void onSendAllInMsg();


	bool CanAddJetton(int64_t lJetton);

	bool CanCompare();

	bool CanCheck();
	bool IsCheck();

	bool CanFlow();

	bool CanGiveUp();

	bool CanShowHand();

	int NeedDouble();
public:
	void onTellSpare();
	void onTellStart(std::map<uint32_t,TableUser> &tableusr);
	void onTellRobBanker(std::map<uint32_t, TableUser> &tableusr);
	void onTellBetRate(uint32_t nBanker,int32_t nRobRate);
	void onTellOpenCard();
	int32_t GetRobBankerRate();
	int32_t GetMaxRobRate();
	void onSendRobBankerMsg();
	int32_t GetBetRate();
	int32_t GetMaxBetRate();
	void onSendBetMsg(); 
	void onSendOpenCardMsg(); 

	void onTellGameResult();

protected:
	//////////////////////////////////////////////////////////////////////////
	void CalcUserLabel();
	int32_t GetLabelByValue(int64_t nValue);
	void SetUserActive(uint32_t playerid,BOOL bActive=FALSE);
	void DeleteGiveUpUser(uint32_t playerid);
	void AddUserExpend(uint32_t playerid,int64_t lGold);
	int32_t GetMaxLabel();
	int32_t GetPlayerLabel(uint32_t playerid);

	int64_t GetAddGold();

	double GetCheckTime();

	double GetFloatTime(double fTime);
private:

	int					m_nReadyTime;
	int					m_nBankerTime;
	int					m_nBetTime;
	int					m_nOperaTime;
	int					m_nResultTime;

	int64_t				m_nBaseGold;
	int32_t				m_nLookCondition;
	int32_t				m_nPkCondition;

	int64_t				m_nMaxChip;		// 最大注

	std::vector<int>	m_VecAddJetton;
	std::vector<int>	m_VecAddRate;

	std::vector<int>	m_VecRandRob;
	std::vector<int>	m_VecRandBet;

	std::vector<int>	m_VecRobRate;
	std::vector<int>	m_VecBetRate;

	std::map<uint32_t,TableUser> m_MapTableUser;

	VectorUserID			m_AllUser;
	VectorUserID			m_CheckUser;	// 看牌
	VectorUserID			m_RobotUser;	// 机器人
	VectorUserID			m_CompareUser;	// 比牌赢

	int64_t					m_lPoolGold;	// 总池

	uint32_t				m_nBankerID;

	int32_t					m_nBankerRate;
protected:
	bool  m_init;
	double m_duration;

private:
	LPlayerPtr	m_player;
	int32_t		m_roomid;
	int32_t		m_tableid;
	int32_t		m_nSelfChairID;
	uint32_t	m_nSelfPlayerID;
	int64_t		m_nSelfGold;
protected:
	int8_t		m_nGameStatus;
	int8_t		m_nUserStatus;

	int64_t		m_lCurJetton;		// 当前筹码值

	int32_t		m_nGameRound;		// 当前轮数

	uint32_t	m_nCurOperatorID;	// 当前操作用户
	uint32_t	m_nFirstID;

};

ROBCOWS_SPACE_END
