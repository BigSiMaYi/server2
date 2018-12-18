#pragma once
#include "robot_def.h"
#include "i_game_def.h"

ROBCOWS_SPACE_BEGIN

class robot_mgr : public enable_singleton<robot_mgr>
{
public:
	robot_mgr(void);
	virtual ~robot_mgr(void);

	void heartbeat(double elapsed);

public:
	void robot_player_enter(RobotPlayerPtr& player);
	void robot_player_leave(uint32_t player_id);
	RobotPlayerPtr& get_robot_player(uint32_t player_id);
	uint32_t get_robot_count();

	void send_packet(uint32_t player_id, uint32_t packet_id, boost::shared_ptr<google::protobuf::Message> packet);
	void recv_packet(uint32_t player_id, uint32_t packet_id, boost::shared_ptr<google::protobuf::Message> packet);

private:
	RobotPlayer_MAP m_robot_players;
};

ROBCOWS_SPACE_END
