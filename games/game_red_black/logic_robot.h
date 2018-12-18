#pragma once
#include "logic_def.h"
#include <i_game_phandler.h>
#include "i_game_player.h"

struct RedBlack_RoomCFGData;

DRAGON_RED_BLACK_BEGIN

class logic_robot : public enable_obj_pool<logic_robot>
{
public:
	logic_robot(void);
	virtual ~logic_robot(void);

public:
	void init(logic_player* player);

	void heartbeat(double elapsed);

	bool need_exit();//机器人需要退出;
	void set_max_bet_gold(double rate);

protected:
	void bet();
	void bc_bet_info(logic_player* player, int index, GOLD_TYPE gold, int ret);
	int    calc_bet_index(GOLD_TYPE betgold);
	
private:
	logic_player* m_player = nullptr;
	double m_life_time = 100;
	double m_interval = 1;

	int32_t m_base_bet = 100;//下注基数;
	GOLD_TYPE m_max_bet_gold = 0; //下局最大下注金额;
};

DRAGON_RED_BLACK_END
