#pragma once
#include "logic_def.h"
#include "i_game_def.h"
#include <vector>
#include <random>
#include "GameLogic.h"
#include "logic_cardmgr.h"

#include "GoldFlower_BaseInfo.h"
#include "GoldFlower_RoomCFG.h"
#include "GoldFlower_RobotCFG.h"

//@add by Hunter 2017/08/08;
//@�����Ϸ���̼�¼;
#include "logic2logsvr_msg_type.pb.h"

namespace zjh_protocols
{
	class packetl2c_scene_info_free;
	class packetl2c_scene_info_play;
}

ZJH_SPACE_BEGIN

typedef struct tagTablePlayer
{
	int32_t		p_state;	// ״̬

	int32_t		p_active;
	int32_t		p_playing;		// 	
	
	int32_t		p_idx;		// ����
	uint32_t	p_id;		

	int32_t		p_label;	// ��ǩ
	int32_t		p_tag;		// ���
	double		p_percent;	// ʤ��

	int64_t		p_expend;	// ��������
	int64_t		p_asset;	// Я�����
	int64_t		p_result;	// ����

	int32_t		p_cardflag;	// ����״̬

	LPlayerPtr	p_playerptr;	

	int32_t		p_noaction;	//

	bool		p_leave;	// ����Ѿ��뿪

	time_t		p_time;

	bool		p_background;

	bool		p_kick;	// �߳�
}TablePlayer;

//typedef std::pair<uint32_t,int64_t> PAIR;
typedef std::vector<PAIR> Vector_pair;

typedef std::vector<int32_t> Vector_Chair;

