#pragma once
#include "cards_def.h"
#include "dragon_tiger_logic.pb.h"
#include "logic2logsvr_msg_type.pb.h"

namespace dragon_tiger_protocols
{
	class msg_result_info;
}

DRAGON_TIGER_SPACE_BEGIN

class logic_main
{
public:
	logic_main(logic_room* room);
	~logic_main(void);

	void heartbeat(double elapsed);

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

	//序号
	int get_inc_id();
	time_t get_history_log_time();
	int get_history_total_count();

	bool can_leave_room(uint32_t player_id);
	//玩家离开房间
	void leave_room(LPlayerPtr& player);

	void fill_result_info(LPlayerPtr& player, dragon_tiger_protocols::msg_result_info* result_info);

	void update_star_gold();

	logic_poker_mgr* get_poker_manager() { return m_poker_mgr; }
	
public:
	logic_room* get_room();

	std::vector<logic_other*>& get_others();
	enum class game_state
	{
		game_state_unknown,	//未开始
		game_state_prepare,	//准备,处理上庄
		game_state_bet,		//下注
		game_state_deal,	//发牌
		game_state_result,	//结果
	};
	game_state get_game_state();
	float get_duration();
	int get_cd_time();

	void kill_points(int32_t cutRound, bool status);
	void service_ctrl(int32_t optype);
	bool cut_round_check();
	void server_stop();

	int get_cut_round_tag();

protected:
	void set_game_state(game_state state);
	void check_player_status(bool server_stop);
private:
	int m_inc_id;
	logic_room* m_room;

	std::shared_ptr<logic2logsvr::DragonTigerGameLog> m_dragon_tiger_log;
	game_state m_game_state;
	float m_duration;
	float m_sync_eleased;
	float m_sync_interval;

	bool m_change_bet;
	//闲家
	std::vector<logic_other*> m_others;

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

	logic_poker_mgr* m_poker_mgr;

	int32_t m_service_status;
	int m_cutroudtag;
	bool m_kill_points_switch;
	int m_cfgCutRound;
	int m_gametimes;
	int m_cuttimes;

	int m_winner_index;

	int m_cutround_tag;
};

DRAGON_TIGER_SPACE_END