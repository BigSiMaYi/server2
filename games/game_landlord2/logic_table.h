#pragma once
#include "logic_def.h"
#include "i_game_def.h"
#include <vector>

#include "Landlord3_BaseInfo.h"
#include "Landlord3_RoomCFG.h"
#include "Landlord3_RobotCFG.h"
#include "card_def.h"
#include "logic_card.h"
#include "card_covert.h"


namespace landlord3_protocols
{
	class packetl2c_scene_info_free;
	class packetl2c_scene_info_play;
    class packetl2c_scene_info_fapai;
    class packetl2c_scene_info_banker;
    class msg_scene_info_play;
}
namespace logic2logsvr
{
    class LandlordGameLog;
    class LandlordCardsInfo;
}

LANDLORD_SPACE_BEGIN
class robot_cfg;

typedef struct tagTablePlayer
{
	int32_t		p_state;	// 状态

	int32_t		p_active;
	int32_t		p_playing;		// 	
	
	int32_t		p_idx;		// 椅子
	uint32_t	p_id;		

	int32_t		p_label;	// 标签
	int32_t		p_tag;		// 标记

	int64_t		p_expend;	// 当局消耗
	int64_t		p_asset;	// 携带金币
	int64_t		p_result;	// 结算

	int32_t		p_cardflag;	// 明牌状态

	LPlayerPtr	p_playerptr;	

	int32_t		p_noaction;	//

	int32_t		p_bankrate;	// 抢庄倍率
	bool		    p_isauto;	// 
	bool		    p_islast1;

	int32_t		p_outs;		// 出牌次数

	int64_t		p_selftime;	// 个人时间
    int64_t		p_bankertime;	//抢庄开始时间;
    int64_t		p_outcardtime;	//出牌时间;
	int64_t		p_autotime;	//托管时间;
    bool         p_discardtimeout;
    int64_t		p_begingold;
}TablePlayer;

//////////////////////////////////////////////////////////////////////////

class logic_table : public enable_obj_pool<logic_table>
{
public:
	logic_table(void);
	virtual ~logic_table(void);

public:
	void init_table(uint16_t tid, logic_room* room);
	void heartbeat( double elapsed );

	int    enter_table(LPlayerPtr player);//进入桌子
	void leave_table(uint32_t pid);//离开桌子
	bool can_leave_table(uint32_t pid);
	bool can_enter_table(LPlayerPtr& player);
	bool change_sit(uint32_t pid, uint32_t seat_index);//改变座位
	
    int32_t get_robot_size();
    int32_t get_player_size();
	LPlayerPtr& get_player_byid(uint32_t pid);
	logic_room* get_room();
	int32_t get_seat_byid(uint32_t pid);
	uint32_t get_id_byseat(int32_t seat);
    uint32_t get_id();
    int8_t get_status();
    void set_status(int8_t state);

    bool is_full();
    bool is_opentable();
    bool is_all_robot();

    //----------------------------
    void SendCardMessage(LPlayerPtr player, VECPOKER& poker);
    bool onEventBaseCard(std::vector<POKER>& basePoker);

public:
    bool onEventUserReady(uint32_t playerid);

    bool onGameStart();
    
    bool onEventJiao(uint32_t playerid, int32_t robrate);
    void defaultJiao(uint32_t playerid);
    void maybe_JiaoEnd(uint32_t playerid, int32_t robrate);
    bool onEventAuto(uint32_t playerid, bool bAuto);
   
    
    void defaultOutCard();
    bool onEventOutCard(uint32_t playerid, const VECPOKER&poker, bool isauto = false);
    bool CheckPoker(uint32_t playerid, const VECPOKER&poker);
    int32_t RemovePoker(uint32_t playerid, const VECPOKER&poker);

    bool onEventPass(uint32_t playerid, bool bTimeOver = false);
	bool defaultPass(uint32_t playerid, uint32_t next_pid, bool bTimeOver = false);

    bool onGameOver(uint32_t playerid, bool bRun = false);
   
protected: //牌相关处理函数;
    void Wash_Card(bool reset = false);
    bool Allot_Card(uint32_t playerid, VECPOKER& pokers);
    bool Allot_BaseCard(VECPOKER &pokers);
    void server_stop();
    void MixRobotCards(uint32_t	landlordpid);
    void CheckPlayerStatus();
	bool CheckTrusteeStatus(TablePlayer& user, bool bAuto);

public:
    int32_t GetActiveUserCount();
    int32_t GetAllUserCount();
    // 初始化游戏参数;
    void initGame(bool bResetAll = false);

    // 重置桌子参数;
    void repositGame();

    uint32_t GetNextPlayerByID(uint32_t playerid, int32_t next_cid);

    bool IsValidRate(int32_t nRate);
    bool CanRobBanker(int32_t nRate);
    void CleanNoAction(uint32_t playerid);

    bool PlayerDiscard(uint32_t playerid, const VECPOKER&pokers);

    void inc_dec_count(bool binc = true);
    uint32_t    GetNextPid(uint32_t pid);
    void CheckLandlordTimeout(uint32_t pid);
    void CheckDiscardTimeout(TablePlayer& tb_player);
    void SetPlayerTime(uint32_t pid);
    bool IsRobot(uint32_t pid);
    void service_ctrl(int32_t optype);
	void robot_ctrl(bool on_off);

	bool check_player_stock();

