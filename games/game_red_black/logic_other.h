#pragma once
#include "logic_def.h"

DRAGON_RED_BLACK_BEGIN

//闲家;
class logic_other
{
public:
	logic_other(logic_main* main);
	~logic_other(void);

public:
	void start_game();//开始下一局游戏;

	void bet_gold(uint32_t pid, GOLD_TYPE gold);//下注;
	void clear_bet_gold(uint32_t pid);

	void win_red_black(logic_room* room, float tax, int winner_odds);
	void lose_red_black(logic_room* room, float tax, int lose_odds );
	void closing(int cards_rate, int symbol); //结算;

	bool is_win();

	GOLD_TYPE get_total_bet_gold();
	GOLD_TYPE get_bet_gold(uint32_t pid);
	GOLD_TYPE get_total_win_gold();
	GOLD_TYPE get_win_gold(uint32_t pid, bool& is_bet);
	GOLD_TYPE get_player_bet_gold_panel();//获取机器人押注金额;

private:
	logic_main* m_main;

	std::map<uint32_t, GOLD_TYPE> m_bet_players;//每个玩家下注金额;
	std::map<uint32_t, GOLD_TYPE> m_win_players;//每个玩家输赢;
	
	int64_t m_bet_gold;//总下注金额;
	GOLD_TYPE m_win_gold;//总输赢;
	bool m_is_win;//输赢;
};

DRAGON_RED_BLACK_END
