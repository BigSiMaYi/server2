#pragma once
#include <zjh_logic.pb.h>
#include <net/peer_tcp.h>

#include "robot_packet_manager.h"
#include "zjh_robot.pb.h"

using namespace zjh_protocols;

void init_robot_proc_logic();

ROBOT_PACKET_REGEDIT_RECV(packetl2c_robot_gameinfo);
ROBOT_PACKET_REGEDIT_RECV(packetl2c_robot_enter);
ROBOT_PACKET_REGEDIT_RECV(packetl2c_robot_leave);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_scene_info_free);
ROBOT_PACKET_REGEDIT_RECV(packetl2c_scene_info_play);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_ask_ready_result);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_game_start);
ROBOT_PACKET_REGEDIT_RECV(packetl2c_notice_start);
// opera
ROBOT_PACKET_REGEDIT_RECV(packetl2c_operator_kan);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_operator_gen);
ROBOT_PACKET_REGEDIT_RECV(packetl2c_operator_jia);
ROBOT_PACKET_REGEDIT_RECV(packetl2c_operator_bi);
ROBOT_PACKET_REGEDIT_RECV(packetl2c_operator_qi);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_ask_operator_showhand_result);
ROBOT_PACKET_REGEDIT_RECV(packetl2c_operator_showhand);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_notice_allin);

ROBOT_PACKET_REGEDIT_RECV(packetl2c_operator_allin);
// result
ROBOT_PACKET_REGEDIT_RECV(packetl2c_game_result);



