#pragma once
#include "cards_def.h"
#include "logic2logsvr_msg_type.pb.h"
#include <memory>
namespace red_black_protocols
{
	class msg_result_info;
	class msg_cards_info;
	class msg_scene_info;
}

struct cards_trend //牌路走势;
{
	cards_trend()
	{
		memset(this, 0, sizeof(*this));
	}
	int m_win_area;
	int m_card_type;
	int m_is_win[3];
};

DRAGON_RED_BLACK_BEGIN

class logic_main
{
public:
	logic_main(logic_room* room);
	~logic_main(void);

public:
	enum class game_state
	{
		game_state_unknown,	//未开始;
		game_state_prepare,	//准备,处理上庄;
		game_state_bet,		//下注;
		game_state_deal,	//发牌;
		game_state_result,	//结果;
	};

public:
	void heartbeat(double elapsed);

	void stop_game();//游戏没人等，暂停游戏;
	
	void pre_start_game();//准备开始游戏;
	
	void start_game();//开始下一局,清空数据;
	void clear_last_data();
	
	int   bet_gold(LPlayerPtr& player, uint32_t index, GOLD_TYPE gold);//下注;
	void clear_bet_gold(LPlayerPtr& player);//清空下注;
	GOLD_TYPE get_player_betgold(LPlayerPtr& player);//获取玩家下注金额;
	
	void add_player_betgold(LPlayerPtr& player, GOLD_TYPE gold);//增加玩家下注金额;
	void clear_player_betgold(LPlayerPtr& player);//清空玩家下注金额;
	void sync_bet_gold();//同步下注金额;

	void deal_cards();//发牌;
	
	void computer_result();//计算结果;

	int   get_inc_id();//序号;

	bool can_leave_room(uint32_t player_id);

	void leave_room(LPlayerPtr& player);

	double  get_room_commission_rate();

	void fill_scene_info(red_black_protocols::msg_scene_info* cards_info);
	void fill_result_info(LPlayerPtr& player, red_black_protocols::msg_result_info* result_info);

public:
	logic_room* get_room();
	std::vector<logic_other*>& get_others();

	game_state get_game_state();
	float get_duration();
	int get_cd_time();

	std::vector<GOLD_TYPE> get_robot_bet_area_gold();
	std::vector<float>             calc_bet_area_rate();

	void kill_points(int32_t cutRound, bool status);
	void service_ctrl(int32_t optype);
	bool cut_round_check(int cut_round);
	void server_stop();

	int get_cut_round_tag();
protected:
	void set_game_state(game_state state);
	void exe_cut_round();

	void check_player_status(bool server_stop);

	void fill_cards_info(red_black_protocols::msg_cards_info* cards_info, std::vector<poker>& poke, int card_type);
	void fill_cards_info2(logic2logsvr::BetAreaCardsInfo* cards_info, std::vector<poker>& poke, int card_type, int pos, GOLD_TYPE bet);

	//计算所有玩家税收;
	void  calc_player_rax();
	int     get_poker_rate(cards_golden_type win_golden_type, int poker_value);

private:
	logic_room* m_room;

	int m_inc_id;
	game_state m_game_state;
	float m_duration;
	float m_sync_eleased;
	float m_sync_interval;

	bool m_change_bet;

	time_t m_log_time;
	std::vector<logic_other*> m_others;//闲家;
	std::map<uint32_t, GOLD_TYPE> m_bet_players;//下注金额;

	std::list<cards_trend> m_history_infos;   //牌路，用于计算机器人下注区域;

	logic_poker_mgr* m_poker_mgr;//发牌管理;
	std::vector<poker> m_red_pokers;
	std::vector<poker> m_black_pokers;
	std::vector<int> m_win_area;
	int m_card_type;
	cards_golden_type m_red_type;
	cards_golden_type m_black_type;

	std::shared_ptr<logic2logsvr::RedBlackGameLog> m_cows_log;
	int32_t m_service_status;
	int m_cutroudtag;
	bool m_kill_points_switch;
	int m_cfgCutRound;
	int m_gametimes;
	int m_cuttimes;
	int m_cutround_tag;
};

DRAGON_RED_BLACK_END