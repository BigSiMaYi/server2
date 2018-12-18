#include "stdafx.h"
#include "logic_cards.h"
#include <dragon_tiger_logic.pb.h>

#include "DragonTiger_CardsCFG.h"

DRAGON_TIGER_SPACE_USING

logic_cards::logic_cards()
{
}

logic_cards::~logic_cards(void)
{
}

void logic_cards::clear_cards()
{
	m_pokers.clear();

}

const std::vector<poker>& logic_cards::get_pokers()
{
	return m_pokers;
}