	void kill_points(int32_t cutRound, bool status);
	int    get_cut_round_tag();
	bool cut_round_check(int cut_round);

public://消息;
    boost::shared_ptr<landlord3_protocols::packetl2c_scene_info_free> get_scene_info_msg_free(uint32_t uid);
    boost::shared_ptr<landlord3_protocols::packetl2c_scene_info_play> get_scene_info_msg_play(uint32_t uid);
    boost::shared_ptr<landlord3_protocols::packetl2c_scene_info_fapai> get_scene_info_msg_fapai(uint32_t uid);
    boost::shared_ptr<landlord3_protocols::packetl2c_scene_info_banker> get_scene_info_msg_banker(uint32_t uid);
    void fill_playerinfo(uint32_t uid, landlord3_protocols::msg_scene_info_play* playerinfo);
    
protected:
    //template<class T>
    //int broadcast_msg_to_client(T msg)
    //{
    //    return broadcast_msg_to_client(this->m_pids, msg->packet_id(), msg);
    //};
    int broadcast_msg_to_client(std::vector<uint32_t>& pids, uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg);

    template<class T>
    void broadcast_msg_to_client2(T msg);

	void bc_enter_seat(int seat_index, LPlayerPtr& player);
	void bc_leave_seat(uint32_t player_id);

    void CleanOutPlayer();
    void onGameCleanOut(uint32_t playerid, int32_t nReason);
    bool onNoticeJiao(uint32_t playerid);
    void onNoticeChangeRate();

    void ReplayCardLog(int32_t playerid, const std::string& logcard);
    float GetRobotTimes(int8_t nStatus);

public:
    std::vector<int32_t>	m_chair_ids;
	std::map<uint32_t,TablePlayer>	m_MapTablePlayer;
	Vec_UserID	m_VecActive;		// 参与游戏：旁观;
    int32_t m_service_status;
    bool     m_server_stop;
private:
    uint32_t     m_bomCounter;
	std::map<uint32_t,VECPOKER> m_player_pokers;	        // 所有牌;
	std::map<uint32_t,VECPOKER> m_player_out_pokers;	// 当前出牌;
	VECPOKER	m_nBasePoker;							// 底牌;
    VECPOKER	m_vecCardData;
    VECPOKER	m_vecOutCard;

    int32_t		m_nGameRate;
    int32_t     m_bankerRate;
	
public:
	char m_chun;
	char m_style,m_val,m_len;

private:
	logic_room* m_room;
    logic_card	m_logiccard;

	uint16_t m_player_count;

	double m_elapse;

	double m_checksave;

private:
    uint16_t m_tid;
    int8_t m_tb_status;
    int m_questioned;

    double		m_duration;
	double		m_ReadyTime;
	double		m_BankerTime;
	double		m_OperaTime;
	double		m_ResultTime;

private:
    card_covert m_analyser;
    uint32_t	m_landlordpid;
    uint32_t	m_lastpid;
    uint32_t	m_curpid;
    uint32_t	m_nFirstID;
    uint32_t	m_nextpid;
    uint32_t	m_firstid; //第一个叫地主的人，如果所有玩家都不叫，则地主给第一个叫地主的人，1分;
    //uint32_t   m_get_landloar_id;
    //uint32_t   m_pro_landloard_id;
	std::pair<int, int> m_player_stock_factor;
	GOLD_TYPE m_player_recharge;
	bool  m_player_cut_roundflag;

	bool m_log_flag;
	bool m_robot_switch;
	int m_tb_palyer_size;
	int m_round;
	int m_cutround_tag;
private:
    std::shared_ptr<logic2logsvr::LandlordGameLog> m_landlordlog;
	std::shared_ptr<robot_cfg> m_robot_cfg;
	int m_cutroudtag = 0;
	bool m_kill_points_switch = false;
	int m_cfgCutRound = 3;
	int m_gametimes = 0;
	int m_cuttimes = 0;
	int32_t m_tb_pid = 0;
};

//////////////////////////////////////////////////////////////////////////

class robot_cfg
{
public:
    robot_cfg(logic_room* room, logic_table* table, std::map<uint32_t, TablePlayer>&	players, int16_t tid);

public:
    void robot_heartbeat(double elapsed);
    void request_robot();
    void release_robot(int32_t playerid);
    int32_t release_robot_seat();
    int32_t robot_rate();
    void reverse_result(uint32_t reqid, uint32_t resid);
    void reverse_result();
    int32_t robot_counter();
    uint32_t robot_id(uint32_t uid);
    void robot_switch(uint32_t uid, int nRandom = 100);
    int32_t robot_maxcounter();
	void robot_ctrl(bool on_off);
private:
    double m_robot_elapsed;
    Landlord3_RobotCFGData	m_robotcfg;
    std::map<uint32_t, TablePlayer>&	m_MapTablePlayer;
    logic_room* m_room;
	logic_table* m_table;
    int16_t m_tid;
	bool m_robot_switch;
};

//////////////////////////////////////////////////////////////////////////

class db_table : public game_object
{
public:
    db_table();
    ~db_table();

public:
    void release();
    bool load_table();

    virtual void init_game_object();//注册属性
    virtual bool store_game_object(bool to_all = false);//非数组对象必须实现此接口


    Tfield<int16_t>::TFieldPtr		TableID;			//桌子id

    Tfield<int8_t>::TFieldPtr		TableStatus;		//桌子状态

    int32_t m_roomid;
};

LANDLORD_SPACE_END
