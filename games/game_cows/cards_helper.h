#pragma once
#include "cards_def.h"
#include <enable_singleton.h>

COWS_SPACE_BEGIN

//计算牌大小
class cards_helper : public enable_singleton<cards_helper>
{
public:
	cards_helper();
	virtual ~cards_helper();
	//计算牛牌
	void computer_cow_cards(const std::vector<poker>& pokers, std::vector<int>& match_pokers, cards_type& type, int& poker_value);
	//获取最大的牌
	int get_max_poker_value(const std::vector<poker>& pokers);
protected:
	//五小牛
	bool computer_small_cow(const std::vector<poker>& pokers, cards_type& type, int& poker_value);
	//金牛
	bool computer_gold_cow(const std::vector<poker>& pokers, cards_type& type, int& poker_value);
	//四炸
	bool computer_bomb(const std::vector<poker>& pokers, cards_type& type, int& poker_value);
	//银牛
	bool computer_silver_cow(const std::vector<poker>& pokers, cards_type& type, int& poker_value);
	//牛牌
	bool computer_cow(const std::vector<poker>& pokers, std::vector<int>& match_pokers, cards_type& type, int& poker_value);
	//没牛牌
	void computer_no_cow(const std::vector<poker>& pokers, cards_type& type, int& poker_value);
private:
	std::vector<std::vector<int>> m_cow_checks;
};

COWS_SPACE_END