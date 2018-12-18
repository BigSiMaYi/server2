#pragma once

#include <db_base.h>
#include <enable_singleton.h>

//////////////////////////////////////////////////////////////////////////
static const std::string unknown_table = "DefaultTable";

static const std::string DB_ROBCOWS_PLAYER = "robcows_player";
static const std::string DB_PLAYER_INDEX = "player_id";
static const std::string DB_ROBCOWSROOM = "robcows_room";
static const std::string DB_ROBCOWSCARDS = "robcows_cards";

static const std::string DB_ROBCOWSTABLE = "robcows_table";

//Íæ¼ÒÊý¾Ý¿â
class db_game : public db_base
	, public enable_singleton<db_game>
{
public:
	db_game();
	virtual ~db_game();
	virtual void init_index();
};