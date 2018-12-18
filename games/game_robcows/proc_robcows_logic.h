#pragma once
#include <net/packet_manager.h>
#include <robcows_robot.pb.h>
#include <net/peer_tcp.h>

class i_game_player;
using namespace robcows_protocols;

void init_proc_robcows_logic();

PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_get_scene_info, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_scene_info_free);

PACKET_REGEDIT_SEND(packetl2c_scene_info_play_bank);
PACKET_REGEDIT_SEND(packetl2c_scene_info_play_bet);
PACKET_REGEDIT_SEND(packetl2c_scene_info_play_opencard);
PACKET_REGEDIT_SEND(packetl2c_scene_info_play_display);

PACKET_REGEDIT_SEND(packetl2c_scene_info_play);
// ׼��
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_ready, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_ask_ready_result);

//////////////////////////////////////////////////////////////////////////

PACKET_REGEDIT_SEND(packetl2c_user_enter_seat);
PACKET_REGEDIT_SEND(packetl2c_user_leave_seat);

PACKET_REGEDIT_SEND(packetl2c_notice_sparetime);
PACKET_REGEDIT_SEND(packetl2c_game_start);
// ��ׯ����
PACKET_REGEDIT_SEND(packetl2c_notice_robbanker);
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp,packetc2l_ask_robbank,i_game_player);
PACKET_REGEDIT_SEND(packetl2c_robbank_result);
// ��ע����
PACKET_REGEDIT_SEND(packetl2c_notice_bet);
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp,packetc2l_ask_bet,i_game_player);
PACKET_REGEDIT_SEND(packetl2c_bet_result);
// ��������
PACKET_REGEDIT_SEND(packetl2c_fifth_card);
// ����
PACKET_REGEDIT_SEND(packetl2c_notice_opencard);
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp,packetc2l_ask_opencard,i_game_player);
PACKET_REGEDIT_SEND(packetl2c_opencard_result);
// չʾ��
PACKET_REGEDIT_SEND(packetl2c_showcard);
// ����
PACKET_REGEDIT_SEND(packetl2c_game_result);
//////////////////////////////////////////////////////////////////////////
PACKET_REGEDIT_SEND(packetl2c_robot_enter);
PACKET_REGEDIT_SEND(packetl2c_robot_leave);

PACKET_REGEDIT_SEND(packetl2c_clean_out);