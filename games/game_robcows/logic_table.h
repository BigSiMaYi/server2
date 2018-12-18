#pragma once
#include "logic_def.h"
#include "i_game_def.h"
#include <vector>
#include "GameLogic.h"
#include "logic_cardmgr.h"

#include "RobCows_BaseInfo.h"
#include "RobCows_RoomCFG.h"
#include "RobCows_RobotCFG.h"

//@�����Ϸ���̼�¼;
#include "logic2logsvr_msg_type.pb.h"

namespace robcows_protocols
{
	class packetl2c_scene_info_free;
	class packetl2c_scene_info_play;

	class packetl2c_scene_info_play_bank;
	class packetl2c_scene_info_play_bet;
	class packetl2c_scene_info_play_opencard;
	class packetl2c_scene_info_play_display;
}

ROBCOWS_SPACE_BEGIN

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

	int32_t		p_bankrate;	// ��ׯ����
	int32_t		p_betrate;	// ��ע����
	bool		p_opencard;	// ����

	LPlayerPtr	p_playerptr;	

	int32_t		p_round;	// ��������Ч
	bool		p_norob;
	bool		p_nobet;
	bool		p_noopen;
	int32_t		p_noaction;	//

	bool		p_leave;	// ����Ѿ��뿪

	bool		p_runaway;	// ����

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
	bool leave_table(uint32_t pid);//�뿪����
	bool can_leave_table(uint32_t pid , bool bReq = false);
	void getout_table(uint32_t pid);
	//bool change_op(uint32_t pid);//����->��ս
	bool change_sit(uint32_t pid, uint32_t seat_index);//�ı���λ
	bool is_full();
	bool is_opentable();
	uint32_t all_robot();
	uint32_t all_human();
	uint32_t all_extraman();
	bool is_all_robot();
	unsigned int get_max_table_player();
	LPlayerPtr& get_player(int index);
	LPlayerPtr& get_player_byid(uint32_t pid);

	void PlatformKiller(int32_t optype, int32_t cutRound);

	logic_room* get_room();

	int32_t get_seat_byid(uint32_t pid);
	uint32_t get_id_byseat(int32_t seat);

	void req_scene(uint32_t playerid);

	boost::shared_ptr<robcows_protocols::packetl2c_scene_info_free> get_scene_info_msg_free();

	boost::shared_ptr<robcows_protocols::packetl2c_scene_info_play_bank> get_scene_info_msg_play_bank(uint32_t uid);

	boost::shared_ptr<robcows_protocols::packetl2c_scene_info_play_bet> get_scene_info_msg_play_bet(uint32_t uid);

	boost::shared_ptr<robcows_protocols::packetl2c_scene_info_play_opencard> get_scene_info_msg_play_opencard(uint32_t uid);

	boost::shared_ptr<robcows_protocols::packetl2c_scene_info_play_display> get_scene_info_msg_play_display();

	boost::shared_ptr<robcows_protocols::packetl2c_scene_info_play> get_scene_info_msg_play(uint32_t uid);

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
	
	
protected:

	//������λ
	void bc_enter_seat(int seat_index, LPlayerPtr& player);
	void bc_enter_seat(TablePlayer& player);
	//�뿪��λ
	void bc_leave_seat(uint32_t player_id);

public:
	// 
	bool onEventUserReady(uint32_t playerid);

	bool onGameNoticeSpare();

	bool onGameNoticeRobBanker();

	bool onEventGameStart(); 

	bool onNoticeRobBanker();
	bool onGameRobBanker(uint32_t playerid,int32_t nRate,bool bTimeOver=false);
	bool IsValidRate(int32_t nRate);
	bool CanRobBanker(uint32_t playerid,int32_t nRate);
	void defaultRobBanker();
	void maybe_AllRobBanker();

	uint32_t SysAlloctBanker(Vec_UserID &findbanker);

	bool onGameBetRate(uint32_t playerid,int32_t nRate,bool bTimeOver=false);
	bool IsValidBet(int nRate);
	bool CanBet(uint32_t playerid,int32_t nRate);
	void defaultBetRate();
	void maybe_AllBetRate();

	bool onGameOpenCard(uint32_t playerid , bool bTimeOver = false);
	void maybe_AllOpenCard();
	void defaultOpenCard();

	bool onGameDisplayCard();
	
	bool onGameResult();

	void SysRegulatoryPolicy();

	void SysKillerPolicy();
	void PreSysRegulatoryPolicy();

	void SysSwitchCard();
	void SysSwitchCardEx();
	void Switch2Robot(int nRandom=100);
	// �ﵽ��������
	bool checkOutofRange();
	// �ﵽ��עһ��
	bool checkOutofBound(uint32_t playerid);

	void onGameEndCardInfo();

	bool checkStartSpare();

	bool checkGameStart();

	void CleanTableEvent();

	void CleanOutPlayer();

	void onGameCleanOut(uint32_t playerid,int32_t nReason);

	void CleanRunaway();

	void CleanDisconnect();

	void CleanKickedPlayer();

	void CleanRobot();
