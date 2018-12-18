#pragma once
#include "logic_def.h"

DRAGON_RED_BLACK_BEGIN

//�м�;
class logic_other
{
public:
	logic_other(logic_main* main);
	~logic_other(void);

public:
	void start_game();//��ʼ��һ����Ϸ;

	void bet_gold(uint32_t pid, GOLD_TYPE gold);//��ע;
	void clear_bet_gold(uint32_t pid);

	void win_red_black(logic_room* room, float tax, int winner_odds);
	void lose_red_black(logic_room* room, float tax, int lose_odds );
	void closing(int cards_rate, int symbol); //����;

	bool is_win();

	GOLD_TYPE get_total_bet_gold();
	GOLD_TYPE get_bet_gold(uint32_t pid);
	GOLD_TYPE get_total_win_gold();
	GOLD_TYPE get_win_gold(uint32_t pid, bool& is_bet);
	GOLD_TYPE get_player_bet_gold_panel();//��ȡ������Ѻע���;

private:
	logic_main* m_main;

	std::map<uint32_t, GOLD_TYPE> m_bet_players;//ÿ�������ע���;
	std::map<uint32_t, GOLD_TYPE> m_win_players;//ÿ�������Ӯ;
	
	int64_t m_bet_gold;//����ע���;
	GOLD_TYPE m_win_gold;//����Ӯ;
	bool m_is_win;//��Ӯ;
};

DRAGON_RED_BLACK_END
