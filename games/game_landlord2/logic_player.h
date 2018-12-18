#pragma once
#include "logic_def.h"
#include <i_game_phandler.h>
#include "i_game_player.h"
#include "robot_mgr.h"

LANDLORD_SPACE_BEGIN

enum quest_type
{
	game_count = 1301,
	win_gold,
	cards_win,
};

class logic_player: 
	public enable_obj_pool<logic_player>
	,public i_game_phandler
	,public game_object
{
public:
	logic_player(void);
	virtual ~logic_player(void);

	void heartbeat( double elapsed );

	void init(iGPlayerPtr player);

	//////////////////////////////////////////////////////////////////////////
	//从服务器通知逻辑的接口
	//属性
	virtual void on_attribute_change(int atype, int v);
	virtual void on_attribute64_change(int atype, GOLD_TYPE v = 0);
	virtual void quest_change_from_world(int quest_type,int count,int param) ;
	//玩家掉线
	virtual void on_change_state();
	virtual int  cltReq_leaveGame();
	//////////////////////////////////////////////////////////////////////////
	//离开游戏时调用
	void enter_game(logic_lobby* lobby);
	bool join_table(logic_table* table,int32_t seat=0);
	logic_table* get_table();
	int32_t get_seat() const;
	void leave_table();
	bool can_leave_table();

	void set_status(int state);
	int32_t get_status();
	
	void clear_round(int round=0);
	void add_round(int round=1);
	int32_t get_round();

	void set_wait(int wait);
	void add_wait(int wait=1);
	int32_t get_wait();

	bool onEventUserReady();
	//////////////////////////////////////////////////////////////////////////
	void release();//退出整个游戏

	// 获取玩家ID
	uint32_t get_pid();
	//获取玩家当前金币(游戏调用)
	GOLD_TYPE get_gold();
    //开局记录当前玩家身上金币;
    GOLD_TYPE get_pre_gold();
	//获取玩家VIP等级
	int16_t get_viplvl();
	//获取礼券
	int get_ticket();
	//获取昵称
	const std::string& get_nickname();
	//性别
	int get_sex();
	//头像
	int get_photo_frame();
	//头像
	const std::string& get_icon_custom();
	//地域信息
	const std::string& GetUserRegion();
	//是否机器人
	bool is_robot();
	//机器人逻辑
	LRobotPtr& get_robot();
	//创建机器人逻辑
	void create_robot();

	//改变金币(游戏调用)
	bool change_gold(GOLD_TYPE v, bool needbc = false);
	//改变礼券(游戏调用)
	bool change_ticket(int v, int season);
	bool change_gold2(int v, int season);
	//同步金额
	void sync_change_gold();
	// 写库
	void write_property(int64_t value,int64_t tax = 0,const std::string& param="");
	//广播
	void bc_game_msg(int money, const std::string& sinfo, int mtype = 1);

	void add_star_lottery_info(int32_t award,int32_t star = 0);
	void quest_change(int questid, int count=1, int param=0);

	template<class T>
	int send_msg_to_client(T msg)
	{
#if 1
		return m_player->send_msg_to_client(msg);
#else
		if(!is_robot())
		{
			return m_player->send_msg_to_client(msg);
		}
		else
		{
			robot_mgr::instance().recv_packet(get_pid(), msg->packet_id(), msg);
		}
		return 1;
#endif
	};

	int send_packet(uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg);
    void reqPlayer_leaveGame();

    void  set_kick_status(int bforce) { m_bforce = bforce; }
    int get_kick_status() { return m_bforce; }
	int get_leave_status() {return m_leave_status;}
	void set_leave_status(int leave_status) { m_leave_status = leave_status; }
	std::pair<int, int> get_stock_factor(); //库存系数;
	int get_roomid();
	GOLD_TYPE get_recharge_gold() { return m_recharge_gold; }
	bool get_cut_round_flag();
	void load_player_stock();

private:
	logic_lobby* m_lobby;
	logic_room* m_room;
	logic_table* m_table;

	LRobotPtr m_robot;

	GOLD_TYPE m_logic_gold;	
	GOLD_TYPE m_change_gold;
    GOLD_TYPE m_pre_logic_gold;
    GOLD_TYPE m_recharge_gold; //充值金额;
	bool  m_cut_round_flag;
    int m_bforce;
	int m_leave_status;
	std::pair<int, int> m_stock_factor;

	Tfield<int32_t>::TFieldPtr m_win_count;    //星星抽奖累计赢钱局数 

	Tfield<int32_t>::TFieldPtr m_nChairID;    

	Tfield<int32_t>::TFieldPtr m_nPlayerStatus;  

	Tfield<int32_t>::TFieldPtr m_nPlayRound;  

	Tfield<int32_t>::TFieldPtr m_nWaitRound;

    Tfield<int32_t>::TFieldPtr		m_nTotalInScore;		//鬥地主积分;
	//////////////////////////////////////////////////////////////////////////
	void create_player();
	bool load_player();	
public:
	virtual void init_game_object();//注册属性
	virtual bool store_game_object(bool to_all = false);//非数组对象必须实现此接口

};

LANDLORD_SPACE_END
