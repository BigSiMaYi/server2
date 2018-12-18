#pragma once

#define EXAMPLE_SPACE_BEGIN namespace fish_space {
#define EXAMPLE_SPACE_END	}
#define EXAMPLE_SPACE_USING using namespace fish_space;

#include <enable_smart_ptr.h>
#include <enable_object_pool.h>
#include <enable_hashmap.h>

#include <game_object.h>
#include <game_object_field.h>
#include <game_object_map.h>
#include <game_object_array.h>
#include <game_object_container.h>

#include <server_log.h>

EXAMPLE_SPACE_BEGIN

class logic_room;
class logic_table;
class logic_player;
class logic_lobby;

typedef boost::shared_ptr<logic_room> LRoomPtr;
typedef boost::shared_ptr<logic_table> LTablePtr;
typedef boost::shared_ptr<logic_player> LPlayerPtr;

typedef std::map<uint16_t, LRoomPtr> LROOM_MAP;
typedef std::map<uint16_t, LTablePtr> LTABLE_MAP;
typedef ENABLE_MAP<uint32_t, LPlayerPtr> LPLAYER_MAP;

#define SAFE_DELETE(v) if(v != nullptr){delete v; v = nullptr;}

EXAMPLE_SPACE_END
