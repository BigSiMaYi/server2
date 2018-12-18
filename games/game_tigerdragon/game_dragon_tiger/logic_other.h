#pragma once
#include "logic_def.h"

DRAGON_TIGER_SPACE_BEGIN


	//闲家
class logic_other
{
public:
	logic_other(logic_main* main);
	~logic_other(void);

	//开始下一局游戏
	void start_game();

	//下注
	void bet_gold(uint32_t pid, GOLD_TYPE gold);
	void clear_bet_gold(uint32_t pid);

	GOLD_TYPE get_total_bet_gold();
	GOLD_TYPE get_bet_gold(uint32_t pid);

	GOLD_TYPE get_total_win_gold();
	GOLD_TYPE get_win_gold(uint32_t pid);

	bool is_win();
	void win_dragon_tiger(logic_room* room, float tax, float winner_odds);
	void lose_dragon_tiger(logic_room* room, float tax, float lose_odds );
	//获取机器人押注金额
	GOLD_TYPE  get_player_bet_gold_panel();

	logic_cards* get_cards();
private:
	typedef ENABLE_MAP<uint32_t, GOLD_TYPE> LBET_MAP;
	//每个玩家下注金额
	LBET_MAP m_bet_players;
	//总下注金额
	int64_t m_bet_gold;

	typedef ENABLE_MAP<uint32_t, GOLD_TYPE> LWIN_MAP;
	//每个玩家输赢
	LWIN_MAP m_win_players;
	//总输赢
	GOLD_TYPE m_win_gold;
	//输赢
	bool m_is_win;
	//庄家的牌
	logic_cards* m_cards;
	//
	logic_main* m_main;
};

DRAGON_TIGER_SPACE_END
