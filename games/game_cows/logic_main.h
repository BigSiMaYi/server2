#pragma once
#include "cards_def.h"

namespace cows_protocols
{
	class msg_result_info;
}
namespace logic2logsvr
{
	class CowsGameLog;
}
COWS_SPACE_BEGIN

class logic_main
{
public:
	logic_main(logic_room* room);
	~logic_main(void);

	void heartbeat(double elapsed);

	void test_game();
	//游戏没人等，暂停游戏
	void stop_game();
	//准备开始游戏
	void pre_start_game();
	//开始下一局,清空数据
	void start_game();
	void clear_last_data();
	//下注
	int bet_gold(LPlayerPtr& player, uint32_t index, GOLD_TYPE gold);
	//清空下注
	void clear_bet_gold(LPlayerPtr& player);

	//获取玩家下注金额
	GOLD_TYPE get_player_betgold(LPlayerPtr& player);
	//增加玩家下注金额
	void add_player_betgold(LPlayerPtr& player, GOLD_TYPE gold);
	//清空玩家下注金额
	void clear_player_betgold(LPlayerPtr& player);

	//同步下注金额
	void sync_bet_gold();
	//发牌
	void deal_cards();
	//计算结果
	void computer_result();
	//申请上庄
	int apply_banker(LPlayerPtr& player);
	//取消申请上庄
	int cancel_apply_banker(LPlayerPtr& player);
	//清空上庄申请
	void clear_apply_banker(LPlayerPtr& player);

	std::list<LPlayerPtr>& get_apply_bankers();

	int get_robot_banker_size();
	//序号
	int get_inc_id();

	//可以抢庄
	bool can_snatch();
	int get_snatch_gold();
	uint32_t get_snatch_player();
	//抢庄
	int snatch_banker(LPlayerPtr& player, int gold);
	//申请下庄
	int ask_leave_banker(LPlayerPtr& player, bool force, int32_t& cost_ticket);
	//牌路
	void log_history_cards();
	struct total_history_info
	{
		int m_total_win[4];
		int m_total_lose[4];
	};
	struct history_info
	{
		bool m_is_win[4];
	};

	int64_t get_history_log_time();
	int get_history_total_count();
	const total_history_info& get_total_history_info();
	std::list<history_info>& get_history_infos();

	bool can_leave_room(uint32_t player_id);
	//玩家离开房间
	void leave_room(LPlayerPtr& player);

	void fill_result_info(LPlayerPtr& player, cows_protocols::msg_result_info* result_info);

	void update_star_gold();
	double  get_room_commission_rate();
public:
	//判断庄家
	bool check_banker();
	void clear_history();
	void init_history(GIntListFieldPtr& history, time_t log_time);
public:
	logic_room* get_room();
	logic_banker* get_banker();
	std::vector<logic_other*>& get_others();
	enum class game_state
	{
		game_state_unknown,	//未开始
		game_state_prepare,	//准备,处理上庄
		game_state_bet,		//下注
		game_state_deal,	//发牌
		game_state_result,	//结果
		game_state_pause,	//暂停;
	};
	game_state get_game_state();
	float get_duration();
	int get_cd_time();

	void kill_points(int32_t cutRound, bool status);
	void service_ctrl(int32_t optype);

protected:
	void set_game_state(game_state state);
	void add_snatch_player(LPlayerPtr& player, int ticket);
	void remove_snatch_player(LPlayerPtr& player);
	void clear_snatch_player();
	//计算所有玩家税收;
	void  calc_player_rax();
	LPlayerPtr getBesterPlayerPtr();
	void server_stop();
	void check_player_status(bool server_stop);
	//换牌;
	void swap_cards(std::vector<std::pair<int, logic_cards*> >& cards, int swap_type);
	//上一局的胜率：通杀-1，通赔1，正常0;
	int pre_game_win_rate();
	int cur_game_wind_rate();
	//检查玩家是否下注;
	bool check_bet(int32_t pid);
private:
	int m_inc_id;
	logic_room* m_room;

	game_state m_game_state;
	float m_duration;
	float m_sync_eleased;
	float m_sync_interval;

	//庄家
	logic_banker* m_banker;
	bool m_banker_killall;
	bool m_change_bet;
	//闲家
	std::vector<logic_other*> m_others;
	//记录机器人区间的下注;
	std::map<int, GOLD_TYPE> m_rebots_bet;
	//真人下注记录;
	std::map<int, GOLD_TYPE> m_player_bet;

	//庄家申请
	std::list<LPlayerPtr> m_apply_players;
	std::set<uint32_t> m_apply_pids;

	LPlayerPtr m_snatch_player;
	int m_snatch_gold;
	std::map<uint32_t, int32_t> m_snatch_player_ids;

	//下注金额
	std::map<uint32_t, GOLD_TYPE> m_bet_players;

	//牌路
	int m_total_count;
	time_t m_log_time;
	total_history_info m_total_history;
	std::list<history_info> m_history_infos;

	//发牌管理
	logic_poker_mgr* m_poker_mgr;

	//玩家和机器人上庄概率发牌概率;
	std::vector<float> m_player_banker_probs;
	std::vector<float> m_robot_banker_probs;
	bool m_kill_points_switch;
	int32_t m_service_status;
	std::shared_ptr<logic2logsvr::CowsGameLog> m_cows_log;
};

COWS_SPACE_END