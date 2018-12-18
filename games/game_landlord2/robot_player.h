#pragma once
#include "robot_def.h"
#include "logic_def.h"
#include <i_game_phandler.h>
#include "i_game_player.h"


LANDLORD_SPACE_BEGIN

typedef struct tagTableUser
{
	int32_t		p_state;	// ״̬
	int32_t		p_active;
	int32_t		p_idx;		// ����
	uint32_t	p_id;		
	int32_t		p_label;	// ��ǩ
	int32_t		p_tag;		// ���
	int64_t		p_expend;	// ��������
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

	int64_t				m_nMaxChip;		// ���ע

	std::vector<int>	m_VecAddJetton;
	std::vector<int>	m_VecAddRate;

	std::map<uint32_t,TableUser> m_MapTableUser;

	VectorUserID			m_AllUser;
	VectorUserID			m_CheckUser;	// ����
	VectorUserID			m_RobotUser;	// ������
	VectorUserID			m_CompareUser;	// ����Ӯ

	int64_t					m_lPoolGold;	// �ܳ�

	bool					m_bHasAddAction;// ��ǰ���Ƿ��м�ע��Ϊ
	bool					m_bHasShowHand;	// ���	
	bool					m_bHasAllin;	// ��עһ��

	bool					m_bGiveUp;		// �Լ��Ƿ�����

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

	int64_t		m_lCurJetton;		// ��ǰ����ֵ

	int32_t		m_nGameRound;		// ��ǰ����

	uint32_t	m_nCurOperatorID;	// ��ǰ�����û�
	uint32_t	m_nFirstID;

};

LANDLORD_SPACE_END
