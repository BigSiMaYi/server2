#pragma once
#include "cards_def.h"

namespace cows_protocols 
{
	class msg_cards_info;
}
namespace logic2logsvr
{
	class CowsAreaCardsInfo;
}

COWS_SPACE_BEGIN

class logic_cards
{
public:
	logic_cards(void);
	~logic_cards(void);

	//��ʼ����
	void init_cards(const std::vector<poker>& pokers, int poker_index);

	void clear_cards();

	const std::vector<poker>& get_pokers();

	int32_t get_cards_rate();

	cards_type get_cards_type();
	int get_cards_value();

	void fill_cards_info(cows_protocols::msg_cards_info* cards_info, logic2logsvr::CowsAreaCardsInfo* cards = nullptr);

	//�Ƚϴ�С ����true,С��false;
	static bool compare(const logic_cards* cards1, const logic_cards* cards2);
private:
	cards_type m_cards_type;	//������
	int m_cards_value;			//�����
	int32_t m_cards_rate;		//�Ʊ���

	std::vector<poker> m_pokers;
	std::vector<int> m_match_indexs;
};

COWS_SPACE_END