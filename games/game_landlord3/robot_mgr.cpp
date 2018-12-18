#include "stdafx.h"
#include "robot_mgr.h"
#include "robot_player.h"

#include "logic_player.h"

#include "robot_packet_manager.h"
#include <net\packet_manager.h>
#include "game_engine.h"
#include "logic_lobby.h"

#include "robot_proc_logic.h"



LANDLORD_SPACE_USING


robot_mgr::robot_mgr(void)
{
	init_robot_proc_logic();
	
}


robot_mgr::~robot_mgr(void)
{
}

void robot_mgr::heartbeat( double elapsed )
{
	// 管理所有入座的机器人
	for (auto it=m_robot_players.begin();it!=m_robot_players.end();it++)
	{
		auto player = it->second;
		if (player)
		{
			player->heartbeat(elapsed);
		}
			
	}
	
}

landlord_space::RobotPlayerPtr& landlord_space::robot_mgr::get_robot_player(uint32_t player_id)
{
	auto it = m_robot_players.find(player_id);
	if (it != m_robot_players.end())
	{
		return it->second;
	}
	return robot_player::EmptyPtr;
}

uint32_t landlord_space::robot_mgr::get_robot_count()
{
	return m_robot_players.size();
}

void landlord_space::robot_mgr::robot_player_leave(uint32_t player_id)
{
	auto it = m_robot_players.find(player_id);
	if (it != m_robot_players.end()) m_robot_players.erase(it);
	
}

void landlord_space::robot_mgr::robot_player_enter(RobotPlayerPtr& player)
{
	auto it = m_robot_players.find(player->get_pid());
	if (it != m_robot_players.end())
	{
		assert(0);
		return;
	}
	m_robot_players.insert(std::make_pair(player->get_pid(), player));
}

void landlord_space::robot_mgr::recv_packet(uint32_t player_id, uint32_t packet_id, boost::shared_ptr<google::protobuf::Message> packet)
{
	auto factory = robot_packet_manager::instance().get_factroy(packet_id);
	if (factory != nullptr)
	{
		factory->packet_process(player_id, packet);
	}
	
}

void landlord_space::robot_mgr::send_packet(uint32_t player_id, uint32_t packet_id, boost::shared_ptr<google::protobuf::Message> packet)
{
	auto& lobby = game_engine::instance().get_lobby();
	auto& player = lobby.get_player(player_id);
	//发
// 	auto factory = packet_manager::instance().get_factroy(packet_id);
// 	if(factory != nullptr)
// 	{			
// 		printf("[send]%p \n",player);
// 		factory->packet_process(nullptr, player, packet);
// 	}			
	if(player)
	{
		player->send_packet(packet_id,packet);
	}
}


