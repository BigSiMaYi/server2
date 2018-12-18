#pragma once

#define ZJH_SPACE_BEGIN namespace zjh_space {
#define ZJH_SPACE_END	}
#define ZJH_SPACE_USING using namespace zjh_space;

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

ZJH_SPACE_BEGIN
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


#define SAFE_DELETE(v) if(v != nullptr){delete v; v = nullptr;}

#define GAME_ID			13
#define GAME_PLAYER		5	
#define MAX_CARDS_HAND	3
#define INVALID_CHAIR	0xFF
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
	eGameState_Spare,
	eGameState_FaPai,
	eGameState_Play,
	eGameState_End,
};

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

#define GITOUT		30

// Fapai duration
#define CHIP_TIME	0.3
#define	SPACE_TIME	0.1
#define	PAUSE_TIME	0.5

struct  tagPlayerCtrlAttr
{
	int32_t	nTag;
	double fwinPercent;
};

struct	tagPlayerFlags
{
	int32_t Flags ;   //@X ��ǩ;
	float WinP ;
	bool  FlagsS ;
	float opCoeff ;
	int64_t flagsMoneyLit ;
	int64_t CurMoneyGet ;
	int32_t GmOpFlags ;
	int64_t SendCoinCnt ;
	int64_t RecvCoinCnt ;
	int64_t RechargeCnt ;
	int64_t WithDrawCnt ;
	int32_t FlagsY ;
	int64_t FlagsX ;
	int64_t SafeBag ;
};

struct	tagOptInfo
{
	int32_t	nOpt;		// 
	int64_t	vargold;	// ��ұ仯
	int32_t targetID;	
	int32_t	round;		// ��Ϸ����
	int64_t opttime;	// ��ǰ����ʱ��

	tagOptInfo()
	{
		nOpt = eOperator_null;
		vargold = 0;
		targetID = 0;
		round = 0;
		opttime = 0;
	}
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

#define UsingLog
//#define UsingCling

//#define GetFlags

ZJH_SPACE_END
