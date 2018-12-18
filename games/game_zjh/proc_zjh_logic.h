#pragma once
#include <net/packet_manager.h>
#include <zjh_logic.pb.h>
#include <zjh_robot.pb.h>
#include <net/peer_tcp.h>

class i_game_player;
using namespace zjh_protocols;

void init_proc_zjh_logic();

PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_get_scene_info, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_scene_info_free);
PACKET_REGEDIT_SEND(packetl2c_scene_info_play);
// 准备
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_ready, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_ask_ready_result);
// 跟注
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_operator_gen, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_operator_gen);
// 看牌
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_operator_kan, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_operator_kan);
// 弃牌
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_operator_qi, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_operator_qi);
// 加注
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_operator_jia, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_operator_jia);
// 比牌
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_operator_bi, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_operator_bi);
// 梭哈
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_operator_showhand, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_ask_operator_showhand_result);
PACKET_REGEDIT_SEND(packetl2c_operator_showhand);
// 孤注一掷
PACKET_REGEDIT_SEND(packetl2c_notice_allin);
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_operator_allin, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_operator_allin);
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_operator_pk, i_game_player);
// 全比
PACKET_REGEDIT_SEND(packetl2c_all_pk);
// 允许看牌
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_let_see, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_let_see_result);
// 开牌
PACKET_REGEDIT_SEND(packetl2c_open_card);
// 结算
PACKET_REGEDIT_SEND(packetl2c_game_result);
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////


PACKET_REGEDIT_SEND(packetl2c_user_enter_seat);
PACKET_REGEDIT_SEND(packetl2c_user_leave_seat);

PACKET_REGEDIT_SEND(packetl2c_game_start);

PACKET_REGEDIT_SEND(packetl2c_notice_sparetime);

PACKET_REGEDIT_SEND(packetl2c_notice_start);

PACKET_REGEDIT_SEND(packetl2c_robot_gameinfo);
PACKET_REGEDIT_SEND(packetl2c_robot_enter);
PACKET_REGEDIT_SEND(packetl2c_robot_leave);

PACKET_REGEDIT_SEND(packetl2c_clean_out);