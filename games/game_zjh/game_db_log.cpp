#include "stdafx.h"
#include "game_db_log.h"
#include "logic_player.h"
#include "time_helper.h"
#include "game_engine.h"

ZJH_SPACE_USING;

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
	static const std::string GAMELOG = "pumpPlayerMoney";
	static const std::string COWSDAYLOG = "CowsEveryday";
	static const std::string COIN_GROWTH = "pumpCoinGrowth";
	static const std::string PLAYER_BANKER = "pumpCowsPlayerBanker";
	static const std::string TOTAL_CONSUME = "pumpTotalConsume";

	//@add by Big O 2017/01/08;
	//@在线标识;
	static const std::string ONLINE_GAME = "pumpOnlineGaming";

	switch ((e_db_log_table)table_type)
	{
	case 1:
		return GAMELOG;
		break;
	case 2:
		return COWSDAYLOG;
		break;
	case 4:
		return TOTAL_CONSUME;
		break;
	case 5:
		return PLAYER_BANKER;
		break;
	case 6:
		return COIN_GROWTH;
		break;

	//@在线标识;
	case 7:
		return ONLINE_GAME;
		break;

	default:
		break;
	}

	return unknown_table;
}

void db_log::property_log(logic_player* player, int gameId, int ptype, GOLD_TYPE addValue, GOLD_TYPE taxValue, int cutRound, int reason, const std::string& param /*= ""*/)
{
	if(addValue == 0)
		return;

	if(player->is_robot())
		return;

	mongo::BSONObjBuilder builder;
	builder.appendTimeT("genTime", time_helper::instance().get_cur_time());
	builder.append("playerId", player->get_pid());
	builder.append("gameId", gameId);
	builder.append("itemId", ptype);
	builder.append("roomId", player->get_roomid());

	GOLD_TYPE newValue = 0;
	if(ptype == 1)
	{		
		newValue = player->get_gold();	
	}
	else if(ptype == 2)
	{
		newValue = player->get_ticket();
	}
	_recordCoinGrowth(ptype, addValue, player);

	builder.append("oldValue", (long long )(newValue-addValue));	
	builder.append("newValue", (long long )newValue);
	builder.append("addValue", (long long )addValue);
	builder.append("taxValue", (long long)taxValue);
	builder.append("cutRound", cutRound);
	builder.append("reason", reason);
	builder.append("param",param);

	push_insert(1,builder.obj());

	dayMoneyStat(addValue, reason, ptype);
}


void db_log::dayMoneyStat(GOLD_TYPE addValue, int reason, int itemId)
{
	int changeType = 0;
	int64_t val = 0;
	if(addValue > 0)
	{
		changeType = 0; // 收入
		val = addValue;
	}
	else
	{
		changeType = 1; // 支出
		val = -addValue;
	}

	auto now = time_helper::instance().get_cur_date();
	time_t nt = time_helper::convert_from_date(now) * 1000;

	mongo::BSONObj cond = BSON("time" << mongo::Date_t(nt)
		<< "reason" << reason << "itemId" << itemId << "changeType" << changeType);
	db_log::instance().push_update(4, cond, BSON("$inc" << BSON("value" << (long long)val)));
}

void db_log::_recordCoinGrowth(int itemId, GOLD_TYPE addValue, logic_player* pl)
{
	//	if(addValue < 0)
	//		return;

	if(itemId == 1)
	{
		mongo::BSONObj cond = BSON("playerId" << pl->get_pid());
		int64_t v = addValue;
		db_log::instance().push_update(6, cond, BSON( "$inc" << BSON("gold" << (long long)v) ));

		db_log::instance().push_update(6, cond, BSON("$set" << BSON(
			"nickName" << pl->get_nickname() 
			<< "vipLevel" << pl->get_viplvl() )
			));
	}
}

void db_log::playerBanker(logic_player* pl, time_t start_time, time_t end_time, int bankerCount,GOLD_TYPE beforeGold,GOLD_TYPE nowGold, GOLD_TYPE resultValue,GOLD_TYPE sysGet, GOLD_TYPE sysLose)
{
	if(pl == nullptr)
		return;

	auto now = time_helper::instance().get_cur_time();

	mongo::BSONObjBuilder builder;
	builder.appendTimeT("genTime", now);
	builder.append("playerId", pl->get_pid());
	builder.append("playerName", pl->get_nickname());
	builder.append("startTime", (long long )start_time);
	builder.append("endTime", (long long )end_time);
	builder.append("bankerCount", bankerCount);
	builder.append("beforeGold",(long long ) beforeGold);
	builder.append("nowGold", (long long)nowGold);
	builder.append("resultValue", (long long)resultValue);
	builder.append("sysGet", (long long)sysGet);
	builder.append("sysLose", (long long)sysLose);

	db_log::instance().push_insert(5, builder.done());
}

void db_log::joingame(int playerid, int roomid)
{
	mongo::BSONObjBuilder builder;
	builder.append("playerId", playerid);
	builder.append("gameId", game_engine::instance().get_gameid());
	builder.append("roomId", roomid);
	push_update(7, BSON("playerId"<<playerid), BSON("$set" <<builder.obj()));
}

void db_log::leavegame(int playerid)
{
	push_delete(7, BSON("playerId"<<playerid));
}