class logic_table: public enable_obj_pool<logic_table>
	,public game_object
{
public:
	logic_table(void);
	virtual ~logic_table(void);
	void release();
	void init_table(uint16_t tid, logic_room* room);

	virtual uint32_t get_id();

	int8_t get_status();
	void set_status(int8_t state);

	void heartbeat( double elapsed );

	int enter_table(LPlayerPtr player);//��������
	void leave_table(uint32_t pid,bool bkick=false);//�뿪����
	bool can_leave_table(uint32_t pid);
	void getout_table(uint32_t pid);
	//bool change_op(uint32_t pid);//����->��ս
	bool change_sit(uint32_t pid, uint32_t seat_index);//�ı���λ
	bool is_full();
	bool is_opentable();
	bool is_all_robot();
	int32_t all_robot();
	int32_t all_man();
	unsigned int get_max_table_player();
	LPlayerPtr& get_player(int index);
	LPlayerPtr& get_player_byid(uint32_t pid);

	void PlatformKiller(int32_t optype, int32_t cutRound);
	logic_room* get_room();
	int32_t get_seat_byid(uint32_t pid);
	uint32_t get_id_byseat(int32_t seat);

	void req_scene(uint32_t playerid);
	boost::shared_ptr<zjh_protocols::packetl2c_scene_info_free> get_scene_info_msg_free();
	boost::shared_ptr<zjh_protocols::packetl2c_scene_info_play> get_scene_info_msg_play(uint32_t uid);

public:
	//�㲥Э�飬���̷���
	template<class T>
	int broadcast_msg_to_client(T msg)
	{
		return broadcast_msg_to_client(m_pids, msg->packet_id(), msg);
	};
	int broadcast_msg_to_client(std::vector<uint32_t>& pids, uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg);


	template<class T>
	void broadcast_msg_to_client2(T msg);

	template<class T>
	void broadcast_msg_to_client3(T msg);
	
	template<class T>
	void add_msg_to_list(T msg)
	{
		m_msglist.push_back(msg_packet_one(msg->packet_id(), msg));
	};
	void broadcast_msglist_to_client();
protected:

	//������λ
	void bc_enter_seat(int seat_index, LPlayerPtr& player);
	//
	void bc_enter_seat(TablePlayer& player);
	//�뿪��λ
	void bc_leave_seat(uint32_t player_id);

public:
	// 
	bool onEventUserReady(uint32_t playerid);

	bool onGameNoticeSpare();

	bool onEventGameStart(); 

	bool onGameNoticeStart();

	bool onGameFlow(uint32_t playerid,int64_t lGold);

	bool onGameAdd(uint32_t playerid,int64_t lAddGold);

	bool onGameCompare(uint32_t reqid,uint32_t resid);

	bool onGameCompareEx(uint32_t playerid);

	bool onGameShowHand(uint32_t playerid);

	bool onGameCheck( uint32_t playerid);

	bool onGameGiveUp(uint32_t playerid,bool bTimeOver=false);

	bool onGameEnd(uint32_t playerid,int nEndType);

	bool onGameAllowToSee(uint32_t playerid);
	// Ⱥħ����
	bool onGameAllPK(int nCount);
	// ��עһ��
	bool onGameAllIn(uint32_t playerid);
	// �ﵽ��������
	bool checkOutofRange();
	// �ﵽ��עһ��
	bool checkOutofBound(uint32_t playerid);

	void onGameEndCardInfo();

	bool checkStartSpare();

	bool checkGameStart();

	void CleanOutPlayer();

	void onGameCleanOut(uint32_t playerid,int32_t nReason);

	void ReadCardRate();
	// ��ɢ
	void DisbandPlayer();

	bool onPlayerRunaway(uint32_t playerid);

	void CleanDisconnect();

	void OnEnterBackground(uint32_t playerid);
	void OnDealEnterBackground();

	void CleanKickedPlayer();

	void SendGameInfo2Robot(uint32_t killer);
public:
	//
	int32_t GetActiveUserCount();
	int32_t GetAllUserCount();
	// ��ʼ����Ϸ����
	void initGame(bool bResetAll=false);

	// �������Ӳ���
	void repositGame();

	// ��ȡ��һ��
	uint32_t GetNextPlayerByID(uint32_t playerid);
	// �ж��Ƿ�����Чλ��
//	bool isVaildSeat(uint16_t wChair);
	// ����
	uint32_t CompareWinner(uint32_t reqid,uint32_t resid);
	uint32_t CompareWinner();
	uint32_t CompareWinnerEx();
	// ��������
	bool isNewRound();
	// 
	bool isVaildGold(int64_t lGold);

	int64_t GetLessGold(uint32_t& lessID);

	void SetCompareNexus(uint32_t playerid,uint32_t nexusid);

	void SetCompareNexus(Vec_UserID &playerids);

	void CleanNoAction(uint32_t playerid);
public:
	// ����
	Vector_Chair	m_VecChairMgr;

	std::map<uint32_t,TablePlayer>	m_MapTablePlayer;
	std::map<int32_t, TablePlayer>	m_MapExtraPlayer;
	// ��Ϸ
	Vec_UserID	m_VecPlaying;		// ��Ϸ�У����ơ�������
	// ������Ϸ
	Vec_UserID	m_VecActive;		// ������Ϸ���Թ�

	std::map<uint32_t,GameCard> m_MapPlayerCard;

	std::map<uint32_t,Vec_UserID> m_MapCompareNexus;	// ���ƹ�ϵ

	// @�ڱ��ƶ����ڼ���Ҫ��ʱ�뿪,��IDΨһ
	uint32_t	m_uCompareID;
	double		m_checkdelay;

	uint32_t	m_uBankerID;

	uint32_t	m_nFirstID;

	uint32_t	m_nLastWiner;
	
	// ��ǰ�����û� id
	uint32_t	m_nCurOperatorID;

	// @ kill 
	int32_t	m_cbKillType;
	int32_t	m_cbKillRound;
	int32_t	m_cbGameRound;

	std::vector<int> m_VecAddJetton;

	int64_t		m_lCurJetton;		// ��ǰ����ֵ

	int64_t		m_lTotalSliver;		// ��������ĳ��������ע;

	int64_t		m_lInventPool;	// ��������

	int64_t		m_lEqualize;	// ����ֵ



	int32_t		m_nGameRound;	// ����

	int32_t		m_nEndType;

	int			m_nShowHandCnt;		// ��ǰ�������

	int32_t		m_nOperaFlag;
	Vec_UserID	m_VecAllin;			// ��עһ��
	
protected:
	double		m_duration;
	double		m_ReadyTime;
	double		m_OperaTime;
	double		m_ResultTime;
	double		m_oversee;
protected:
	CGameLogic	m_GameLogic;

	logic_cardmgr m_GameCardMgr;
	std::vector<int> m_VecCardRate;
private:

	logic_room* m_room;

	std::vector<LPlayerPtr> m_players;

	std::vector<uint32_t> m_pids;

	uint16_t m_player_count;
	void inc_dec_count(bool binc = true);

	double m_elapse;
	std::vector<msg_packet_one> m_msglist;
	double m_checksave;

	//////////////////////////////////////////////////////////////////////////
	void create_table();
	bool load_table();
public:
	virtual void init_game_object();//ע������
	virtual bool store_game_object(bool to_all = false);//������������ʵ�ִ˽ӿ�


	Tfield<int16_t>::TFieldPtr		TableID;			//����id

	Tfield<int8_t>::TFieldPtr		TableStatus;		//����״̬

	//////////////////////////////////////////////////////////////////////////
public:
	void robot_heartbeat(double elapsed);

	void request_robot();

	void release_robot(int32_t playerid);

	int32_t release_robot_seat();
	bool can_release_robot();

	int32_t robot_rate();
	void CalcUserLabel();
	int32_t GetMaxLabel();
	int32_t GetLabelByValue(int64_t nValue);

	void reverse_result(uint32_t reqid,uint32_t resid);
	void reverse_result();

	void reverse_resultEx(uint32_t reqid,uint32_t resid);

	int32_t robot_counter();
	int32_t robot_counter(uint32_t reqid,uint32_t resid);

	uint32_t robot_id(uint32_t uid);

	void robot_switch(uint32_t uid);
	void robot_switch(uint32_t robotid,uint32_t playerid);

	int32_t cling_robotid();
	void joinnextround();

	uint32_t GetCurLargestPlayer(bool bAll=true);
	int64_t GetAllRobotExpend();

	void SysRegulatory4Model(uint32_t playerid);

	bool SysRegulatoryCompare(uint32_t playerid,uint32_t robotid);
	
	bool SysRegulatoryCompareEx(uint32_t playerid, uint32_t robotid);

	bool SysRegulatoryCheckCard(uint32_t robotid);

	int32_t CounterTagC();

	void SysKillerPolicy();

	uint32_t PreSysRegulatoryPolicy();

private:
	double m_robot_elapsed;

	GoldFlower_RobotCFGData	m_robotcfg;

	//////////////////////////////////////////////////////////////////////////
	//@ÿ�ֽ�����¼һ������;
	logic2logsvr::ZJHGameLog  mGameLog;
	std::map<uint32_t, std::vector<tagOptInfo>> mOptLog;
	void save_gamelog(uint32_t playerid,tagOptInfo  OptInfo);
};




ZJH_SPACE_END
