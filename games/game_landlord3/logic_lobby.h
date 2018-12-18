#pragma once
#include "logic_def.h"
#include <i_game_def.h>

LANDLORD_SPACE_BEGIN

class logic_lobby
{
public:
	logic_lobby(void);
	~logic_lobby(void);

	void init_game(int count, int room_id, int child_id);
	void release_game();
	void heartbeat(double elapsed);
	void heartbeat_robot(double elapsed);
	
	void kick_player(uint32_t playerid, int bforce);
	bool player_enter_game(iGPlayerPtr igplayer);
	void player_leave_game(uint32_t playerid);
	int player_join_friend_game(iGPlayerPtr igplayer, uint32_t friendid);

	const LROOM_MAP& get_rooms();

	int enter_room(uint32_t pid, uint16_t rid, uint16_t tid=0);	

	void enter_room_nocheck(uint32_t pid, uint16_t rid);
	void leave_room(uint32_t pid);

	LPlayerPtr& get_player(uint32_t pid);
	logic_table* get_player_table(uint32_t pid);

	void response_robot(int32_t playerid, int tag);
	void robot_leave(int32_t playerid);
	void robot_enter(LPlayerPtr& player, int tag);

    void service_ctrl(int32_t opttype, int32_t roomID);
	void kill_points(uint16_t rid, int32_t cutRound, bool status);

protected:
    void init_config();
    void init_protocol();
    void reload_fishcfg();

private:
	bool m_init;
	int m_max_player;
	LROOM_MAP m_rooms;
	LPLAYER_MAP m_all_players;

	double m_check_cache;
	void save_cache();		//统计当天收益
	
};



LANDLORD_SPACE_END
