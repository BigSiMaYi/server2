#pragma once

#include <db_base.h>
#include <enable_singleton.h>

//////////////////////////////////////////////////////////////////////////
static const std::string unknown_table = "DefaultTable";

static const std::string DB_DRAGON_TIGER_PLAYER = "dragon_tiger_player";
static const std::string DB_PLAYER_INDEX = "player_id";
static const std::string DB_DRAGON_TIGER_ROOM = "dragon_tiger_room";
static const std::string DB_DRAGON_TIGER_CARDS = "dragon_tiger_cards";
static const std::string DB_DRAGON_TIGER_ROOMCFG = "dragon_tiger_roomcfg";
//Íæ¼ÒÊý¾Ý¿â
class db_game : public db_base
	, public enable_singleton<db_game>
{
public:
	db_game();
	virtual ~db_game();
	virtual void init_index();
};