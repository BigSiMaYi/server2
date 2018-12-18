#pragma once
#include "robot_def.h"
#include "logic_def.h"
#include <i_game_phandler.h>
#include "i_game_player.h"

#include "BMW_ResultCtrl.h"

BMW_SPACE_BEGIN

typedef struct tagTableUser
{
	int32_t		p_state;	// 状态
	int32_t		p_active;
	int32_t		p_idx;		// 椅子
	uint32_t	p_id;		
	int32_t		p_label;	// 标签
	int32_t		p_tag;		// 标记
	int64_t		p_expend;	// 当局消耗
	int32_t		p_round;	// 局数
}TableUser;

typedef struct tagBankerInfo
{
	uint32_t	uid;
	int64_t		money;
	int32_t		round;
	int64_t		result;
}BankerInfo;

typedef struct tagAreaInfo
{
	std::array<int64_t,PLACE_AREA> anayone;

	std::array<int64_t,PLACE_AREA> total;
}AreaInfo;

class robot_player
	: public enable_obj_pool<robot_player>
{
public:
	robot_player(void);
	virtual ~robot_player(void);

	void heartbeat(double elapsed);

	void init(uint32_t uid,int64_t gold,int32_t rid,int32_t tid,int32_t seat);

	uint32_t get_pid();

	int64_t get_gold();

	int32_t	get_seat(uint32_t playerid);

	uint32_t get_id(int32_t seat);
public:
	// 
	void initScene();
	// 
	void initGame_free(tagBankerInfo bankerinfo,std::list<uint32_t>& applylist );
	//
	void initGame_play(int32_t nGameState,int32_t nLeftTime,tagBankerInfo bankerinfo,tagAreaInfo areainfo,std::list<uint32_t>& applylist);
	//
	bool onEventUserReady(uint32_t uid);
	//
	void onEventNoticeBanker(tagBankerInfo bankerinfo);
	//
	void onEventBeginPlace();
	//
	void onHeartPlace();
	//
	void onHeartApply();
	//
	void onEventPlace(uint32_t playerid,int32_t nArea,int64_t lGold);
	//
	void onEventNoticeFull();
	//
	void onEventNoticeRun(int32_t nResult);
	//
	void onEventApplyBanker(uint32_t playerid,std::list<uint32_t>& applylist);
	//
	void onEventUnApplyBanker(uint32_t playerid,std::list<uint32_t>& applylist);
	//
	bool onEventEnd(uint32_t playerid,int64_t nResult);

	void onSendReadyMsg();
	
	void onSendPlaceMsg(int32_t nArea,int64_t lGold);
	void onSendApplyBanker();
	void onSendUnApplyBanker();

	bool IsValidPlayer(uint32_t uid);


protected:
	//////////////////////////////////////////////////////////////////////////

	double GetFloatTime(double fTime);
private:

	int					m_nReadyTime;
	int					m_nOperaTime;
	int					m_nResultTime;
	
	std::map<uint32_t,TableUser> m_MapTableUser;

	VectorUserID			m_AllUser;
	VectorUserID			m_CheckUser;	// 看牌
	VectorUserID			m_RobotUser;	// 机器人
	VectorUserID			m_CompareUser;	// 比牌赢

	// 房间配置
	std::vector<int>		m_VecJetton; 
	int64_t					m_lBankerCondition;

	std::list<uint32_t>		m_ApplyList;
	int32_t					m_nApplyMin;
	int32_t					m_nApplyMax;

	static const int32_t	m_cbJettonArea[PLACE_AREA+1];
	// 机器人配置
	int64_t					m_lRobotMinJetton;					//最小筹码
	int64_t					m_lRobotMaxJetton;					//最大筹码
	int32_t					m_nRobotMinBetTimes;				//最小下注次数
	int32_t					m_nRobotMaxBetTimes;				//最大下注次数	
	int32_t					m_nRobotMinBankerTimes;				//最小坐庄次数
	int32_t					m_nRobotMaxBankerTimes;				//最大坐庄次数
	int64_t					m_lRobotGiveUpMinWinScore;			//下庄条件 (最小赢分)
	int64_t					m_lRobotGiveUpMaxWinScore;			//下庄条件 (最大赢分)
	int64_t					m_lRobotGiveUpMinLostScore;			//下庄条件 (最小输分)
	int64_t					m_lRobotGiveUpMaxLostScore;			//下庄条件 (最大输分)
	int64_t					m_lRobotGiveUpWinScore;				//下庄条件 (赢分)
	int64_t					m_lRobotGiveUpLostScore;			//下庄条件 (输分)
	int32_t					m_nRobotGiveUpWinTimes;				//下庄条件 (赢的局数)
	int32_t					m_nRobotGiveUpLostTimes;			//下庄条件 (输的局数)	

	bool					m_bAddScore;						//是否爆发下筹码

	tagAreaChip				m_PlaceChip;

	tagBankerInfo			m_NowBanker;						// 当前庄家

	int64_t					m_lUserMaxScore;					//本局最大可下分数（剩余）
	int						m_nCurrentBetTimes;					//本局可下注次数（剩余）
	int64_t					m_lAreaLimitScore[PLACE_AREA+1];	//本局各区域当前可下分（剩余）

protected:
	bool	m_init;
	double	m_duration;
	double	m_fApply;
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
	int32_t		m_nGameRound;		// 当前轮数


};

BMW_SPACE_END
