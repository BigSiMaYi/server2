#pragma once

#include <enable_singleton.h>
#include <db_query.h>
#include "logic_def.h"

//日志表
enum e_db_log_table
{
	e_dlt_none = 0,
};

BMW_SPACE_BEGIN;

class logic_player;
//////////////////////////////////////////////////////////////////////////
//日志数据库
class db_log : public db_queue
	, public enable_singleton<db_log>
{
public:
	db_log();
	virtual ~db_log();
	virtual void init_index();

	virtual const std::string& get_tablename(uint16_t table_type);

	void property_log(logic_player* player, int gameId, int ptype, GOLD_TYPE addValue, GOLD_TYPE taxValue, int reason, const std::string& param = "");

	void dayMoneyStat(GOLD_TYPE addValue, int reason, int itemId);
	void playerBanker(logic_player* pl, time_t start_time, time_t end_time, int bankerCount,GOLD_TYPE beforeGold,GOLD_TYPE nowGold, GOLD_TYPE resultValue,GOLD_TYPE sysGet, GOLD_TYPE sysLose);

	//@add by Big O 2017/01/08;
	//在线游戏记录
	void joingame(int playerid, int roomid,int childid);
	void leavegame(int playerid);

private:
	void _recordCoinGrowth(int itemId, GOLD_TYPE addValue, logic_player* pl);
};

BMW_SPACE_END;
