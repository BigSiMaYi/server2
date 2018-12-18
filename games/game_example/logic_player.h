
#pragma once
#include "logic_def.h"
#include <i_game_phandler.h>
#include <i_game_player.h>

EXAMPLE_SPACE_BEGIN


class logic_player: 
	public enable_obj_pool<logic_player>
	,public i_game_phandler
	,public game_object
{
public:
	logic_player(void);
	virtual ~logic_player(void);

	void heartbeat( double elapsed );

	//////////////////////////////////////////////////////////////////////////
	//从服务器通知逻辑的接口
	//属性
	virtual void on_attribute_change(int atype, int v);

	virtual void on_attribute64_change(int atype, GOLD_TYPE v = 0) {};
	//玩家掉线
	virtual void on_offline();

	virtual void on_change_state(void){};

	//////////////////////////////////////////////////////////////////////////
	//离开游戏时调用
	void enter_game(logic_lobby* lobby);
	bool join_table(logic_table* table);
	void leave_table();
	void release();//退出整个游戏

	uint32_t get_pid();
	//获取玩家当前金币(游戏调用)
	int get_gold();
	//获取玩家VIP等级
	int16_t get_viplvl();

	//是否体验VIP
	bool is_ExperienceVIP();

	//获取礼券
	int get_ticket();

	//改变金币(游戏调用)
	bool change_gold(int v, bool needbc = false);

	//改变礼券(游戏调用)
	bool change_ticket(int v, bool needbc = false);

	//获取昵称
	const std::string& get_nickname();


	int get_sex();
	int get_photo_frame();
	const std::string& get_icon_custom();

	logic_table* get_table();

	void addexp(int exp);

	void bc_game_msg(int money, const std::string& sinfo, int mtype = 1);

	void init_tickettime();
	int check_ticket();//检测每日送礼券


	template<class T>
	int send_msg_to_client(T msg)
	{
		return m_player->send_msg_to_client(msg->packet_id(), msg);
	};
private:
	logic_table* m_table;
	logic_lobby* m_lobby;

	int32_t m_logic_gold;	
	int32_t m_change_gold;
	int32_t m_log_gold;
	double m_check_sync;
	double m_check_ticket;
	double m_cur_tickettime;

	bool m_isRobot;

	//////////////////////////////////////////////////////////////////////////
	void create_player();
	bool load_player();	
public:
	virtual void init_game_object();//注册属性
	virtual bool store_game_object(bool to_all = false);//非数组对象必须实现此接口
		
	Tfield<int32_t>::TFieldPtr		KillFishCount;		//杀鱼数
	Tfield<int32_t>::TFieldPtr		KillFishScore;		//杀鱼积分
	Tfield<int16_t>::TFieldPtr		Level;				//等级
	Tfield<int32_t>::TFieldPtr		Exp;				//经验

	Tfield<int32_t>::TFieldPtr		TodayTicket;			//今日获取礼券
	Tfield<time_t>::TFieldPtr		LastGetTicket;			//最后获取礼券日期
	Tfield<bool>::TFieldPtr			ReceiveTicket;			//首次礼券
	//////////////////////////////////////////////////////////////////////////
};
EXAMPLE_SPACE_END
