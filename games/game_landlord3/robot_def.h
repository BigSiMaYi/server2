#pragma once
#include "logic_def.h"
#include <vector>

LANDLORD_SPACE_BEGIN

#define TIME_LESS 2
#define TIME_END  4

#define ROBOT_LESS		3.5
#define ROBOT_ACTION	1

class robot_player;
struct robot_packet
{
	uint32_t m_player_id;
	uint32_t m_packet_id;
	boost::shared_ptr<google::protobuf::Message> m_packet;

	robot_packet(uint32_t player_id, uint32_t packet_id, boost::shared_ptr<google::protobuf::Message> packet)
		:m_player_id(player_id)
		,m_packet_id(packet_id)
		,m_packet(packet)
	{

	}
};

typedef boost::shared_ptr<robot_player> RobotPlayerPtr;
typedef ENABLE_MAP<uint32_t, RobotPlayerPtr> RobotPlayer_MAP;

typedef std::list<robot_packet> RobotPacketList;

typedef std::vector<uint32_t> VectorUserID;

LANDLORD_SPACE_END
