#include "stdafx.h"
#include "logic_cards.h"
#include "cards_helper.h"
#include <cows_logic.pb.h>

#include "Cows_CardsCFG.h"
#include "logic2logsvr_msg_type.pb.h"

COWS_SPACE_USING

logic_cards::logic_cards()
{
}

logic_cards::~logic_cards(void)
{
}

void logic_cards::init_cards(const std::vector<poker>& pokers, int poker_index)
{
	for (int i = 0; i < CARDSPOKERCOUNT; i++)
	{
		m_pokers.push_back(pokers[poker_index + i]);
	}

	m_cards_type = cards_type::cards_cow_0;
	m_cards_value = 0;

	m_match_indexs.clear();
	cards_helper::instance().computer_cow_cards(m_pokers, m_match_indexs, m_cards_type, m_cards_value);

	auto cardsCFGData = Cows_CardsCFG::GetSingleton()->GetData((int)m_cards_type);
	assert(cardsCFGData);
	m_cards_rate = cardsCFGData->mCardsRate;
}

void logic_cards::clear_cards()
{
	m_pokers.clear();

	m_cards_type = cards_type::cards_cow_0;
	m_cards_value = 0;
}

const std::vector<poker>& logic_cards::get_pokers()
{
	return m_pokers;
}

bool logic_cards::compare(const logic_cards* cards1, const logic_cards* cards2)
{
	if (cards1->m_cards_type == cards2->m_cards_type)
	{
		return cards1->m_cards_value > cards2->m_cards_value;
	}
	else
	{
		return (int)cards1->m_cards_type > (int)cards2->m_cards_type;
	}
}

int32_t logic_cards::get_cards_rate()
{
	return m_cards_rate;
}

cards_type logic_cards::get_cards_type()
{
	return m_cards_type;
}

int logic_cards::get_cards_value()
{
	return m_cards_value;
}

void logic_cards::fill_cards_info(cows_protocols::msg_cards_info* cards_info, logic2logsvr::CowsAreaCardsInfo* cards)
{
	cards_info->set_cards_type((int)get_cards_type());
	cards_info->set_cards_value(m_cards_value);
	cards_info->mutable_pokers()->Reserve(CARDSPOKERCOUNT);
	if (cards)
	{
		auto type = get_cards_type();
		cards->set_card_type((logic2logsvr::cards_type)type);
	}
	
	for (int i = 0; i < CARDSPOKERCOUNT; i++)
	{
		if (cards)
		{
			int val = Covert(m_pokers[i]);
			cards->add_cards(val);
		}
		cards_info->add_pokers(m_pokers[i].m_id);
	}
	for (unsigned int i = 0; i < m_match_indexs.size(); i++)
	{
		cards_info->add_match_indexs(m_match_indexs[i]);
	}
}