#include "stdafx.h"
#include "logic_player.h"
#include "logic_table.h"

#include <i_game_player.h>
#include "game_db.h"
#include "game_db_log.h"
#include "logic_room.h"

#include "robot_player.h"

#include "proc_bmw_logic.h"
#include "proc_bmw_protocol.h"

#include "BMW_RoomCFG.h"
#include "BMW_BaseInfo.h"
#include "game_engine.h"

#include "msg_type_def.pb.h"
#include "pump_type.pb.h"

BMW_SPACE_USING

bmw_space::logic_player::logic_player(void)
:m_lobby(nullptr)
,m_table(nullptr)
,m_logic_gold(0)
,m_change_gold(0)
,m_nServerFlag(0)
{
	init_game_object();

	m_nChairID->set_value(INVALID_CHAIR);
	m_nPlayerStatus->set_value(eUserState_null);
	m_nPlayRound->set_value(0);
	m_nWaitRound->set_value(0);
}



logic_player::~logic_player(void)
{

}

//////////////////////////////////////////////////////////////////////////
void logic_player::on_attribute_change(int atype, int v)
{
	if(atype == msg_type_def::e_itd_gold)
		m_logic_gold += v;
}

void logic_player::on_attribute64_change(int atype, GOLD_TYPE v)
{
	m_logic_gold += v;
}


void bmw_space::logic_player::quest_change_from_world(int quest_type,int count,int param)
{

}

void logic_player::on_change_state()
{
	// TODO
}

int bmw_space::logic_player::cltReq_leaveGame()
{
	// @1,可离开 2,不可离开
	if (can_leave_table())
	{
		reqPlayer_leaveGame();
		return 1;
	}
	else
	{
		m_nServerFlag = 1;
	}
	return 2;
}

//////////////////////////////////////////////////////////////////////////
void logic_player::heartbeat( double elapsed )
{
	// TODO 超时踢人
}

void logic_player::init(iGPlayerPtr player)
{
	set_player(player);
}

void logic_player::enter_game(logic_lobby* lobby)
{
	m_lobby = lobby;
	if(!load_player())
		create_player();

	m_logic_gold = m_player->get_attribute64(msg_type_def::e_itd_gold);
}

bool logic_player::join_table(logic_table* table,int32_t seat/*=0*/)
{
	// 入桌
	m_table = table;
	// 设置用户椅子、状态
	m_nChairID->set_value(seat);

	m_nPlayRound->set_value(0);
	m_nWaitRound->set_value(0);


	return true;
}

int32_t logic_player::get_seat() const
{
	
	return m_nChairID->get_value();
}

void logic_player::set_status(int state)
{
	m_nPlayerStatus->set_value(state);
}

int32_t logic_player::get_status()
{
	return m_nPlayerStatus->get_value();
}


void bmw_space::logic_player::set_wait(int wait)
{
	m_nWaitRound->set_value(wait);
}

void bmw_space::logic_player::add_wait(int wait/*=1*/)
{
	m_nWaitRound->add_value(wait);
}

int32_t bmw_space::logic_player::get_wait()
{
	return m_nWaitRound->get_value();
}

void bmw_space::logic_player::add_round(int round/*=1*/)
{
	m_nPlayRound->add_value(round);
}

void bmw_space::logic_player::clear_round(int round/*=0*/)
{
	m_nPlayRound->set_value(round);
}

int32_t bmw_space::logic_player::get_round()
{
	return m_nPlayRound->get_value();
}

logic_table* logic_player::get_table()
{
	return m_table;
}

void bmw_space::logic_player::set_serverflag(int32_t nflag)
{
	m_nServerFlag = nflag;
}

int32_t bmw_space::logic_player::get_serverflag() const
{
	return m_nServerFlag;
}

void logic_player::leave_table()
{
	// TODO 用户离开桌子
	uint32_t uid = get_pid();
	SLOG_DEBUG << boost::format("logic_player::leave_table >>>> :%1%") % uid;
	//
	if(m_table != nullptr)
	{
		//  Add fix invaild-leave of clt-send-leavegame 
		if (m_table->leave_table(uid) == false)
		{
			SLOG_WARNING << boost::format("logic_player::leave_table Invaild>>>> :%1%") % uid;
			return;
		}
		// reset
		m_table = nullptr;
		m_nChairID->set_value(INVALID_CHAIR);
		set_status(eUserState_null);
		clear_round();
		set_wait(0);
	}
}

bool logic_player::can_leave_table()
{
	if(m_table==nullptr) return true;

	return m_table->can_leave_table(get_pid());
}

void logic_player::release()
{
	leave_table();
	m_player.reset();
}


uint32_t logic_player::get_pid()
{
	return m_player->get_playerid();
}

