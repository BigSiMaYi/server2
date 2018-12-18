#pragma once

#define DRAGON_RED_BLACK_BEGIN namespace red_black_space {
#define DRAGON_RED_BLACK_END	}
#define DRAGON_RED_BLACK_USING using namespace red_black_space;

#include <enable_smart_ptr.h>
#include <enable_object_pool.h>
#include <enable_hashmap.h>

#include <game_object.h>
#include <game_object_field.h>
#include <game_object_map.h>
#include <game_object_array.h>
#include <game_object_container.h>

#include <server_log.h>

DRAGON_RED_BLACK_BEGIN
class logic_room;
class logic_lobby;
class logic_player;

class logic_main;
class logic_banker;
class logic_other;
class logic_cards;
class logic_poker_mgr;
class logic_robot;

enum panel_info
{
	red_panel  = 1, //红方面板
	black_panel = 2, //黑方面板
	other_panel  = 3, //金花、豹子、顺子面板等

};

typedef boost::shared_ptr<logic_room> LRoomPtr;
typedef boost::shared_ptr<logic_player> LPlayerPtr;
typedef boost::shared_ptr<logic_robot> LRobotPtr;
typedef std::map<uint16_t, LRoomPtr> LROOM_MAP;
typedef ENABLE_MAP<uint32_t, LPlayerPtr> LPLAYER_MAP;

#define SAFE_DELETE(v) if(v != nullptr){delete v; v = nullptr;}

#define PROPERTY_DEFINE(type, name, access_permission)\
access_permission:\
	type m_##name;\
	public:\
	inline void set_##name(type v) {\
	m_##name = v;\
}\
	inline type get_##name() {\
	return m_##name;\
}\

DRAGON_RED_BLACK_END
