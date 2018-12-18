#pragma once
#include <net/packet_manager.h>
#include <cows_logic.pb.h>
#include <net/peer_tcp.h>

class i_game_player;
using namespace cows_protocols;

void init_proc_cows_logic();

PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_get_scene_info, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_get_scene_info_result);

PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_bet_info, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_bet_info_result);

PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_apply_banker, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_apply_banker_result);

PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_cancel_apply_banker, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_cancel_apply_banker_result);

PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_leave_banker, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_leave_banker_result);

PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_bankerlist, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_bankerlist_result);

PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_snatch_banker, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_snatch_banker_result);

PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_history_info, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_history_info);

PACKET_REGEDIT_SEND(packetl2c_banker_info);

PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_continue_bet, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_continue_bet_result);

PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_clear_bet, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_clear_bet_result);

PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_ask_playerlist, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_playerlist_result);

PACKET_REGEDIT_SEND(packetl2c_banker_success);

PACKET_REGEDIT_SEND(packetl2c_bc_scene_prepare_into);
PACKET_REGEDIT_SEND(packetl2c_bc_scene_bet_into);
PACKET_REGEDIT_SEND(packetl2c_bc_sync_scene_bet_into);
PACKET_REGEDIT_SEND(packetl2c_bc_scene_deal_into);
PACKET_REGEDIT_SEND(packetl2c_bc_scene_result_into);
PACKET_REGEDIT_SEND(packetl2c_bc_snatch_banker);

PACKET_REGEDIT_SEND(packetl2c_kick_player);