GOLD_TYPE logic_player::get_gold()
{
	return m_player->get_attribute64(msg_type_def::e_itd_gold);
//	return m_logic_gold;
}

int logic_player::get_ticket()
{
	return m_player->get_attribute(msg_type_def::e_itd_ticket);
}

int16_t logic_player::get_viplvl()
{
	return m_player->get_attribute(msg_type_def::e_itd_vip);
}

bool logic_player::change_gold(GOLD_TYPE v, bool needbc)
{
	if(m_logic_gold + v >= 0)
	{
		if (m_logic_gold + v > MAX_MONEY)
			v = MAX_MONEY - m_logic_gold;
		if(v==0) return false;

		m_change_gold += v;
		m_logic_gold += v;
		if(needbc && m_table!=nullptr)
		{
			// TODO
		}
		return true;
	}
	
	return false;
}

bool logic_player::change_ticket(int v, int season)
{
	bool ret = m_player->change_ticket(v);
	if (ret)
	{
		//记录
		db_log::instance().property_log(this, game_engine::instance().get_gameid(), 
			msg_type_def::e_item_type_def::e_itd_ticket, v,0, season);
	}
	return ret;
}

bool logic_player::change_gold2(int v, int season)
{
	bool ret = m_player->change_gold(v);
	if (ret)
	{
		m_logic_gold += v;
		//记录
		db_log::instance().property_log(this, game_engine::instance().get_gameid(), 
			msg_type_def::e_item_type_def::e_itd_gold, v, 0,season);
	}
	return ret;
}

const std::string& logic_player::get_nickname()
{
	return m_player->get_nickname();
}

void logic_player::sync_change_gold()
{
	if(m_change_gold != 0)
	{
		if (!is_robot())
		{
			if (m_change_gold > 0)
			{
				add_star_lottery_info(0, 1);

				quest_change(win_gold, m_change_gold);

				//广播
				static int broadcast_gold = BMW_BaseInfo::GetSingleton()->GetData("BroadcastGold")->mValue;
				if (m_change_gold >= broadcast_gold)
				{
					const std::string *pStr = m_player->getLan("CowsBroadcast");
					if(pStr)
					{
						boost::format fmt = boost::format(pStr->c_str()) 
							% get_viplvl()
							% get_nickname() 
							% m_change_gold;
						m_player->gameBroadcast(fmt.str());
					}
				}
			}
		}
		if(m_player->change_gold(m_change_gold))
		{
			if (!is_robot())
			{
				//记录
				db_log::instance().property_log(this, game_engine::instance().get_gameid(), 
					1,//e_item_type_def::e_itd_gold
					m_change_gold,0,
					11//PropertyReasonType::type_reason_single_round_balance
					);
			}
			m_change_gold = 0;		
		}
	}	
	store_game_object();
}

void bmw_space::logic_player::write_property(int64_t value,int64_t tax /*= 0*/,const std::string& param/*=""*/)
{
	if(value==0) return;
	if(m_player->change_gold(value,tax))
	{
		db_log::instance().property_log(this, game_engine::instance().get_gameid(), 
				msg_type_def::e_item_type_def::e_itd_gold,
				value,
				tax,
				PropertyReasonType::type_reason_single_round_balance,
				param);
			
	}
	store_game_object();
	// broadcast
	if (is_robot())
	{
		static int robot_gold = BMW_BaseInfo::GetSingleton()->GetData("RobotBroadcastGold")->mValue;
		if (value < robot_gold) return;
		static int robot_rate = BMW_BaseInfo::GetSingleton()->GetData("RobotBroadcastRate")->mValue;
		if (global_random::instance().rand_1w() > robot_rate) return;

	}
	else
	{
		static int broadcast_gold = BMW_BaseInfo::GetSingleton()->GetData("BroadcastGold")->mValue;
		if (value < broadcast_gold) return;
		static int broadcast_rate = BMW_BaseInfo::GetSingleton()->GetData("BroadcastRate")->mValue;
		if (global_random::instance().rand_1w() > broadcast_rate) return;
	}
	// 
	const std::string *pStr = m_player->getLan("N_WinPrize4Others");
	if (pStr)
	{
		std::string strRoomNameInfo;
		if (m_table)
		{
			auto room = m_table->get_room();
			if (room) {
				strRoomNameInfo = room->get_roomcfg()->mRoomName;
			}
		}
		boost::format fmt;
		auto remon = value / 100.0;
		fmt = boost::format(pStr->c_str()) % GetUserRegion() % strRoomNameInfo.c_str() % remon;
		m_player->gameBroadcast(fmt.str());
	}
}

void bmw_space::logic_player::reqPlayer_leaveGame(int status/* = 0*/)
{
	m_player->reqPlayer_leaveGame(status);
}


