#include "stdafx.h"
#include "logic_poker_mgr.h"
#include "logic_cards.h"
#include "cards_def.h"
#include "game_db.h"
#include "DragonTiger_WeightCFG.h"

DRAGON_TIGER_SPACE_USING

logic_poker_mgr::logic_poker_mgr(int seed)
	:m_random(seed)
{
	m_pokers.reserve(POKERCOUNT);
	
	//8付牌随机
	for (int pokes_pairs = 0;  pokes_pairs < 8; pokes_pairs++)
	{
		for (int i = 0; i < POKERCOUNT; i++)
		{
			poker_type type = (poker_type)(i/POKERTYPECOUNT);
			int value = i%POKERTYPECOUNT + 1;

			m_pokers.push_back(poker(type, value));
		}
	}

	for (int i = 0; i < 51 * 8 ; i++)
	{
		rand_swap_poker(i);
	}
}

logic_poker_mgr::~logic_poker_mgr(void)
{
}

void logic_poker_mgr::rand_swap_poker(int i)
{
	if (i < POKERCOUNT * 8-1)
	{
		int rand_value = m_random.rand_int(i+1, POKERCOUNT * 8 - 1);
		std::swap(m_pokers[i], m_pokers[rand_value]);
	}
}

void logic_poker_mgr::swap_poker(int i, int j)
{
	if (i < POKERCOUNT-1 && j < POKERCOUNT-1 )
	{
		std::swap(m_pokers[i], m_pokers[j]);
	}
}


const poker& logic_poker_mgr::get_poker(int index)
{
	return m_pokers[index];
}

void logic_poker_mgr::rand_shuffle()
{
	for (int i = 0; i < 2; i++)
	{
		rand_swap_poker(i);
	}
}

void dragon_tiger_space::logic_poker_mgr::swap_set_poker_inequality()
{
	
	for (int i = 2; i < m_pokers.size(); i++)
	{
		if (m_pokers[1].m_poker_point != m_pokers[i].m_poker_point )
		{
			std::swap(m_pokers[1], m_pokers[i]);

			break;
		}
		
	}

}
void logic_poker_mgr::pre_analysis_swap_pokes( int current_water, int record_index)
{
	//不触发个控直接反回
	int robot_random_count = 0;
	int player_random_count = 0;
	if ( !is_trigger_control( current_water ) ) return;
	const poker& poker_dragon = get_poker(0);
	const poker& poker_tiger =  get_poker(1);
	if (poker_dragon.m_poker_point == poker_tiger.m_poker_point)
	{
		swap_set_poker_inequality();
	}

	const poker& poker_dragon_swap = get_poker(0);
	const poker& poker_tiger_swap =  get_poker(1);
	//压龙的玩家多，  让龙盘输
	if (record_index == 1)
	{
		if (poker_dragon.m_poker_point > poker_tiger.m_poker_point)
		{
			std::swap(m_pokers[0], m_pokers[1]);
		}
	}

	//压虎的最多， 让虎盘输
	if (record_index == 2)
	{
		if (poker_tiger.m_poker_point > poker_dragon.m_poker_point)
		{
			std::swap(m_pokers[0], m_pokers[1]);
		}
	}


}

//根据水位值随机出来权重值
bool logic_poker_mgr::is_trigger_control(int current_water )
{
	boost::unordered_map<int, DragonTiger_WeightCFGData>& synDatas = DragonTiger_WeightCFG::GetSingleton()->GetMapData();

	for (auto it = synDatas.begin(); it != synDatas.end(); ++it)
	{
		if (it->second.mIndex == current_water)
		{
			if (it->second.mTriggerControl == 0)
			{
				return false;
			}

			int value = it->second.mKillScoreRate; 
			int rand_value = global_random::instance().rand_int(0,100);
			if (rand_value <= value)
			{
				return true;
			}
		
		}
	}

	return false;
}


