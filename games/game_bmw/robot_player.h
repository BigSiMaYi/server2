#pragma once
#include "robot_def.h"
#include "logic_def.h"
#include <i_game_phandler.h>
#include "i_game_player.h"

#include "BMW_ResultCtrl.h"

BMW_SPACE_BEGIN

typedef struct tagTableUser
{
	int32_t		p_state;	// ״̬
	int32_t		p_active;
	int32_t		p_idx;		// ����
	uint32_t	p_id;		
	int32_t		p_label;	// ��ǩ
	int32_t		p_tag;		// ���
	int64_t		p_expend;	// ��������
	int32_t		p_round;	// ����
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
	VectorUserID			m_CheckUser;	// ����
	VectorUserID			m_RobotUser;	// ������
	VectorUserID			m_CompareUser;	// ����Ӯ

	// ��������
	std::vector<int>		m_VecJetton; 
	int64_t					m_lBankerCondition;

	std::list<uint32_t>		m_ApplyList;
	int32_t					m_nApplyMin;
	int32_t					m_nApplyMax;

	static const int32_t	m_cbJettonArea[PLACE_AREA+1];
	// ����������
	int64_t					m_lRobotMinJetton;					//��С����
	int64_t					m_lRobotMaxJetton;					//������
	int32_t					m_nRobotMinBetTimes;				//��С��ע����
	int32_t					m_nRobotMaxBetTimes;				//�����ע����	
	int32_t					m_nRobotMinBankerTimes;				//��С��ׯ����
	int32_t					m_nRobotMaxBankerTimes;				//�����ׯ����
	int64_t					m_lRobotGiveUpMinWinScore;			//��ׯ���� (��СӮ��)
	int64_t					m_lRobotGiveUpMaxWinScore;			//��ׯ���� (���Ӯ��)
	int64_t					m_lRobotGiveUpMinLostScore;			//��ׯ���� (��С���)
	int64_t					m_lRobotGiveUpMaxLostScore;			//��ׯ���� (������)
	int64_t					m_lRobotGiveUpWinScore;				//��ׯ���� (Ӯ��)
	int64_t					m_lRobotGiveUpLostScore;			//��ׯ���� (���)
	int32_t					m_nRobotGiveUpWinTimes;				//��ׯ���� (Ӯ�ľ���)
	int32_t					m_nRobotGiveUpLostTimes;			//��ׯ���� (��ľ���)	

	bool					m_bAddScore;						//�Ƿ񱬷��³���

	tagAreaChip				m_PlaceChip;

	tagBankerInfo			m_NowBanker;						// ��ǰׯ��

	int64_t					m_lUserMaxScore;					//���������·�����ʣ�ࣩ
	int						m_nCurrentBetTimes;					//���ֿ���ע������ʣ�ࣩ
	int64_t					m_lAreaLimitScore[PLACE_AREA+1];	//���ָ�����ǰ���·֣�ʣ�ࣩ

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
	int32_t		m_nGameRound;		// ��ǰ����


};

BMW_SPACE_END