int logic_player::get_sex()
{
	return m_player->get_attribute(msg_type_def::e_itd_sex);
}
int logic_player::get_photo_frame()
{
	return m_player->get_attribute(msg_type_def::e_itd_photoframe);
}

const std::string& logic_player::get_icon_custom()
{
	return m_player->get_icon_custom();
}

const std::string& bmw_space::logic_player::GetUserRegion()
{
	std::string *pStr = m_player->GetUserRegion();
	return *pStr;
}

bool logic_player::is_robot()
{
	return m_player->is_robot();
}

LRobotPtr& logic_player::get_robot()
{
	return m_robot;
}

void logic_player::create_robot()
{
	if(is_robot() && m_robot==nullptr)
	{
		m_robot = robot_player::malloc();
	//	m_robot->init(this);
	}
}

//////////////////////////////////////////////////////////////////////////
void logic_player::create_player()
{
	m_win_count->set_value(0);

	store_game_object(true);
}

bool logic_player::load_player()
{
	mongo::BSONObj b = db_game::instance().findone(DB_BMW_PLAYER, BSON("player_id" << get_pid()));
	if(b.isEmpty())
		return false;

	return from_bson(b);
}

void logic_player::init_game_object()
{	
	m_win_count = CONVERT_POINT(Tfield<int32_t>, regedit_tfield(e_got_int32, "win_count"));

	m_nChairID = CONVERT_POINT(Tfield<int32_t>,regedit_tfield(e_got_int32,"chair_id"));

	m_nPlayerStatus = CONVERT_POINT(Tfield<int32_t>,regedit_tfield(e_got_int32,"player_state"));

	m_nPlayRound = CONVERT_POINT(Tfield<int32_t>,regedit_tfield(e_got_int32,"player_round"));

	m_nWaitRound = CONVERT_POINT(Tfield<int32_t>,regedit_tfield(e_got_int32,"wait_round"));
}

bool logic_player::store_game_object(bool to_all)
{
	if(!has_update())
		return true;

	auto err = db_game::instance().update(DB_BMW_PLAYER, BSON("player_id" << get_pid()), BSON("$set"<<to_bson(to_all)));
	if(!err.empty())
	{
		SLOG_ERROR << "logic_player::store_game_object :" <<err;
		return false;
	}
	return true;
}

void logic_player::bc_game_msg(int money, const std::string& sinfo, int mtype)
{
	if(m_table)
	{
		auto rcfg = m_table->get_room()->get_data();
		m_player->game_broadcast(rcfg->mRoomName, 1, sinfo, money, mtype);
	}
}


void logic_player::add_star_lottery_info(int32_t award,int32_t star /*= 0*/)
{
	m_player->add_starinfo(award);

	m_win_count->add_value(star);
	static int WinStarCount = BMW_BaseInfo::GetSingleton()->GetData("WinStarCount")->mValue;
	if (m_win_count->get_value() > WinStarCount)
	{
		m_win_count->set_value(0);
		m_player->add_starinfo(0,1);
	}
}

void logic_player::quest_change(int questid, int count, int param)
{
	if (!is_robot())
	{
		m_player->quest_change(questid, count, param);
	}
}


int bmw_space::logic_player::send_packet(uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg)
{
	__ENTER_FUNCTION
	auto factory = packet_manager::instance().get_factroy(packet_id);
	if(factory != nullptr)
	{			
		factory->packet_process(nullptr, m_player, msg);
	}			
	return 0;
	__LEAVE_FUNCTION
	return -1;
}



int32_t bmw_space::logic_player::get_player_tag()
{
	// Tag -- 初入游戏
	/*
	Tag_Supper		110	// 超级用户
	Tag_UserA		10	// A 类玩家
	Tag_UserB		30	// B 类玩家
	Tag_UserC		60	// C 类玩家
	Tag_Robot		100
	*/
	// Label -- 在游戏中的值
	// Test
	int64_t lGold = get_gold();
	if(is_robot())
	{
		return Tag_Robot;
	}
	else if(1000144==get_pid() || 1000503==get_id())
	{
		return Tag_Supper;
	}
	else
	{
// 		int nRandom = global_random::instance().rand_100();
// 		if(nRandom<=10)
// 		{
// 			return Tag_UserA;
// 		}
// 		else if (nRandom<=40)
// 		{
// 			return Tag_UserB;
// 		}
// 		else
		{
			return Tag_UserC;
		}
	}

}

bool bmw_space::logic_player::is_payer()
{
	auto staticOptItems = m_player->GetOpItems();
	if (staticOptItems == nullptr) return false;
	int64_t rechargegold = staticOptItems->RecvGiftCoinTotal + staticOptItems->RechargeCoinTotal;
	if (rechargegold > 0)
	{
		return true;
	}
	return false;
}

