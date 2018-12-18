// game_fishlord.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "game_zjh.h"
#include "game_engine.h"
#include <net/packet_manager.h>

GAME_ZJH_API void* get_game_engine()
{
	return &game_engine::instance();
}


GAME_ZJH_API void* get_packet_mgr()
{
	return &packet_manager::instance();
}