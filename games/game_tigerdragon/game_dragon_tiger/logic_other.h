#pragma once
#include "logic_def.h"

DRAGON_TIGER_SPACE_BEGIN


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
	GOLD_TYPE get_win_gold(uint32_t pid);

	bool is_win();
	void win_dragon_tiger(logic_room* room, float tax, float winner_odds);
	void lose_dragon_tiger(logic_room* room, float tax, float lose_odds );
	//��ȡ������Ѻע���
	GOLD_TYPE  get_player_bet_gold_panel();

	logic_cards* get_cards();
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

DRAGON_TIGER_SPACE_END