bool bmw_space::logic_player::get_player_ctrl(tagPlayerCtrlAttr& attr,int64_t nIncome/*=0*/,int64_t nConsume/*=0*/)
{
	// 缺省参数
	uint32_t playerid = get_pid();
	attr.nTag = Tag_UserA;
	attr.fwinPercent = 50;
	// 机器人初始值
	if(is_robot())
	{
		attr.nTag = Tag_Robot;
		attr.fwinPercent = 0;
		return true;
	}
	auto staticOptItems = m_player->GetOpItems();
	if(staticOptItems==nullptr) return false;
	int64_t flagsX = staticOptItems->flagsX;
	int64_t safebag = staticOptItems->safeBag;
	int64_t gold = get_gold();
	int64_t holdgold = safebag + gold;
	int64_t exchangegold = staticOptItems->SendGiftCoinCount + staticOptItems->withDrawCoinTotal;
	int64_t rechargegold = staticOptItems->RecvGiftCoinTotal + staticOptItems->RechargeCoinTotal;
	int64_t nAnly = flagsX + holdgold + exchangegold - rechargegold;

	auto mapOptItems = staticOptItems->opItems;
	auto it = mapOptItems.find(GAME_ID);
	if(it == mapOptItems.end())
	{
	//	assert(0);
		SLOG_WARNING<<boost::format("get_player_ctrl:playerid:%d,can't find game-id:%d")%playerid%GAME_ID;
		return false;
	}
	auto optItems = it->second;
	bool bSupper = optItems->flagsS;
	// 超级用户
	if(bSupper)
	{
		attr.nTag = Tag_Supper;
		attr.fwinPercent = 100;
		return true;
	}
	// refetch
	nAnly = nAnly + nIncome - nConsume;
	// 运维标签
	int32_t GMFlags = optItems->gmOpFlags;
	if(GMFlags>0)
	{
		nAnly = GMFlags;
	}
	int32_t flagsA = optItems->flagsA;
	int32_t flagsB = optItems->flagsB;
	int32_t flagsC = optItems->flagsC;

	double winsPercentA = optItems->winsPercentA;
	double winsPercentB = optItems->winsPercentB;
	double winsPercentC = optItems->winsPercentC;
	SLOG_CRITICAL<<boost::format("================================");
	SLOG_CRITICAL<<boost::format("id:%d")%playerid;
	SLOG_CRITICAL<<boost::format("flagsX:%1%,safebag:%2%,gold:%3%")%flagsX%safebag%gold;
	SLOG_CRITICAL<<boost::format("holdgold:%d,exchangegold:%d,rechargegold:%d")%holdgold%exchangegold%rechargegold;
	SLOG_CRITICAL<<boost::format("Supper:%d")%((int32_t)bSupper);
	SLOG_CRITICAL<<boost::format("GMFlags:%d")%GMFlags;
	SLOG_CRITICAL<<boost::format("nAnly:%d")%nAnly;
	SLOG_CRITICAL<<boost::format("flagsA:%d,Percent:%f")%flagsA%winsPercentA;
	SLOG_CRITICAL<<boost::format("flagsB:%d,Percent:%f")%flagsB%winsPercentB;
	SLOG_CRITICAL<<boost::format("flagsC:%d,Percent:%f")%flagsC%winsPercentC;
	SLOG_CRITICAL<<boost::format("================================");
	// 系数
	double opCoeff = optItems->opCoefficient;
	// 
	int64_t nCalcCoeff = rechargegold - exchangegold;
	if(nCalcCoeff*opCoeff <= flagsB)
	{
		nCalcCoeff = flagsB;
	}

	if(nAnly <= flagsA)
	{
		attr.nTag = Tag_UserA;
		attr.fwinPercent = winsPercentA;
		return true;
	}
	else if(nAnly <= nCalcCoeff)
	{
		attr.nTag = Tag_UserB;
		attr.fwinPercent = winsPercentB;
		return true;
	}
	else
	{
		attr.nTag = Tag_UserC;
		attr.fwinPercent = winsPercentC;
		return true;
	}
	return false;
}



bool logic_player::onEventUserReady()
{
// 	if(get_status()>eUserState_free)
// 	{
// 		SLOG_CRITICAL<<"logic_player::onEventUserReady Can't Ready:usestate";
// 		return false;
// 	}
// 	if(m_table && m_table->get_status()!=eGameState_Free)
// 	{
// 		SLOG_CRITICAL<<"logic_player::onEventUserReady Can't Ready:gamestate";
// 		return false;
// 
// 	}

	return m_table->onEventUserReady(get_pid());
}

int logic_player::get_roomid()
{
	if (m_table)
	{
		if (m_table->get_room())
		{
			return m_table->get_room()->get_id();
		}
	}
	return 0;
}