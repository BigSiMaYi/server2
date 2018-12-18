#pragma once
#include <net\packet_manager.h>
#include <game_example_protocol.pb.h>
#include <net\peer_tcp.h>

class i_game_player;
using namespace game_example_protocols;

void init_proc_example_logic();

PACKET_REGEDIT_RECVGATE_LOG(peer_tcp, packetc2l_game_play, i_game_player);
PACKET_REGEDIT_SEND(packetl2c_game_play_result);