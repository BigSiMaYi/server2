#pragma once
#include "cards_def.h"
#include <enable_random.h>

COWS_SPACE_BEGIN

class logic_poker_mgr
{
public:
	logic_poker_mgr(int seed);
	~logic_poker_mgr(void);

	//随机洗牌
	void rand_shuffle();
	//玩家随机洗牌 开始牌，数量
	void player_rand_shuffle(int playerIndex);

	//庄家必出牛牌
	void banker_power_shuffle();
	//闲家必出牛牌
	void other_power_shuffle();

	//庄家必赢洗牌
	void banker_win_shuffle();

	//GM洗牌
	bool check_gm_shuffle();

	void gm_shuffle(int banker_cards, int other_cards1, int other_cards2, int other_cards3, int other_cards4);
	void shuffle(int playerIndex, int cards);

	//发牌
	void deal(logic_cards* cards);
protected:
	void rand_swap_poker(int i);
	void swap_poker(int i, int j);
	void swap_player_poker(int player_index);
	const poker& get_poker(int index);

	void small_suffle(int player_index);
	void gold_cows_suffle(int player_index);
	void silver_cows_suffle(int player_index);
	void bomb_cows_suffle(int player_index);
	void cows_N_suffle(int player_index, int cows_n);
	void cows_0_suffle(int player_index);
private:
	int m_start_index;
	std::vector<poker> m_pokers;

	enable_random<boost::rand48> m_random;
};

COWS_SPACE_END