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
	ensure_index(DB_ROBCOWS_PLAYER, BSON("player_id"<<1), "_player_id_");
	ensure_index(DB_ROBCOWSROOM, BSON("room_id"<<1), "_room_id_");
}
