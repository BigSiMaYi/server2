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
}

LANDLORD_SPACE_BEGIN

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
	bool		p_isauto;	// 
	bool		p_islast1;

	int32_t		p_outs;		// 出牌次数

	int64_t		p_selftime;	// 个人时间

}TablePlayer;

typedef std::vector<int32_t> Vector_Chair;

class logic_table : public enable_obj_pool<logic_table>
	, public game_object
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

	int enter_table(LPlayerPtr player);//进入桌子
	void leave_table(uint32_t pid);//离开桌子
	bool can_leave_table(uint32_t pid);
	//bool change_op(uint32_t pid);//坐下->观战
	bool change_sit(uint32_t pid, uint32_t seat_index);//改变座位
	bool is_full();
	bool is_opentable();

	unsigned int get_max_table_player();
	LPlayerPtr& get_player(int index);
	LPlayerPtr& get_player_byid(uint32_t pid);

	logic_room* get_room();

	int32_t get_seat_byid(uint32_t pid);
	uint32_t get_id_byseat(int32_t seat);
	boost::shared_ptr<landlord3_protocols::packetl2c_scene_info_free> get_scene_info_msg_free();
	boost::shared_ptr<landlord3_protocols::packetl2c_scene_info_play> get_scene_info_msg_play(uint32_t uid);

    int32_t get_robot_size();
    bool is_all_robot();

public:
	//广播协议，立刻发送
	template<class T>
	int broadcast_msg_to_client(T msg)
	{
		return broadcast_msg_to_client(m_pids, msg->packet_id(), msg);
	};
	int broadcast_msg_to_client(std::vector<uint32_t>& pids, uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg);


	template<class T>
	void broadcast_msg_to_client2(T msg);
	
	
protected:
	//进入座位
	void bc_enter_seat(int seat_index, LPlayerPtr& player);
	//离开座位
	void bc_leave_seat(uint32_t player_id);

public:
	// 
	bool onEventUserReady(uint32_t playerid);

	void Wash_Card();
	bool Allot_Card(uint32_t playerid,VECPOKER& pokers);
	bool Allot_BaseCard(VECPOKER &pokers);
	bool onGameStart(); 

	bool onNoticeJiao(uint32_t playerid);

	bool onEventJiao(uint32_t playerid,int32_t robrate);
	void defaultJiao(uint32_t playerid);
	void maybe_JiaoEnd(uint32_t playerid,int32_t robrate);
	bool onEventAuto(uint32_t playerid,bool bAuto);
	bool onEventBaseCard();
	void onNoticeChangeRate();

	void defaultOutCard();
	bool onEventOutCard(uint32_t playerid,const VECPOKER&poker);
	bool CheckPoker(uint32_t playerid,const VECPOKER&poker );
	int32_t RemovePoker(uint32_t playerid,const VECPOKER&poker );

	bool onEventPass(uint32_t playerid,bool bTimeOver=false);
	void defaultPass(uint32_t playerid, uint32_t next_pid, bool bTimeOver=false);

	bool onEventTrustee(uint32_t playerid,bool bTrustee=true);

	bool onGameOver(uint32_t playerid,bool bRun=false);

	void CleanOutPlayer();

	void onGameCleanOut(uint32_t playerid,int32_t nReason);

public:
	//
	int32_t GetActiveUserCount();
	int32_t GetAllUserCount();
	// 初始化游戏参数
	void initGame(bool bResetAll=false);

	// 重置桌子参数
	void repositGame();

	uint32_t GetNextPlayerByID(uint32_t playerid, int32_t next_cid);
	// 判断是否是有效位置
//	bool isVaildSeat(uint16_t wChair);

	// 
	bool IsValidRate(int32_t nRate);
	bool CanRobBanker(int32_t nRate);
	void CleanNoAction(uint32_t playerid);

    void PlayerDiscard(uint32_t playerid, const VECPOKER&pokers);

public:
	// 椅子
	Vector_Chair	m_VecChairMgr;

	std::map<uint32_t,TablePlayer>	m_MapTablePlayer;
	
	Vec_UserID	m_VecActive;		// 参与游戏：旁观
	//
	Vec_UserID	m_VecRobBank;		// 抢地主

	std::map<uint32_t,VECPOKER> m_MapPlayerVecPoker;	// 所有牌

	std::map<uint32_t,VECPOKER> m_MapPlayerPokerOut;	// 当前出牌

	VECPOKER	m_nBasePoker;							// 底牌

	uint32_t	m_landlordpid;
    uint32_t	m_lastpid;
    uint32_t	m_curpid;
    int32_t		m_nGameRate;

	uint32_t	m_nFirstID;
	VECPOKER	m_LastPoker;
	uint32_t	m_nLastWiner;

public:

	char m_chun;
	//poker style,max poker val,poker len
	char m_style,m_val,m_len;//
	//那一个玩家是地主
	char m_zidx;
	//这个是游戏翻倍
	int m_Game_FanBei;//max 64
	//这个是每一个玩家相关的信息
	char m_firstidx;
	//这个是底牌
	POKER m_pokerdi[MAXPOKERDI];//牌值

	uint32_t     m_uBomCounter;

	VECPOKER		m_vecCardData;
	VECPOKER		m_vecOutCard;
protected:
	float		m_duration;
	float		m_ReadyTime;
	float		m_BankerTime;
	float		m_OperaTime;
	float		m_ResultTime;
protected:
	logic_card		m_logiccard;
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
	virtual void init_game_object();//注册属性
	virtual bool store_game_object(bool to_all = false);//非数组对象必须实现此接口


	Tfield<int16_t>::TFieldPtr		TableID;			//桌子id

	Tfield<int8_t>::TFieldPtr		TableStatus;		//桌子状态

	//////////////////////////////////////////////////////////////////////////

protected:
    int GenLoadlord();
    int m_questioned;

public:
	void robot_heartbeat(double elapsed);
	void request_robot();
	void release_robot(int32_t playerid);
	int32_t release_robot_seat();
	int32_t robot_rate();
	void reverse_result(uint32_t reqid,uint32_t resid);
	void reverse_result();
	int32_t robot_counter();
	uint32_t robot_id(uint32_t uid);
	void robot_switch(uint32_t uid,int nRandom=100);

private:
	double m_robot_elapsed;

	Landlord3_RobotCFGData	m_robotcfg;
    card_covert m_analyser;
	//////////////////////////////////////////////////////////////////////////
};




LANDLORD_SPACE_END
