#pragma once
#include "logic_def.h"

COWS_SPACE_BEGIN

	//�м�
class logic_other
{
public:
	logic_other(logic_main* main);
	~logic_other(void);

	//��ʼ��һ����Ϸ
	void start_game();

	//��ע
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
	//ÿ�������ע���
	LBET_MAP m_bet_players;
	//����ע���
	int64_t m_bet_gold;

	typedef ENABLE_MAP<uint32_t, GOLD_TYPE> LWIN_MAP;
	//ÿ�������Ӯ
	LWIN_MAP m_win_players;
	//����Ӯ
	GOLD_TYPE m_win_gold;
	//��Ӯ
	bool m_is_win;
	//ׯ�ҵ���
	logic_cards* m_cards;
	//
	logic_main* m_main;
};

COWS_SPACE_END