public:
	//
	int32_t GetActiveUserCount();
	int32_t GetAllUserCount();
	int32_t GetJoinCount();
	// ��ʼ����Ϸ����
	void initGame(bool bResetAll=false);

	// �������Ӳ���
	void repositGame();

	// ��ȡ��һ��
	uint16_t getNextOperator(uint16_t wChair);
	uint32_t getNextOperator2ID(uint16_t wChair);

	uint32_t getNextPlayerID(uint32_t playerid);
	uint32_t GetNextPlayerByID(uint32_t playerid);
	// �ж��Ƿ�����Чλ��
//	bool isVaildSeat(uint16_t wChair);
	// ����
	uint16_t getWiner(uint16_t request,uint32_t respone);
	uint16_t getWiner();
	
	uint32_t CompareWinner(uint32_t reqid,uint32_t resid);
	uint32_t CompareWinner();

	// ��ȡ����
	int getCardModel(uint16_t wChair);
	int getCardModel(BYTE bCard[MAX_CARDS_HAND]);
	// ��������
	bool isNewRound();
	// 
	bool isVaildGold(int64_t lGold);

	int64_t getLessGold(uint16_t& wChair);


	int64_t GetLessGold(uint32_t& lessID);

	void SetCompareNexus(uint32_t playerid,uint32_t nexusid);

	void SetCompareNexus(Vec_UserID &playerids);

	void CleanNoAction(uint32_t playerid);

	void OnEnterBackground(uint32_t playerid);
	void OnDealEnterBackground();
public:
	// ����
	Vector_Chair	m_VecChairMgr;
	Vector_Chair	m_VecClingChairMgr;
	std::map<uint32_t,TablePlayer>	m_MapTablePlayer;
	std::map<int32_t,TablePlayer>	m_MapExtraPlayer;
	// ��Ϸ
	Vec_UserID	m_VecPlaying;		// 
	// ������Ϸ
	Vec_UserID	m_VecActive;		// ������Ϸ���Թ�

	std::map<uint32_t,GameCard> m_MapPlayerCard;
	int32_t	m_cbKillType;
	int32_t	m_cbKillRound;
	int32_t	m_cbGameRound;

	std::map<uint32_t,Vec_UserID> m_MapCompareNexus;	// ���ƹ�ϵ

	uint32_t	m_nBankerID;
	int32_t		m_nBankerRate;

	uint32_t	m_nFirstID;

	// ��ǰ�����û� id
	uint32_t	m_nCurOperatorID;

	int32_t		m_nGameRound;	// ����

	std::vector<int> m_VecAddJetton;

	Vec_UserID	m_VecRecord;
	std::vector<int> m_VecRobRate;
	std::vector<int> m_VecBetRate;
protected:
	float		m_duration;

	float		m_StartTime;
	float		m_BankerTime;
	float		m_BetTime;
	float		m_OpenTime;
	float		m_nDisplayTime;
	float		m_ResultTime;

	int64_t		m_base;
	int32_t		m_BankerRate;
protected:
	logic_cardmgr m_GameCardMgr;
	
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

	int32_t robot_rate();
	void CalcUserLabel();
	int32_t GetMaxLabel();
	int32_t GetLabelByValue(int64_t nValue);
	int32_t GetPlayerLabel(uint32_t playerid);

	void reverse_result(uint32_t reqid,uint32_t resid);
	void reverse_result();
	void reverse_resultEx(uint32_t reqid,uint32_t resid);

	int32_t robot_counter();
	int32_t robot_counter(uint32_t reqid,uint32_t resid);

	uint32_t robot_id(uint32_t uid);

	void robot_switch(uint32_t uid,int nRandom=100);
	int32_t release_robot_seat();
	bool can_release_robot();
	int32_t cling_robotid();
	void joinnextround();
private:
	double m_robot_elapsed;
	double m_clean_elapsed;

	RobCows_RobotCFGData	m_robotcfg;

	//////////////////////////////////////////////////////////////////////////
	// @��Ϸ��Ϣ��¼
	logic2logsvr::RobcowsGameLog mGameLog;
};




ROBCOWS_SPACE_END
