#pragma once
#include "logic_def.h"
#include "i_game_def.h"
#include <vector>

#include "BMW_BaseInfo.h"
#include "BMW_RoomCFG.h"
#include "BMW_RobotCFG.h"
#include "BMW_LotteryCFG.h"
#include "BMW_ResultCtrl.h"
#include "card_def.h"


namespace bmw_protocols
{
	class packetl2c_scene_info_free;
	class packetl2c_scene_info_play;
}

BMW_SPACE_BEGIN

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
	int64_t		p_tax;		// ˰��

	LPlayerPtr	p_playerptr;	

	int32_t		p_noaction;	//

	bool		p_leave;	// ����Ѿ��뿪

	time_t		p_time;

	bool		p_runaway;	// ����

	bool		p_background;

	bool		p_kick;	// �߳�
}TablePlayer;

typedef std::vector<int32_t> Vector_Chair;

class logic_table: public enable_obj_pool<logic_table>
	,public game_object
{
public:
	logic_table(void);
	virtual ~logic_table(void);
	void release();
	void init_table(uint16_t tid,int32_t nPlayerCount ,logic_room* room);

	virtual uint32_t get_id();

	int8_t get_status();
	void set_status(int8_t state);

	void heartbeat( double elapsed );

	int enter_table(LPlayerPtr player);//��������
	bool leave_table(uint32_t pid);//�뿪����
	bool can_leave_table(uint32_t pid);
	void getout_table(uint32_t pid);
	//bool change_op(uint32_t pid);//����->��ս
	bool change_sit(uint32_t pid, uint32_t seat_index);//�ı���λ
	bool is_full();
	bool is_opentable();
	bool is_all_robot();
	int32_t all_robot();
	unsigned int get_max_table_player();
	LPlayerPtr& get_player(int index);
	LPlayerPtr& get_player_byid(uint32_t pid);

	logic_room* get_room();
	int32_t get_seat_byid(uint32_t pid);
	uint32_t get_id_byseat(int32_t seat);

	void req_scene(uint32_t playerid);

	boost::shared_ptr<bmw_protocols::packetl2c_scene_info_free> get_scene_info_msg_free();
	boost::shared_ptr<bmw_protocols::packetl2c_scene_info_play> get_scene_info_msg_play(uint32_t uid);

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

	bool onEventGameStart(); 

	bool onGameNoticeStart();

	bool onNoticeBanker();
	bool onNoticeChangeBanker();

	bool onGameBetArea(uint32_t playerid,int32_t nArea,int64_t lGold);
	bool CheckBetArea(uint32_t playerid,int32_t nArea,int64_t lGold);
	bool CheckPlaceFull();

	void onSendBetResult(uint32_t playerid,int32_t nArea,int64_t lGold,int32_t nRet);

	bool onGameApplyBanker(uint32_t playerid);
	void onSendApplyBankerResult(uint32_t playerid , int32_t nRet);

	bool onGameUnApplyBanker(uint32_t playerid);
	void onSendUnApplyBankerResult(uint32_t playerid , int32_t nRet);
	bool ChangeBanker(bool bCancelCurrentBanker);
	bool IsContinueBanker(int64_t lBankerScore);
	bool CheckApply();
	bool CheckNextBanker();

	bool onGameRequestHistory(uint32_t playerid);

	bool onGamePlaceAgain(uint32_t playerid);
	void onSendPlaceAgainResult(uint32_t playerid , int32_t nRet); 
	
	bool onGameEnd();

	void CleanOutPlayer();

	void onGameCleanOut(uint32_t playerid,int32_t nReason);

	void ReadLotteryRate();
	// ��ɢ
	void DisbandPlayer();

	bool onPlayerRunaway(uint32_t playerid);

	void CleanRunaway();

	bool IsPlace(uint32_t playerid);
 
	void OnEnterBackground(uint32_t playerid);
	void OnDealEnterBackground();

	void CleanKickedPlayer();

public:
	int64_t CountPlayerAllChip(uint32_t playerid);

	int64_t CountPlayerMaxChip(uint32_t playerid);

	int64_t CountAreaAllChip(int32_t nArea);

	int64_t CountBigBenzChip(uint32_t playerid,int64_t lJetton);
	int64_t CountSmallBenzChip(uint32_t playerid,int64_t lJetton);

	int64_t CountBigBMWChip(uint32_t playerid,int64_t lJetton);
	int64_t CountSmallBMWChip(uint32_t playerid,int64_t lJetton);

	int64_t CountBigVWChip(uint32_t playerid,int64_t lJetton);
	int64_t CountSmallVWChip(uint32_t playerid,int64_t lJetton);

	int64_t CountBigPorscheChip(uint32_t playerid,int64_t lJetton);
	int64_t CountSmallPorscheChip(uint32_t playerid,int64_t lJetton);

	void RandGameIndex();
	void RandomIndexByStock();
	void GetRandRate(int nRate[]);
	int32_t GetIndexSummary(int32_t nIndex, int32_t nRand, int64_t& lAndroidScore);

	int64_t CalculateScore();

	void WriteSystemScore(int64_t lSystemScore);

	// ��������
	void AnalysisAreaScore(int32_t nIndex,int64_t& lAndroidScore);

	int32_t RandGameResult();

	int32_t GetMaxArea();
	// �����ע��������
	int32_t GetPlayerMaxArea();

public:
	//
	int32_t GetAllUserCount();
	// ��ʼ����Ϸ����
	void initGame(bool bResetAll=false);

	// �������Ӳ���
	void repositGame();

	uint32_t getNextPlayerID(uint32_t playerid);
	uint32_t GetNextPlayerByID(uint32_t playerid);


	// ��������
	bool isNewRound();
	// 
	bool isVaildGold(int64_t lGold);
	bool isVaildArea(int32_t nArea);
	bool isApplyBank(uint32_t playerid);


	void CleanNoAction(uint32_t playerid);
public:
	// ����
	Vector_Chair	m_VecChairMgr;

	std::map<uint32_t,TablePlayer>	m_MapTablePlayer;
	// ��Ϸ	
	BYTE		m_bPlayingCnt;
	Vec_UserID	m_VecPlaying;		// ��Ϸ�У����ơ�������
	// ������Ϸ
	BYTE		m_bActiveUserCnt;
	Vec_UserID	m_VecActive;		// ������Ϸ���Թ�


	std::map<uint32_t,Vec_UserID> m_MapCompareNexus;	// ���ƹ�ϵ

	uint32_t	m_nFirstID;

	uint32_t	m_nLastWiner;
	
	// ��ǰ�����û� id
	uint32_t	m_nCurOperatorID;

	int64_t		m_lCurJetton;		// ��ǰ����ֵ

	int64_t		m_lTotalSliver;		// ��������ĳ��������ע;

	int64_t		m_lInventPool;	// ��������

	int64_t		m_lEqualize;	// ����ֵ

	int32_t		m_nGameRound;	// ����

	int32_t		m_nEndType;

	int			m_nShowHandCnt;		// ��ǰ�������

	int32_t		m_nOperaFlag;
	Vec_UserID	m_VecAllin;			// ��עһ��
	
public:
	uint32_t	m_nBankerID;		// ��ǰׯ��Ϣ
	int64_t		m_nBankerGold;
	int32_t		m_nBankerRound;
	int64_t		m_nBankerResult;
	std::deque<uint32_t> m_ApplyBanker; // ��ׯ

	Vec_UserID m_unApply;	// ��ׯ

	const BMW_RoomCFGData* m_roomcfg;

	BMW_LotteryCFGData m_lotterycfg;


	std::vector<int>	m_VecJetton; 

	std::map<uint32_t,tagAreaChip> m_MapAreaChip;
	std::map<uint32_t,tagAreaChip> m_MapAreaChipHistory;
	std::map<uint32_t,int64_t>	m_MapWiner;

	typedef std::pair<uint32_t,int64_t> BMWPAIR;

	int64_t		m_lAllBigBenz;
	int64_t		m_lAllSmallBenz;

	int64_t		m_lAllBigBMW;
	int64_t		m_lAllSmallBMW;

	int64_t		m_lAllBigVW;
	int64_t		m_lAllSmallVW;

	int64_t		m_lAllBigPorsche;
	int64_t		m_lAllSmallPorsche;
	//
	int64_t		m_lAllBigBenz2;
	int64_t		m_lAllSmallBenz2;

	int64_t		m_lAllBigBMW2;
	int64_t		m_lAllSmallBMW2;

	int64_t		m_lAllBigVW2;
	int64_t		m_lAllSmallVW2;

	int64_t		m_lAllBigPorsche2;
	int64_t		m_lAllSmallPorsche2;

	bool		m_bChangeBanker;

	std::list<int32_t>	m_ListHistory;

	uint64_t	m_dwLastRandTick;

	int32_t		m_cbResultIndex;

	int64_t		m_lFleeScore;	//�û�����

	static int64_t	m_lAndroidWinScore;	// �������ۼ�

	Vec_UserID	m_enterback;

protected:
	float		m_duration;
	float		m_fFreeTime;
	float		m_fPlaceTime;
	float		m_fAheadTime;
	float		m_fRunTime;
	float		m_fResultTime;
protected:

	std::vector<int> m_VecCardRate;

	int32_t		m_nRate;	// ˰��
	int64_t		m_lAllTax;	// ˰��
	int64_t		m_lAITax;	// ������˰��
private:


	logic_room* m_room;

	int32_t		m_nMaxPlayerCount;

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
	void banker_heartbeat(double elapsed);
	void request_robot();
	void request_banker();
	void release_robot(int32_t playerid);

	int32_t release_robot_seat();

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

	void robot_switch(uint32_t uid,int nRandom=100);
 
private:
	double m_robot_elapsed;
	double m_banker_elapsed;
	BMW_RobotCFGData	m_robotcfg;

	//////////////////////////////////////////////////////////////////////////
};





BMW_SPACE_END
