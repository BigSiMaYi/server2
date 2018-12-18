#pragma once
#include "logic_def.h"
#include <i_game_def.h>


EXAMPLE_SPACE_BEGIN

class logic_lobby
{
public:
	logic_lobby(void);
	~logic_lobby(void);

	void init_game(int count);
	void release_game();
	void heartbeat( double elapsed );
	bool player_enter_game(iGPlayerPtr igplayer);
	void player_leave_game(uint32_t playerid);
	int player_join_friend_game(iGPlayerPtr igplayer, uint32_t friendid);
	const LROOM_MAP& get_rooms();

	int join_table(uint32_t pid, uint16_t rid, uint16_t tid = 0);	
	void leave_talbe(uint32_t pid);
	logic_table* get_player_table(uint32_t pid);
	LPlayerPtr& get_player(uint32_t pid);

	void response_robot(int32_t playerid, int tag);
private:
	void init_config();
	void init_protocol();
	LROOM_MAP m_rooms;
	LPLAYER_MAP m_all_players;

	bool m_init;
	int m_max_player;	
};



EXAMPLE_SPACE_END
