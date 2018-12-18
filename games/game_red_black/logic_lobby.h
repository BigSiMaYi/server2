#pragma once
#include "logic_def.h"
#include <i_game_def.h>

DRAGON_RED_BLACK_BEGIN

class logic_lobby
{
public:
	logic_lobby(void);
	~logic_lobby(void);

	void init_game(int count, int room_id, int child_id);
	void release_game();
	void heartbeat( double elapsed );

	bool player_enter_game(iGPlayerPtr igplayer);
	void player_leave_game(uint32_t playerid);
	int   player_join_friend_game(iGPlayerPtr igplayer, uint32_t friendid);

	const LROOM_MAP& get_rooms();

	int enter_room(uint32_t pid, uint16_t rid);	
	void enter_room_nocheck(uint32_t pid, uint16_t rid);
	LPlayerPtr& get_player(uint32_t pid);

	void response_robot(int32_t playerid, int tag);

	void service_ctrl(int32_t opttype, int32_t roomID);
	void kick_player(uint32_t playerid, int bforce);
	void kill_points(uint16_t rid, int32_t cutRound, bool status);

private:
	void init_config();
	void init_protocol();
	void reload_fishcfg();

	bool m_init;
	int m_max_player;
	LROOM_MAP m_rooms;
	LPLAYER_MAP m_all_players;
};



DRAGON_RED_BLACK_END
