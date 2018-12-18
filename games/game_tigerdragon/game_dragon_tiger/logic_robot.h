#pragma once
#include "logic_def.h"
#include <i_game_phandler.h>
#include "i_game_player.h"

struct DragonTiger_RoomCFGData;

DRAGON_TIGER_SPACE_BEGIN

class logic_robot: 
	public enable_obj_pool<logic_robot>
{
public:
	logic_robot(void);
	virtual ~logic_robot(void);

	void heartbeat(double elapsed);

	void init(logic_player* player);

	void set_max_bet_gold(GOLD_TYPE bet_gold);
	//机器人需要退出
	bool need_exit();

protected:
	void bet();
private:
	logic_player* m_player;
	double m_life_time;

	int32_t m_banker_count;
	//下局最大下注金额
	GOLD_TYPE m_max_bet_gold;
	//下注基数
	int32_t m_base_bet;

	double m_interval;
	const DragonTiger_RoomCFGData* m_roomcfg;
};

DRAGON_TIGER_SPACE_END
