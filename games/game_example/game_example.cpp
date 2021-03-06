// game_example.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "game_example.h"

#include "game_engine.h"
#include <net/packet_manager.h>

GAME_EXAMPLE_API void* get_game_engine()
{
	return &game_engine::instance();
}


GAME_EXAMPLE_API void* get_packet_mgr()
{
	return &packet_manager::instance();
}