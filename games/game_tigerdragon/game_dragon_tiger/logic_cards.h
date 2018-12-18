#pragma once
#include "cards_def.h"

namespace dragon_tiger_protocols 
{
	class msg_cards_info;
}

DRAGON_TIGER_SPACE_BEGIN

class logic_cards
{
public:
	logic_cards(void);
	~logic_cards(void);
	void clear_cards();
	const std::vector<poker>& get_pokers();

private:
	std::vector<poker> m_pokers;
};

DRAGON_TIGER_SPACE_END
