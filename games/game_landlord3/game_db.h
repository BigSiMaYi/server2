#pragma once

#include <db_base.h>
#include <enable_singleton.h>

//////////////////////////////////////////////////////////////////////////
static const std::string unknown_table = "DefaultTable";

static const std::string DB_LANDLORD_PLAYER = "landlord3_player";
static const std::string DB_PLAYER_INDEX = "player_id";
static const std::string DB_LANDLORD3ROOM = "landlord3_room";
static const std::string DB_COWSCARDS = "cows_cards";

static const std::string DB_LANDLORD3TABLE = "landlord3_table";

//Íæ¼ÒÊý¾Ý¿â
class db_game : public db_base
	, public enable_singleton<db_game>
{
public:
	db_game();
	virtual ~db_game();
	virtual void init_index();
};