#pragma once
#include "robot_def.h"
#include "logic_def.h"
#include <i_game_phandler.h>
#include "i_game_player.h"
#include "GameLogic.h"


ZJH_SPACE_BEGIN

#define MAX_ACTION	20
typedef struct tagTableUser
{
	int32_t		p_state;	// 状态
	int32_t		p_active;
	int32_t		p_idx;		// 椅子
	uint32_t	p_id;		
	int32_t		p_label;	// 标签
	int32_t		p_tag;		// 标记
	int64_t		p_expend;	// 当局消耗

	std::array<int32_t, MAX_CARDS_HAND> nCard;

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
	bool onGetGameInfo(uint32_t uid, GameCard &clvl);
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
	bool onEventEnd(uint32_t playerid,int32_t nEndType);

	void onSendReadyMsg();
	void onSendCheckMsg();
	void onSendFlowMsg();
	void onSendAddMsg(int64_t lAdd);
	void onSendShowHandMsg();
	void onSendGiveUpMsg();
	void onSendLetSeeMsg();
	uint32_t FindCompareUser();
	uint32_t FindNoSelf(VectorUserID &vecusers);
	void onSendCompareMsg(uint32_t playerid);
	void onSendAllInMsg();
	void onSendCompareExMsg();


	int64_t GetRandJetton();

	bool CanAddJetton(int64_t lJetton);

	bool IsCompare();
	bool CanCompare();
	bool IsValidPlayer(uint32_t uid);

	bool CanCheck();
	bool IsCheck();

	bool CanFlow();

	bool CanGiveUp();
	bool IsActive();

	bool CanShowHand();

	int NeedDouble();

protected:
	//////////////////////////////////////////////////////////////////////////
	void CalcUserLabel();
	int32_t GetLabelByValue(int64_t nValue);
	void SetUserActive(uint32_t playerid,BOOL bActive=FALSE);
	void DeleteGiveUpUser(uint32_t playerid);
	void AddUserExpend(uint32_t playerid,int64_t lGold);
	void AddComare(uint32_t playerid);
	int32_t GetMaxLabel();

	double GetShowHandRate2Active();

	double GetShowHandRate2Passive();

	double GetGiveUpRate();

	double GetAddRate();

	int64_t GetAddGold();

	double GetCompareRate();

	double GetFlowRate();

	double GetCheckRate();

	double GetCheckTime();

	double GetFloatTime(double fTime);
private:
	CGameLogic			m_GameLogic;

	double				m_nReadyTimeMin;
	double				m_nReadyTimeMax;
	double				m_nOperaTimeMin;
	double				m_nOperaTimeMax;
	double				m_nResultTime;

	double				m_nCheckParam1;
	double				m_nCheckParam2;
	double				m_nCheckParam3;
	double				m_nCheckParam4;

	struct tagRobotAction
	{
		tagRobotAction()
		{
			nModelId = 1;
			strModeName = "";
			card.clear();
			fAddParam = 0.0;
			vecAddRate.clear();

			fCmpParam1 = 0.0;
			fCmpParam2 = 0.0;

			fAllInParam1 = 0.0;
			fAllInParam2 = 0.0;
			fGiveUpParam = 0.0;
		}
		tagRobotAction(const tagRobotAction& action)
		{
			nModelId = action.nModelId;
			strModeName = action.strModeName;
			card = action.card;
			fAddParam = action.fAddParam;
			vecAddRate = action.vecAddRate;

			fCmpParam1 = action.fCmpParam1;
			fCmpParam2 = action.fCmpParam2;
			fCmpParam3 = action.fCmpParam3;

			fAllInParam1 = action.fAllInParam1;
			fAllInParam2 = action.fAllInParam2;
			fGiveUpParam = action.fGiveUpParam;
		}
		tagRobotAction& operator =(const tagRobotAction& action)
		{
			if (this == &action)
				return *this;
			nModelId = action.nModelId;
			strModeName = action.strModeName;
			card = action.card;
			fAddParam = action.fAddParam;
			vecAddRate = action.vecAddRate;

			fCmpParam1 = action.fCmpParam1;
			fCmpParam2 = action.fCmpParam2;
			fCmpParam3 = action.fCmpParam3;

			fAllInParam1 = action.fAllInParam1;
			fAllInParam2 = action.fAllInParam2;
			fGiveUpParam = action.fGiveUpParam;
			return *this;
		}
		int					nModelId;
		std::string			strModeName;
		std::vector<int>	card;
		double				fAddParam;
		std::vector<int>	vecAddRate;

		double				fCmpParam1;
		double				fCmpParam2;
		double				fCmpParam3;

		double				fAllInParam1;
		double				fAllInParam2;

		double				fGiveUpParam;
	};
	struct tagRobotAction	m_RobotAction;

	std::map<int, tagRobotAction> m_MapRobotAction;

	std::map<uint32_t, GameCard> m_MapSaveCard;

	int64_t				m_nBaseGold;
	int32_t				m_nLookCondition;
	int32_t				m_nPkCondition;

	int64_t				m_nMaxChip;		// 最大注

	std::vector<int>	m_VecAddJetton;
	std::vector<int>	m_VecAddRate;

	std::map<uint32_t,TableUser> m_MapTableUser;

	VectorUserID			m_AllUser;
	VectorUserID			m_CheckUser;	// 看牌
	VectorUserID			m_RobotUser;	// 机器人
	VectorUserID			m_CompareUser;	// 比牌赢

	int64_t					m_lPoolGold;	// 总池

	bool					m_bHasAddAction;// 当前轮是否有加注行为
	std::pair<int32_t,int32_t>	m_pairRound;

	bool					m_bHasShowHand;	// 梭哈	
	bool					m_bHasAllin;	// 孤注一掷

	bool					m_bGiveUp;		// 自己是否弃牌

	bool					m_bCompareAction;

	bool					m_bChecking;

	bool					m_bIsGod;

protected:
	bool	m_init;
	double	m_duration;
	double	m_fLetSee;
private:
	LPlayerPtr	m_player;
	int32_t		m_roomid;
	int32_t		m_tableid;
	int32_t		m_nSelfChairID;
	uint32_t	m_nSelfPlayerID;
	int64_t		m_nSelfGold;
	int64_t		m_nExpend;
protected:
	int8_t		m_nGameStatus;
	int8_t		m_nUserStatus;

	int64_t		m_lCurJetton;		// 当前筹码值

	int32_t		m_nGameRound;		// 当前轮数

	uint32_t	m_nCurOperatorID;	// 当前操作用户
	uint32_t	m_nFirstID;

	uint32_t	m_nShowHandID;

};

ZJH_SPACE_END
