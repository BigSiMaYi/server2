#include "stdafx.h"
#include "robot_mgr.h"
#include "robot_player.h"

#include "logic_player.h"

#include "robot_packet_manager.h"
#include <net/packet_manager.h>
#include "game_engine.h"
#include "logic_lobby.h"

#include "robot_proc_logic.h"



ZJH_SPACE_USING


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
	//
	process_recv_packet();
	
}

zjh_space::RobotPlayerPtr& zjh_space::robot_mgr::get_robot_player(uint32_t player_id)
{
	auto it = m_robot_players.find(player_id);
	if (it != m_robot_players.end())
	{
		return it->second;
	}
	return robot_player::EmptyPtr;
}

uint32_t zjh_space::robot_mgr::get_robot_count()
{
	return m_robot_players.size();
}

void zjh_space::robot_mgr::robot_player_leave(uint32_t player_id)
{
	auto it = m_robot_players.find(player_id);
	if (it != m_robot_players.end()) m_robot_players.erase(it);
	
}

void zjh_space::robot_mgr::robot_player_enter(RobotPlayerPtr& player)
{
	auto it = m_robot_players.find(player->get_pid());
	if (it != m_robot_players.end())
	{
		assert(0);
		return;
	}
	m_robot_players.insert(std::make_pair(player->get_pid(), player));
}


void zjh_space::robot_mgr::recv_bc_packet(std::vector<uint32_t>& pids, std::vector<msg_packet_one>& msglist)
{
	if (pids.size() == 0) return;
	
	for (unsigned int i = 0; i < msglist.size(); i++)
	{
		auto factory = robot_packet_manager::instance().get_factroy(msglist[i].packet_id);
		if (factory != nullptr)
		{
			// ADD 2017-8-28 
			for(auto uid : pids)
			{
				m_recv_packets.push_back(robot_packet(uid, msglist[i].packet_id, msglist[i].msg_packet));	
			}
		}
	}
}

void zjh_space::robot_mgr::recv_packet(uint32_t player_id, uint32_t packet_id, boost::shared_ptr<google::protobuf::Message> packet)
{
#if 0
	auto factory = robot_packet_manager::instance().get_factroy(packet_id);
	if (factory != nullptr)
	{
		factory->packet_process(player_id, packet);
	}
#else
	m_recv_packets.push_back(robot_packet(player_id, packet_id, packet));
#endif
	
}

void zjh_space::robot_mgr::send_packet(uint32_t player_id, uint32_t packet_id, boost::shared_ptr<google::protobuf::Message> packet)
{
#if 0
	auto& lobby = game_engine::instance().get_lobby();
	auto& player = lobby.get_player(player_id);
	//发		
	if(player)
	{
		player->send_packet(packet_id,packet);
	}
#else
	m_send_packets.push_back(robot_packet(player_id, packet_id, packet));
#endif
}

void zjh_space::robot_mgr::process_recv_packet()
{
	//收
	for (auto it = m_recv_packets.begin(); it != m_recv_packets.end(); it++)
	{
		robot_packet& packet = *it;
		auto factory = robot_packet_manager::instance().get_factroy(packet.m_packet_id);
		if (factory != nullptr)
		{
			factory->packet_process(packet.m_player_id, packet.m_packet);
		}
	}
	m_recv_packets.clear();
}

void zjh_space::robot_mgr::process_send_packet()
{
	auto& lobby = game_engine::instance().get_lobby();

	for (auto it = m_send_packets.begin(); it != m_send_packets.end(); it++)
	{
		auto& packet = *it;
		auto& player = lobby.get_player(packet.m_player_id);
		if (player != nullptr)
		{
			player->recv_cache(packet.m_packet_id, packet.m_packet);
		}
	}

	m_send_packets.clear();
}

