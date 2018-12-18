#pragma once
#include "cards_def.h"
#include <enable_random.h>

DRAGON_TIGER_SPACE_BEGIN

class logic_poker_mgr
{
public:
	logic_poker_mgr(int seed);
	~logic_poker_mgr(void);

	//Ëæ»úÏ´ÅÆ
	void rand_shuffle();

	//Ï´8´ÎÅÆ
	void shuffle_total_pokes();
	void pre_analysis_swap_pokes( int max_index, bool  kill_point );
	bool  is_trigger_control( int current_water );
	const poker& get_poker(int index);
	void swap_set_poker_inequality();
protected:
	void rand_swap_poker(int i);
	void swap_poker(int i, int j);

private:
	std::vector<poker> m_pokers;
	enable_random<boost::rand48> m_random;
};

DRAGON_TIGER_SPACE_END
