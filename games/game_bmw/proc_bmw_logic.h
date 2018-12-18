#pragma once
#include <net/packet_manager.h>
#include <bmw_logic.pb.h>
#include <net/peer_tcp.h>

class i_game_player;
using namespace bmw_protocols;

void init_proc_bmw_logic();

PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_get_scene_info, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_scene_info_free);
PACKET_REGEDIT_SEND(packetl2c_scene_info_play);
// 准备
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_ready, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_ask_ready_result);
// 下注
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_place_bet, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_ask_place_bet_result);
// 续投
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_place_again, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_ask_place_again_result);
// 上庄
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_apply_banker, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_ask_apply_banker_result);
// 下庄
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_unapply_banker, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_ask_unapply_banker_result);
// 历史记录
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_history, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_respone_history);

PACKET_REGEDIT_SEND(packetl2c_change_banker);
PACKET_REGEDIT_SEND(packetl2c_run_result);
// 结算
PACKET_REGEDIT_SEND(packetl2c_game_result);
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////


PACKET_REGEDIT_SEND(packetl2c_user_enter_seat);
PACKET_REGEDIT_SEND(packetl2c_user_leave_seat);

PACKET_REGEDIT_SEND(packetl2c_notice_start);


PACKET_REGEDIT_SEND(packetl2c_robot_enter);
PACKET_REGEDIT_SEND(packetl2c_robot_leave);

PACKET_REGEDIT_SEND(packetl2c_clean_out);