#pragma once
#include "logic_def.h"
#include <i_game_phandler.h>
#include "i_game_player.h"

COWS_SPACE_BEGIN

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
	virtual void quest_change_from_world(int quest_type,int count,int param);

	//玩家掉线
	virtual void on_change_state();

	//////////////////////////////////////////////////////////////////////////
	//离开游戏时调用
	void enter_game(logic_lobby* lobby);

	logic_room* get_room();
	bool enter_room(logic_room* room);
	void leave_room();
	int can_leave_room();

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
	//广播
	void bc_game_msg(int money, const std::string& sinfo, int mtype = 1);

	bool check_bet_gold(GOLD_TYPE bet_gold, GOLD_TYPE total_gold);
	void add_star_lottery_info(int32_t award,int32_t star = 0);
	void quest_change(int questid, int count=1, int param=0);

	void set_self_is_bet(bool is_bet){m_self_is_bet=is_bet;}
	bool get_self_is_bet(){return m_self_is_bet;}

	void set_self_win_gold(GOLD_TYPE win_gold){m_self_win_gold=win_gold;}
	GOLD_TYPE get_self_win_gold(){return m_self_win_gold;}

	//更新税收，闲家手续费: 每个下注区域输赢总和后扣税;
	void  set_tax(GOLD_TYPE tax);
	//开局记录当前玩家身上金币;
	GOLD_TYPE get_pre_gold();
	void record_gold();
	GOLD_TYPE get_tax();
	void  set_kick_status(int bforce);
	int get_kick_status();
		
	void set_otherplayers(std::vector<LPlayerPtr> other_players)
	{
		m_other_players.clear();
		m_other_players=other_players;
	}
	std::vector<LPlayerPtr>& get_otherplayers(){return m_other_players;}

	template<class T>
	int send_msg_to_client(T msg)
	{
		return m_player->send_msg_to_client(msg);
	};
private:
	logic_lobby* m_lobby;
	logic_room* m_room;

	LRobotPtr m_robot;
	GOLD_TYPE m_pre_logic_gold; //开局时身上的钱;
	GOLD_TYPE m_logic_gold;	
	GOLD_TYPE m_change_gold;
	GOLD_TYPE m_tax_gold;

	bool m_self_is_bet;
	GOLD_TYPE m_self_win_gold;

	std::vector<LPlayerPtr> m_other_players;

	Tfield<int32_t>::TFieldPtr m_win_count;    //星星抽奖累计赢钱局数   
	int m_bforce;

	//////////////////////////////////////////////////////////////////////////
	void create_player();
	bool load_player();	
public:
	virtual void init_game_object();//注册属性
	virtual bool store_game_object(bool to_all = false);//非数组对象必须实现此接口

	void on_turret_change();
};

COWS_SPACE_END