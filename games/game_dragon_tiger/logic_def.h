#pragma once

#define DRAGON_TIGER_SPACE_BEGIN namespace dragon_tiger_space {
#define DRAGON_TIGER_SPACE_END	}
#define DRAGON_TIGER_SPACE_USING using namespace dragon_tiger_space;

#include <enable_smart_ptr.h>
#include <enable_object_pool.h>
#include <enable_hashmap.h>

#include <game_object.h>
#include <game_object_field.h>
#include <game_object_map.h>
#include <game_object_array.h>
#include <game_object_container.h>

#include <server_log.h>

DRAGON_TIGER_SPACE_BEGIN
class logic_room;
class logic_lobby;
class logic_player;

class logic_main;
class logic_other;
class logic_cards;
class logic_poker_mgr;
class logic_robot;

enum panel_info
{
	dragon_panel = 1, //Áú
	equal_panel  = 2, //ºÍÅÆÃæ°å
	tiger_panel  = 3, //»¢

};

typedef boost::shared_ptr<logic_room> LRoomPtr;
typedef boost::shared_ptr<logic_player> LPlayerPtr;
typedef boost::shared_ptr<logic_robot> LRobotPtr;
typedef std::map<uint16_t, LRoomPtr> LROOM_MAP;
typedef ENABLE_MAP<uint32_t, LPlayerPtr> LPLAYER_MAP;

#define SAFE_DELETE(v) if(v != nullptr){delete v; v = nullptr;}

DRAGON_TIGER_SPACE_END
