#pragma once
#include <bmw_logic.pb.h>
#include <net/peer_tcp.h>

#include "robot_packet_manager.h"

using namespace bmw_protocols;

void init_robot_proc_logic();

ROBOT_PACKET_REGEDIT_RECV(packetl2c_robot_enter);
ROBOT_PACKET_REGEDIT_RECV(packetl2c_robot_leave);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_scene_info_free);
ROBOT_PACKET_REGEDIT_RECV(packetl2c_scene_info_play);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_ask_ready_result);


ROBOT_PACKET_REGEDIT_RECV(packetl2c_notice_start);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_change_banker);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_ask_apply_banker_result);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_ask_unapply_banker_result);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_ask_place_bet_result);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_notice_placefull);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_run_result);

// opera

// result
ROBOT_PACKET_REGEDIT_RECV(packetl2c_game_result);



