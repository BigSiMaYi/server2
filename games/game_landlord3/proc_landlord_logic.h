#pragma once
#include <net\packet_manager.h>
#include <landlord3_logic.pb.h>
#include <net\peer_tcp.h>
#include "landlord3_logic.pb.h"
class i_game_player;
using namespace landlord3_protocols;

void init_proc_landlord_logic();

PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_get_scene_info, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_scene_info_free);
PACKET_REGEDIT_SEND(packetl2c_scene_info_play);
// ׼��
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_ready, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_ask_ready_result);
// ��ׯ
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_robbank, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_robbank_result);
// ����
PACKET_REGEDIT_SEND(packetl2c_bombrate);
// ����
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_pass, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_pass_result);
// ����
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_outcard, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_outcard_result);
// �й�
PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_trustee, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_trustee_result);
// ����
PACKET_REGEDIT_SEND(packetl2c_open_card);
// ����
PACKET_REGEDIT_SEND(packetl2c_game_result);
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////


PACKET_REGEDIT_SEND(packetl2c_user_enter_seat);
PACKET_REGEDIT_SEND(packetl2c_user_leave_seat);

PACKET_REGEDIT_SEND(packetl2c_game_start);




PACKET_REGEDIT_SEND(packetl2c_robot_enter);
PACKET_REGEDIT_SEND(packetl2c_robot_leave);

PACKET_REGEDIT_SEND(packetl2c_clean_out);

PACKET_REGEDIT_SEND(packetl2c_notice_robbanker);
PACKET_REGEDIT_SEND(packetl2c_base_card);