#pragma once
#include "logic_def.h"
#include <i_game_phandler.h>
#include "i_game_player.h"

DRAGON_TIGER_SPACE_BEGIN

#define PROPERTY_DEFINE(type, name, access_permission)\
access_permission:\
	type m_##name;\
	public:\
	inline void set_##name(type v) {\
	m_##name = v;\
}\
	inline type get_##name() {\
	return m_##name;\
}\

enum quest_type
{
	game_count = 401,
	banker_win_count,
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
	void					parse_nickname();		//昵称扩展pb的解析
	//玩家掉线
	virtual void on_change_state();

	//////////////////////////////////////////////////////////////////////////
	//离开游戏时调用
	void enter_game(logic_lobby* lobby);

	logic_room* get_room();
	bool enter_room(logic_room* room);
	void leave_room();
	bool can_leave_room();

	void release();//退出整个游戏

	bool is_offline();
	uint32_t get_pid();
	//获取玩家当前金币(游戏调用)
	GOLD_TYPE get_gold();
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
	//是否机器人
	bool is_robot();
	//机器人逻辑
	LRobotPtr& get_robot();
	//创建机器人逻辑
	void create_robot();
	std::string m_nickname;				//昵称

	//改变金币(游戏调用)
	bool change_gold(GOLD_TYPE v, bool needbc = false);
	//改变礼券(游戏调用)
	bool change_ticket(int v, int season);
	bool change_gold2(int v, int season);
	//同步金额
	void sync_change_gold();

	void store_balance_record();
	//广播
	void bc_game_msg(int money, const std::string& sinfo, int mtype = 1);

	bool check_bet_gold(GOLD_TYPE bet_gold, GOLD_TYPE total_gold);
	void add_star_lottery_info(int32_t award,int32_t star = 0);
	void quest_change(int questid, int count=1, int param=0);

	template<class T>
	int send_msg_to_client(T msg)
	{
		return m_player->send_msg_to_client(msg);
	};

	//反回近20局的总押注金币
	GOLD_TYPE get_bet_gold();
	//反回近20局的总赢的局数
	int      get_winner_count();

	void  set_kick_status(int bforce) { m_bforce = bforce; }
	int     get_kick_status() { return m_bforce; }
	void  set_leave_status(int leave_status) { m_leave_status = 0; }
	int     get_leave_status() { return m_leave_status; }
private:
	logic_lobby* m_lobby;
	logic_room* m_room;

	LRobotPtr m_robot;

	GOLD_TYPE m_logic_gold;	
	GOLD_TYPE m_change_gold;
	GOLD_TYPE m_tax_gold;
	int      m_record_banker_count;

	Tfield<int32_t>::TFieldPtr m_win_count;    //星星抽奖累计赢钱局数   

	//////////////////////////////////////////////////////////////////////////
	void create_player();
	bool load_player();	
public:
	virtual void init_game_object();//注册属性
	virtual bool store_game_object(bool to_all = false);//非数组对象必须实现此接口

	void on_turret_change();

	virtual void quest_change_from_world(int quest_type,int count,int param);

	int get_roomid();

	std::list<GOLD_TYPE> & get_bet_gold_list();
	std::list<bool>& get_winner_list();

	void clear_balace_info();

	PROPERTY_DEFINE( GOLD_TYPE,   balance_bet_gold,		private );//单局总共压的钱
	PROPERTY_DEFINE( int,			balance_win_or_lose,	private );//单局是赢还是输
	PROPERTY_DEFINE( int,			win_lose_final_gold,	private );//单局最终赢的钱
	PROPERTY_DEFINE( bool,		balance_bet,			private );//本局是不是压注
private:
	std::list<GOLD_TYPE>		m_list_bet_gold;
	std::list<bool>			m_list_winner;
	int m_leave_status;
	int m_bforce;
};

DRAGON_TIGER_SPACE_END
