#include "stdafx.h"
#include "cards_helper.h"


COWS_SPACE_USING

cards_helper::cards_helper()
{
	std::vector<int> check;
	check.push_back(0);
	check.push_back(1);
	check.push_back(2);
	check.push_back(3);
	check.push_back(4);
	m_cow_checks.push_back(check);

	check.clear();
	check.push_back(0);
	check.push_back(1);
	check.push_back(3);
	check.push_back(2);
	check.push_back(4);
	m_cow_checks.push_back(check);

	check.clear();
	check.push_back(0);
	check.push_back(1);
	check.push_back(4);
	check.push_back(2);
	check.push_back(3);
	m_cow_checks.push_back(check);

	check.clear();
	check.push_back(0);
	check.push_back(2);
	check.push_back(3);
	check.push_back(1);
	check.push_back(4);
	m_cow_checks.push_back(check);

	check.clear();
	check.push_back(0);
	check.push_back(2);
	check.push_back(4);
	check.push_back(1);
	check.push_back(3);
	m_cow_checks.push_back(check);

	check.clear();
	check.push_back(0);
	check.push_back(3);
	check.push_back(4);
	check.push_back(1);
	check.push_back(2);
	m_cow_checks.push_back(check);

	check.clear();
	check.push_back(1);
	check.push_back(2);
	check.push_back(3);
	check.push_back(0);
	check.push_back(4);
	m_cow_checks.push_back(check);

	check.clear();
	check.push_back(1);
	check.push_back(2);
	check.push_back(4);
	check.push_back(0);
	check.push_back(3);
	m_cow_checks.push_back(check);

	check.clear();
	check.push_back(1);
	check.push_back(3);
	check.push_back(4);
	check.push_back(0);
	check.push_back(2);
	m_cow_checks.push_back(check);

	check.clear();
	check.push_back(2);
	check.push_back(3);
	check.push_back(4);
	check.push_back(0);
	check.push_back(1);
	m_cow_checks.push_back(check);
}

cards_helper::~cards_helper()
{

}

int cards_helper::get_max_poker_value(const std::vector<poker>& pokers)
{
	int poker_value = 0;
	for (int i = 0; i < pokers.size(); i++)
	{
		if (pokers[i].m_poker_value > poker_value)
		{
			poker_value = pokers[i].m_poker_value;
		}
	}
	return poker_value;
}

void cards_helper::computer_cow_cards(const std::vector<poker>& pokers, std::vector<int>& match_indexs, cards_type& type, int& poker_value)
{
	if (computer_small_cow(pokers, type, poker_value))
		return;

	if (computer_gold_cow(pokers, type, poker_value))
		return;

	if (computer_bomb(pokers, type, poker_value))
		return;

	if (computer_silver_cow(pokers, type, poker_value))
		return;

	if (computer_cow(pokers, match_indexs, type, poker_value))
		return;

	computer_no_cow(pokers, type, poker_value);
}

bool cards_helper::computer_small_cow(const std::vector<poker>& pokers, cards_type& type, int& poker_value)
{
	int poker_score = 0;
	for (unsigned int i = 0; i < pokers.size(); i++)
	{
		poker_score += pokers[i].m_poker_score;
		if (pokers[i].m_poker_score >= 5)
		{
			return false;
		}
	}
	if (poker_score > 10)
	{
		return false;
	}
	else
	{
		type = cards_type::cards_small_cow;
		if (poker_score == 10)
		{
			poker_value = POKERCOUNT + 1;
		}
		else
		{
			poker_value = get_max_poker_value(pokers);
		}
		return true;
	}
}

bool cards_helper::computer_gold_cow(const std::vector<poker>& pokers, cards_type& type, int& poker_value)
{
	for (unsigned int i = 0; i < pokers.size(); i++)
	{
		if (pokers[i].m_poker_point <= 10)
		{
			return false;
		}
	}
	type = cards_type::cards_gold_cow;
	poker_value = get_max_poker_value(pokers);
	return true;
}

bool cards_helper::computer_bomb(const std::vector<poker>& pokers, cards_type& type, int& poker_value)
{
	for (unsigned int i = 0; i < 2; i++)
	{
		int poker_count = 1;
		for (unsigned int j = i + 1; j < pokers.size(); j++)
		{
			if (pokers[i].m_poker_point == pokers[j].m_poker_point)
			{
				poker_count++;
			}
		}
		if (poker_count == 4)
		{
			type = cards_type::cards_bomb;
			poker_value = pokers[i].m_poker_point;
			return true;
		}
	}
	return false;
}


bool cards_helper::computer_silver_cow(const std::vector<poker>& pokers, cards_type& type, int& poker_value)
{
	int count = 0;	//花牌数
	for (unsigned int i = 0; i < pokers.size(); i++)
	{
		if (pokers[i].m_poker_point < 10)
		{
			return false;
		}
		else
		{
			if (pokers[i].m_poker_point > 10)
			{
				count++;
			}
		}
	}
	if (count >= 4)
	{
		type = cards_type::cards_silver_cow;
		poker_value = get_max_poker_value(pokers);
		return true;
	}

	return false;
}

bool cards_helper::computer_cow(const std::vector<poker>& pokers, std::vector<int>& match_indexs, cards_type& type, int& poker_value)
{
	for (unsigned int i = 0; i < m_cow_checks.size(); i++)
	{
		int poker_score = 0;
		for (unsigned int j = 0; j < 3; j++)
		{
			poker_score += pokers[m_cow_checks[i][j]].m_poker_score;
		}
		//任意三张为10或10的倍数
		if (poker_score%10 == 0)
		{
			match_indexs.push_back(m_cow_checks[i][0]+1);
			match_indexs.push_back(m_cow_checks[i][1]+1);
			match_indexs.push_back(m_cow_checks[i][2]+1);

			poker_score = (pokers[m_cow_checks[i][3]].m_poker_score + pokers[m_cow_checks[i][4]].m_poker_score)%10;
			if (poker_score == 0)
			{
				poker_score = 10;
			}
			type = (cards_type)poker_score;
			poker_value = get_max_poker_value(pokers);
			return true;
		}
	}
	return false;
}

void cards_helper::computer_no_cow(const std::vector<poker>& pokers, cards_type& type, int& poker_value)
{
	type = cards_type::cards_cow_0;
	poker_value = get_max_poker_value(pokers);
}