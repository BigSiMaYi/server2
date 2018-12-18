#pragma once

#define BMW_SPACE_BEGIN namespace bmw_space {
#define BMW_SPACE_END	}
#define BMW_SPACE_USING using namespace bmw_space;

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

BMW_SPACE_BEGIN
class logic_room;
class logic_table;

class logic_player;
class logic_lobby;

class logic_main;
class robot_player;


typedef boost::shared_ptr<logic_room> LRoomPtr;
typedef boost::shared_ptr<logic_table> LTablePtr;
typedef boost::shared_ptr<logic_player> LPlayerPtr;
typedef boost::shared_ptr<robot_player> LRobotPtr;

typedef std::map<uint16_t, LRoomPtr> LROOM_MAP;
typedef std::map<uint16_t, LTablePtr> LTABLE_MAP;

typedef ENABLE_MAP<uint32_t, LPlayerPtr> LPLAYER_MAP;
typedef ENABLE_MAP<uint32_t, LRobotPtr> LPROBOT_MAP;


#define SAFE_DELETE(v) if(v != nullptr){delete v; v = nullptr;}

#define GAME_ID			16
#define GAME_PLAYER		300	

#define MAX_CARDS_HAND	3
#define INVALID_CHAIR	-1
#define PLAY_MIN_COUNT	2

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
	eGameState_Place,	// ��ע��ʼ
	eGameState_Play,	// ��ע����
	eGameState_End,		// ����
};

struct tagAreaChip
{
	int64_t big_vw;
	int64_t small_vw;

	int64_t big_benz;
	int64_t small_benz;

	int64_t big_bmw;
	int64_t small_bmw;

	int64_t big_porsche;
	int64_t small_porsche;

	tagAreaChip()
	{
		big_vw = 0;
		small_vw = 0;

		big_benz = 0;
		small_benz = 0;

		big_bmw = 0;
		small_bmw = 0;

		big_porsche = 0;
		small_porsche = 0;
	}
};

#define JETTON_COUNT		5

#define	PLACE_AREA			8

#define ID_BIG_Porsche		8	//��ʱ��
#define ID_SMALL_Porsche	4	//

#define ID_BIG_BMW			7	//����
#define ID_SMALL_BMW		3	//

#define ID_BIG_Benz			6	//����
#define ID_SMALL_Benz		2	//

#define ID_BIG_VW			5	//����
#define ID_SMALL_VW			1	//
// Rate
#define BIG_Porsche			40	//��ʱ��
#define SMALL_Porsche		5	//
#define BIG_BMW				30	//����
#define SMALL_BMW			5	//
#define BIG_Benz			20	//����
#define SMALL_Benz			5	//
#define BIG_VW				10	//����
#define SMALL_VW			5	//

#define TOP_WINER			5

#define BANKER_MIN_ROUND	3

enum eUserStatus
{
	eUserState_null,	
	eUserState_free,	// ����

	eUserState_ready,	// ׼��

	eUserState_play,	// ��Ϸ---
	eUserState_Wait,	// �ȴ���һ��
	eUserState_dead,	// ����״̬

	eUserState_Trustee,	// �й�
	eUserState_OffLine,	// ����
};

enum eCardStatus
{
	eCardState_null	= 0x0000,
	eCardState_An	= 0x0001,
	eCardState_Ming = 0x0002,
	eCardState_Shu	= 0x0004,
	eCardState_Qi	= 0x0008,
};

enum eOperatorFlag
{
	eOperator_null,
	eOperator_Flow,
	eOperator_Add,
	eOperator_Check,
	eOperator_Compare,
	eOperator_ShowHand,
	eOperator_GiveUp,
	eOperator_Allin,		// ����һע
	eOperator_Allpk,		// Ⱥħ����
	eOperator_Max,
};

enum eCompareType
{
	eCompareType_null,
	eCompareType_client,
	eCompareType_server,
};

enum eGameEndType
{
	eGameEndType_null,		//
	eGameEndType_OnlyOne,	// ʣһ����
	eGameEndType_Compare,	// ����
	eGameEndType_ShowHand,	//
	eGameEndType_AllIN,		// ��עһ��
	eGameEndType_AllPK,		// Ⱥħ����

};

#define DISSMISS	900

// Fapai duration
#define CHIP_TIME	0.3
#define	SPACE_TIME	0.1
#define	PAUSE_TIME	0.5

struct  tagPlayerCtrlAttr
{
	int32_t	nTag;
	double fwinPercent;
};

// Tag Test
#define Tag_Supper		110	// �����û�
#define Tag_UserA		10	// A �����
#define Tag_UserB		30	// B �����
#define Tag_UserC		60	// C �����

#define Tag_Robot		100
// Label Test
#define Label_Supper	110	

#define Label_Null		1	// 
#define Label_UserA		10	// A �����
#define Label_UserB		30	// B �����
#define Label_UserC		60	// C �����

#define Label_Robot		100

BMW_SPACE_END
