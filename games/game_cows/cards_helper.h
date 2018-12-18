#pragma once
#include "cards_def.h"
#include <enable_singleton.h>

COWS_SPACE_BEGIN

//�����ƴ�С
class cards_helper : public enable_singleton<cards_helper>
{
public:
	cards_helper();
	virtual ~cards_helper();
	//����ţ��
	void computer_cow_cards(const std::vector<poker>& pokers, std::vector<int>& match_pokers, cards_type& type, int& poker_value);
	//��ȡ������
	int get_max_poker_value(const std::vector<poker>& pokers);
protected:
	//��Сţ
	bool computer_small_cow(const std::vector<poker>& pokers, cards_type& type, int& poker_value);
	//��ţ
	bool computer_gold_cow(const std::vector<poker>& pokers, cards_type& type, int& poker_value);
	//��ը
	bool computer_bomb(const std::vector<poker>& pokers, cards_type& type, int& poker_value);
	//��ţ
	bool computer_silver_cow(const std::vector<poker>& pokers, cards_type& type, int& poker_value);
	//ţ��
	bool computer_cow(const std::vector<poker>& pokers, std::vector<int>& match_pokers, cards_type& type, int& poker_value);
	//ûţ��
	void computer_no_cow(const std::vector<poker>& pokers, cards_type& type, int& poker_value);
private:
	std::vector<std::vector<int>> m_cow_checks;
};

COWS_SPACE_END