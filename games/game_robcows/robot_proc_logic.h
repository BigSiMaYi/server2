#pragma once
#include <robcows_robot.pb.h>
#include <net/peer_tcp.h>

#include "robot_packet_manager.h"

using namespace robcows_protocols;

void init_robot_proc_logic();

ROBOT_PACKET_REGEDIT_RECV(packetl2c_robot_enter);
ROBOT_PACKET_REGEDIT_RECV(packetl2c_robot_leave);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_scene_info_free);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_ask_ready_result);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_scene_info_play_bank);
ROBOT_PACKET_REGEDIT_RECV(packetl2c_scene_info_play_bet);
ROBOT_PACKET_REGEDIT_RECV(packetl2c_scene_info_play_opencard);
ROBOT_PACKET_REGEDIT_RECV(packetl2c_scene_info_play_display);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_scene_info_play);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_notice_sparetime);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_game_start);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_notice_robbanker);
ROBOT_PACKET_REGEDIT_RECV(packetl2c_notice_bet);
ROBOT_PACKET_REGEDIT_RECV(packetl2c_notice_opencard);
ROBOT_PACKET_REGEDIT_RECV(packetl2c_fifth_card);
ROBOT_PACKET_REGEDIT_RECV(packetl2c_game_result);

