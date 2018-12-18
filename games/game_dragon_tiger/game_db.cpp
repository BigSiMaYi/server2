#include "stdafx.h"
#include "game_db.h"


//////////////////////////////////////////////////////////////////////////
db_game::db_game()
{

}
db_game::~db_game()
{

}
void db_game::init_index()
{
	ensure_index(DB_DRAGON_TIGER_PLAYER, BSON("player_id"<<1), "_player_id_");
	ensure_index(DB_DRAGON_TIGER_ROOM, BSON("room_id"<<1), "_room_id_");
	ensure_index(DB_DRAGON_TIGER_ROOMCFG,		BSON("room_id"<<1), "_room_id_");

}
