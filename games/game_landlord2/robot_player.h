#pragma once
#include "robot_def.h"
#include "logic_def.h"
#include <i_game_phandler.h>
#include "i_game_player.h"


LANDLORD_SPACE_BEGIN

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
	bool onEventEnd(uint32_t playerid, int32_t nEndType);

    bool onEventGetLandlord(uint32_t uid);

	void onSendReadyMsg();
    void onSendApplyBankerMsg();
protected:
	//////////////////////////////////////////////////////////////////////////

	double GetFloatTime(double fTime);
    float     GetRobotTimes(int8_t nStatus);
private:

	int					m_nReadyTime;
	int					m_nOperaTime;
	int					m_nResultTime;
	
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
	bool					m_bHasShowHand;	// 梭哈	
	bool					m_bHasAllin;	// 孤注一掷

	bool					m_bGiveUp;		// 自己是否弃牌

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
protected:
	int8_t		m_nGameStatus;
	int8_t		m_nUserStatus;

	int64_t		m_lCurJetton;		// 当前筹码值

	int32_t		m_nGameRound;		// 当前轮数

	uint32_t	m_nCurOperatorID;	// 当前操作用户
	uint32_t	m_nFirstID;

};

LANDLORD_SPACE_END
