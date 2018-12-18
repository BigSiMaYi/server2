#include "stdafx.h"
#include "game_db_log.h"
#include "logic_player.h"
#include "time_helper.h"

EXAMPLE_SPACE_USING;

//////////////////////////////////////////////////////////////////////////
db_log::db_log()
{

}
db_log::~db_log()
{

}
void db_log::init_index()
{

}

const std::string& db_log::get_tablename(uint16_t table_type)
{
	static const std::string unknown_table = "DefaultTable";

	

	return unknown_table;
}


void db_log::property_log(logic_player* player, int gameId, int ptype, int addValue, int reason, const std::string& param)
{
	if(addValue == 0)
		return;

	mongo::BSONObjBuilder builder;
	builder.appendTimeT("genTime", time_helper::instance().get_cur_time());
	builder.append("playerId", player->get_pid());
	builder.append("gameId", gameId);
	builder.append("itemId", ptype);

	int newValue = 0;
	if(ptype == 1)
	{		
		newValue = player->get_gold();	
	}
	else if(ptype == 2)
	{
		newValue = player->get_ticket();
	}

	builder.append("oldValue", newValue-addValue);	
	builder.append("newValue", newValue);
	builder.append("addValue", addValue);

	builder.append("reason", reason);
	builder.append("param",param);

	push_insert(1,builder.obj());
}