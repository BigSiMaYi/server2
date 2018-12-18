#pragma once
#include "cards_def.h"
#include <enable_random.h>

DRAGON_RED_BLACK_BEGIN

class logic_poker_mgr
{
public:
	logic_poker_mgr(int seed);
	~logic_poker_mgr(void);

	//���ϴ��
	void rand_shuffle();
	void shuffle_total_pokes();
	const poker& get_poker(int index);
	
	void get_red_pokers(std::vector<poker> &vec_red_pokes);
	void get_black_pokers(std::vector<poker> &vec_black_pokes);
	
	//�ȽϺ�ڴ�С ����Ӯ�̼����ͣ��𻨣����ӵȣ�
	cards_golden_type compare_red_black_pokes(std::vector<poker> &vec_red_pokes, std::vector<poker> &vec_black_pokes,
										  cards_golden_type &red_type, cards_golden_type &black_type, win_camp &win_camp_, int &duizi_value);

	cards_golden_type get_red_black_pokes_type(std::vector<poker> &vec_pokes, std::vector<poker> &vec_poke_point);


protected:
	void rand_swap_poker(int i);
	void swap_poker(int i, int j);

private:
	std::vector<poker> m_pokers;
	enable_random<boost::rand48> m_random;
};

DRAGON_RED_BLACK_END
