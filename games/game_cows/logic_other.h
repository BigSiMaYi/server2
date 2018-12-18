#pragma once
#include "logic_def.h"

COWS_SPACE_BEGIN

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
	GOLD_TYPE get_win_gold(uint32_t pid, bool& is_bet);

	void win(GOLD_TYPE& player_gold, GOLD_TYPE& robot_gold, GOLD_TYPE& bet_gold);
	void lose(int cards_rate, GOLD_TYPE& player_gold, GOLD_TYPE& robot_gold, GOLD_TYPE& bet_gold);
	bool is_win();

	logic_cards* get_cards();
	void set_cards(logic_cards* cards);
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

COWS_SPACE_END