#include "stdafx.h"
#include "logic_player.h"
#include <i_game_player.h>
#include "game_db.h"
#include "game_db_log.h"
#include "logic_room.h"
#include "logic_main.h"
#include "logic_robot.h"
#include "enable_random.h"
#include "proc_cows_logic.h"
#include "proc_cows_protocol.h"

#include "Cows_RoomCFG.h"
#include "Cows_BaseInfo.h"
#include "game_engine.h"

#include "Cows_BaseInfo.h"
#include "M_MultiLanguageCFG.h"

#include "pump_type.pb.h"

COWS_SPACE_USING

cows_space::logic_player::logic_player(void)
:m_lobby(nullptr)
,m_room(nullptr)
,m_logic_gold(0)
,m_change_gold(0)
,m_self_is_bet(false)
,m_self_win_gold(0)
, m_bforce(0)
, m_tax_gold(0)
{
	init_game_object();
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

void logic_player::quest_change_from_world(int quest_type,int count,int param)
{

}

void logic_player::on_change_state()
{

}

//////////////////////////////////////////////////////////////////////////
void logic_player::heartbeat( double elapsed )
{
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

bool logic_player::enter_room(logic_room* room)
{
	assert(m_room == nullptr);
	m_room = room;

	return true;
}

int logic_player::can_leave_room()
{
	if (m_room == nullptr)
	{
		return true;
	}

	auto game_main = m_room->get_game_main();
	int nRet=0;
	if (game_main->can_leave_room(this->get_pid()))
	{
		nRet=0;
	}
	else
	{
		nRet=53;
		return nRet;
	}

	if (game_main->get_player_betgold(m_room->get_player(this->get_pid()))==0)
	{
		nRet=0;
	}
	else
	{
		nRet=2;
	}

	return nRet;
}

void logic_player::leave_room()
{
	if (m_room != nullptr)
	{
		m_room->leave_room(get_pid());
		m_room = nullptr;
	}

	sync_change_gold();

	store_game_object();
}

void logic_player::release()
{
	leave_room();
	m_player.reset();
}

bool logic_player::is_offline()
{
	return m_player->get_state() != e_ps_playing;
}

uint32_t logic_player::get_pid()
{
	return m_player->get_playerid();
}

GOLD_TYPE logic_player::get_gold()
{
	return m_logic_gold;
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
	if(-v <= m_logic_gold)
	{
		if(m_logic_gold > MAX_MONEY - v)
			v = MAX_MONEY - m_logic_gold;

		m_change_gold += v;
		m_logic_gold += v;

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
			msg_type_def::e_item_type_def::e_itd_ticket, v, 0, season);
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
			msg_type_def::e_item_type_def::e_itd_gold, v, 0, season);
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
		//if (!is_robot())
		//{
			if (m_change_gold > 0)
			{
				add_star_lottery_info(0, 1);

				quest_change(win_gold, m_change_gold);

				//广播
                int broadcastgold = 0;
                bool broadcast = true;
                if (this->is_robot())
                {
                    static int broadcast_gold = Cows_BaseInfo::GetSingleton()->GetData("RobotBroadcastGold")->mValue;
                    static int broadcast_rate = Cows_BaseInfo::GetSingleton()->GetData("RobotBroadcastRate")->mValue;
                    broadcastgold = broadcast_gold;
                    int rate = global_random::instance().rand_100();
                    if (rate >  broadcast_rate)
                    {
                        broadcast = false;
                    }
                }
                else
                {
                    static int broadcast_gold = Cows_BaseInfo::GetSingleton()->GetData("BroadcastGold")->mValue;
                    static int broadcast_rate = Cows_BaseInfo::GetSingleton()->GetData("BroadcastRate")->mValue;

                    broadcastgold = broadcast_gold;
                    int rate = global_random::instance().rand_100();
                    if (rate > broadcast_rate)
                    {
                        broadcast = false;
                    }
                }
				if (this->get_self_win_gold() >= broadcastgold && broadcast)
				{
                    i_game_player *iPlayer = this->getIGamePlayer();
                    const std::string *pStr = iPlayer->getLan("N_WinPrize4Others");
                    if (iPlayer && pStr)
                    {
                        boost::format fmt;
                        double remon = (double)this->get_self_win_gold() / 100;
                        std::string room = m_room->get_name();
                        std::string& region = *iPlayer->GetUserRegion();
                        fmt = boost::format(pStr->c_str()) % region % room %  remon;
                        iPlayer->gameBroadcast(fmt.str());
                    }
                    //const std::string *pStr = m_player->getLan("CowsBroadcast");
                    //if(pStr)
                    //{
                    //	/*
                    //	boost::format fmt = boost::format(pStr->c_str()) 
                    //		% get_viplvl()
                    //		% get_nickname() 
                    //		% m_change_gold;*/
                    //	boost::format fmt = boost::format(pStr->c_str()) 
                    //		% GetUserRegion() 
                    //		% (m_change_gold/100.0);
                    //	m_player->gameBroadcast(fmt.str());
                    //}
				}
			}
		//}

		if(m_player->change_gold(m_change_gold, m_tax_gold))
		{
			if (!is_robot())
			{
				//记录
				db_log::instance().property_log(this, game_engine::instance().get_gameid(), 
					1,//e_item_type_def::e_itd_gold
					m_change_gold, m_tax_gold,
					11//PropertyReasonType::type_reason_single_round_balance
					);
			}
		}
	}	
	m_change_gold = 0;
	//m_tax_gold = 0;
	store_game_object();
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

const std::string& logic_player::GetUserRegion()
{
	std::string *pStr = m_player->GetUserRegion();
	return *pStr;
}

logic_room* logic_player::get_room()
{
	return m_room;
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
	if (is_robot() && m_robot == nullptr)
	{
		m_robot = logic_robot::malloc();
		m_robot->init(this);
	}
}

//////////////////////////////////////////////////////////////////////////
void logic_player::create_player()
{
	m_win_count->set_value(0);
}

bool logic_player::load_player()
{
	mongo::BSONObj b = db_game::instance().findone(DB_COWS_PLAYER, BSON("player_id" << get_pid()));
	if(b.isEmpty())
		return false;

	return from_bson(b);
}

void logic_player::init_game_object()
{	
	m_win_count = CONVERT_POINT(Tfield<int32_t>, regedit_tfield(e_got_int32, "win_count"));
}

bool logic_player::store_game_object(bool to_all)
{
	if(!has_update())
		return true;

	auto err = db_game::instance().update(DB_COWS_PLAYER, BSON("player_id" << get_pid()), BSON("$set"<<to_bson(to_all)));
	if(!err.empty())
	{
		SLOG_ERROR << "logic_player::store_game_object :" <<err;
		return false;
	}
	return true;
}

void logic_player::bc_game_msg(int money, const std::string& sinfo, int mtype)
{
	if(m_room)
	{
		auto rcfg = m_room->get_data();
		m_player->game_broadcast(rcfg->mRoomName, 1, sinfo, money, mtype);
	}
}

bool logic_player::check_bet_gold(GOLD_TYPE bet_gold, GOLD_TYPE total_gold)
{
	if (get_gold() < bet_gold)
	{
		return false;
	}

	static int betRate = Cows_BaseInfo::GetSingleton()->GetData("BetRate")->mValue;
	if (get_gold() < (total_gold + bet_gold) * betRate)
	{
		return false;
	}
	return true;
}

void logic_player::add_star_lottery_info(int32_t award,int32_t star /*= 0*/)
{
	m_player->add_starinfo(award);

	m_win_count->add_value(star);
	static int WinStarCount = Cows_BaseInfo::GetSingleton()->GetData("WinStarCount")->mValue;
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

void cows_space::logic_player::set_tax(GOLD_TYPE tax)
{
	m_tax_gold = tax;
	m_logic_gold -= tax;
	m_change_gold -= tax;
}

GOLD_TYPE cows_space::logic_player::get_pre_gold()
{
	return m_pre_logic_gold;
}

void cows_space::logic_player::record_gold()
{
	m_tax_gold = 0;
	m_pre_logic_gold = m_logic_gold;
}

GOLD_TYPE logic_player::get_tax()
{
	return m_tax_gold;
}

void logic_player::set_kick_status(int bforce)
{
	m_bforce = bforce;
}

int logic_player::get_kick_status()
{
	return m_bforce;
}

