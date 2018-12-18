#pragma once
#include "logic_def.h"


namespace cows_protocols
{
	class msg_player_info;
}

COWS_SPACE_BEGIN
	//庄家
class logic_banker
{
public:
	logic_banker(void);
	~logic_banker(void);

	void check_player_state();
	//开始下一局游戏
	void start_game();

	//改变金钱
	void win_gold(GOLD_TYPE gold, GOLD_TYPE tax);

	logic_cards* get_cards();
	void set_cards(logic_cards* cards);

	uint32_t get_player_id();
	LPlayerPtr& get_player();
	//名字
	std::string get_banker_name();
	//下注金额
	GOLD_TYPE get_max_bet_gold();
	//庄家金额
	GOLD_TYPE get_banker_gold();

	GOLD_TYPE get_cur_bet_gold();
	//判断庄家金额是否足够
	bool check_bet_gold(GOLD_TYPE gold);
	void bet_gold(GOLD_TYPE gold);
	void clear_bet_gold(GOLD_TYPE gold);

	void resetSystemBanker();

	//设置玩家庄家
	void set_player_banker(LPlayerPtr& player);
	bool is_banker(uint32_t player_id);
	//是否有庄家
	bool is_player_banker();

	bool is_banker_protect();
	//系统或机器人上庄
	bool is_system_banker();
	//上庄次数
	int get_banker_count();
	int get_min_banker_count();
	GOLD_TYPE get_total_win_gold();
	GOLD_TYPE get_last_win_gold();

	void fill_player_info(cows_protocols::msg_player_info* player_info);
	//申请下庄
	int force_leave();
	void apply_leave();
	bool is_apply_leave();

	//地域信息
	const std::string getBankerRegion();
private:
	//庄家的牌
	logic_cards* m_cards;

	//最大下注金额
	GOLD_TYPE m_max_bet_gold;
	//当前下注金额
	GOLD_TYPE m_cur_bet_gold;

	std::string m_system_name;
	GOLD_TYPE m_system_gold;

	int m_min_banker_count;
	int m_max_banker_count;
	float m_leave_banker_cost;

	int m_offline_count;
	bool m_apply_leave;
	LPlayerPtr m_player;
	int m_banker_count;			//上庄次数
	time_t m_start_time;		//开始庄家时间
	GOLD_TYPE m_start_gold;			//开始上庄金钱
	int64_t m_total_win_gold;
	GOLD_TYPE m_last_win_gold;
	GOLD_TYPE m_sys_lose_gold;		//爆庄系统支付金钱
};

COWS_SPACE_END