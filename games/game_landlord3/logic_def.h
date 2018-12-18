#pragma once

#define LANDLORD_SPACE_BEGIN namespace landlord_space {
#define LANDLORD_SPACE_END	}
#define LANDLORD_SPACE_USING using namespace landlord_space;

#include <enable_smart_ptr.h>
#include <enable_object_pool.h>
#include <enable_hashmap.h>

#include <game_object.h>
#include <game_object_field.h>
#include <game_object_map.h>
#include <game_object_array.h>
#include <game_object_container.h>

#include <server_log.h>

#include <enable_random.h>

LANDLORD_SPACE_BEGIN
class logic_room;
class logic_table;

class logic_player;
class logic_lobby;

class robot_player;


typedef boost::shared_ptr<logic_room> LRoomPtr;
typedef boost::shared_ptr<logic_table> LTablePtr;
typedef boost::shared_ptr<logic_player> LPlayerPtr;
typedef boost::shared_ptr<robot_player> LRobotPtr;

typedef std::map<uint16_t, LRoomPtr> LROOM_MAP;
typedef std::map<uint16_t, LTablePtr> LTABLE_MAP;

typedef ENABLE_MAP<uint32_t, LPlayerPtr> LPLAYER_MAP;
typedef ENABLE_MAP<uint32_t, LRobotPtr> LPROBOT_MAP;

typedef std::vector<uint32_t> Vec_UserID;

#define SAFE_DELETE(v) if(v != nullptr){delete v; v = nullptr;}

#define GAME_ID			15
#define GAME_PLAYER		3

#define INVALID_CHAIR	0xFF

#define TIME_INTERVAL	3
#define TIME_COMPARE	4

enum eReadyResult
{
	eReadyResult_Faild_unknow,
	eReadyResult_Faild_gold,
	eReadyResult_Faild_userstate,
	eReadyResult_Faild_gamestate,
	eReadyResult_Succeed,
};

enum eGameStatus
{
	eGameState_Free,
	eGameState_FaPai,
	eGameState_Banker,
	eGameState_Play,
	eGameState_End,
};

enum eUserStatus
{
	eUserState_null,	
	eUserState_free,	// 入座

	eUserState_ready,	// 准备

	eUserState_play,	// 游戏---
	eUserState_Wait,	// 等待下一局
	eUserState_dead,	// 死亡状态

	eUserState_Trustee,	// 托管
	eUserState_OffLine,	// 掉线
};


enum eCallBanker
{
	eCallBanker_uncall,
	eCallBanker_OnePoint,
	eCallBanker_TwoPoint,
	eCallBanker_ThreePoint,
};




LANDLORD_SPACE_END
