#include "stdafx.h"
#include "logic_player.h"
#include <i_game_player.h>
#include "game_db.h"
#include "game_db_log.h"
#include "logic_room.h"
#include "logic_main.h"
#include "logic_robot.h"
#include "enable_crypto.h"
#include "proc_dragon_tiger_logic.h"
#include "proc_dragon_tiger_protocol.h"
#include "msg_info_def.pb.h"
#include "msg_info_def_ex.pb.h"
#include "DragonTiger_RoomCFG.h"
#include "DragonTiger_BaseInfo.h"
#include "game_engine.h"

#include "DragonTiger_BaseInfo.h"

DRAGON_TIGER_SPACE_USING

dragon_tiger_space::logic_player::logic_player(void)
:m_lobby(nullptr)
,m_room(nullptr)
,m_logic_gold(0)
,m_change_gold(0)
,m_bforce(0)
,m_leave_status(0)
{
	init_game_object();
	m_record_banker_count = 0;
	m_balance_bet_gold = 0;
	m_balance_win_or_lose = 0;
	m_win_lose_final_gold = 0;
	m_balance_bet = false;
	m_list_bet_gold.clear();
	m_list_winner.clear();

	m_robot_bet = false;

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

void logic_player::on_change_state()
{

}

int logic_player::cltReq_leaveGame()
{
	if (can_leave_room() == true)
	{
		getIGamePlayer()->reqPlayer_leaveGame();
		return 1;
	}
	else
	{
		m_leave_status = 3;
		return 2;
	}
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
	record_gold();

}

bool logic_player::enter_room(logic_room* room)
{
	assert(m_room == nullptr);
	m_room = room;

	return true;
}

bool logic_player::can_leave_room()
{
	if (m_room == nullptr)
	{
		return true;
	}
	if (get_balance_bet())
	{
		return false;
	}
	
	return m_room->get_game_main()->can_leave_room(this->get_pid());
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
		int cut_tag = 0;
		if (m_room && m_room->get_game_main())
		{
			cut_tag = m_room->get_game_main()->get_cut_round_tag();
		}
			
		//记录
		db_log::instance().property_log(this, game_engine::instance().get_gameid(), 
			msg_type_def::e_item_type_def::e_itd_ticket, v, 0, cut_tag, season);
	}
	return ret;
}

bool logic_player::change_gold2(int v, int season)
{
	bool ret = m_player->change_gold(v);
	if (ret)
	{
		int cut_tag = 0;
		if (m_room && m_room->get_game_main())
		{
			cut_tag = m_room->get_game_main()->get_cut_round_tag();
		}

		m_logic_gold += v;
		//记录
		db_log::instance().property_log(this, game_engine::instance().get_gameid(), 
			msg_type_def::e_item_type_def::e_itd_gold, v, 0, cut_tag, season);
	}
	return ret;
}

const std::string& logic_player::get_nickname()
{
	return m_player->get_nickname();
}

const std::string logic_player::get_region()
{
	return *(m_player->GetUserRegion());
}

void logic_player::sync_change_gold()
{
	if(m_change_gold != 0)
	{
		if (!is_robot())
		{
			if (m_change_gold > 0)
			{
				//add_star_lottery_info(0, 1);

				quest_change(win_gold, m_change_gold);

				//广播
// 				static int broadcast_gold = DragonTiger_BaseInfo::GetSingleton()->GetData("BroadcastGold")->mValue;
// 				if (m_change_gold >= broadcast_gold)
// 				{
// 					const std::string *pStr = m_player->getLan("CowsBroadcast");
// 					if(pStr)
// 					{
// 						boost::format fmt = boost::format(pStr->c_str()) 
// 							% get_viplvl()
// 							% get_nickname() 
// 							% m_change_gold;
// 						m_player->gameBroadcast(fmt.str());
// 					}
// 				}
			}
		}
		if(m_player->change_gold(m_change_gold))
		{
			if (!is_robot())
			{
				int cut_tag = 0;
				if (m_room && m_room->get_game_main())
				{
					cut_tag = m_room->get_game_main()->get_cut_round_tag();
				}

				//记录
				db_log::instance().property_log(this, game_engine::instance().get_gameid(), 
					1,//e_item_type_def::e_itd_gold
					m_change_gold, m_tax_gold, cut_tag,
					11//PropertyReasonType::type_reason_single_round_balance
					);
			}
			m_change_gold = 0;		
		}
	}	
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
	//m_win_count->set_value(0);
}

bool logic_player::load_player()
{
	mongo::BSONObj b = db_game::instance().findone(DB_DRAGON_TIGER_PLAYER, BSON("player_id" << get_pid()));
	if(b.isEmpty())
		return false;

	return from_bson(b);
}

void logic_player::init_game_object()
{	
	//m_win_count = CONVERT_POINT(Tfield<int32_t>, regedit_tfield(e_got_int32, "win_count"));
}

bool logic_player::store_game_object(bool to_all)
{
	if(!has_update())
		return true;

	auto err = db_game::instance().update(DB_DRAGON_TIGER_PLAYER, BSON("player_id" << get_pid()), BSON("$set"<<to_bson(to_all)));
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

	static int betRate = DragonTiger_BaseInfo::GetSingleton()->GetData("BetRate")->mValue;
	if (get_gold() < (total_gold + bet_gold) * betRate)
	{
		return false;
	}
	return true;
}

// void logic_player::add_star_lottery_info(int32_t award,int32_t star /*= 0*/)
// {
// 	m_player->add_starinfo(award);
// 
// 	m_win_count->add_value(star);
// 	static int WinStarCount = DragonTiger_BaseInfo::GetSingleton()->GetData("WinStarCount")->mValue;
// 	if (m_win_count->get_value() > WinStarCount)
// 	{
// 		m_win_count->set_value(0);
// 		m_player->add_starinfo(0,1);
// 	}
// }

void logic_player::quest_change(int questid, int count, int param)
{
	if (!is_robot())
	{
		m_player->quest_change(questid, count, param);
	}
}

void logic_player::quest_change_from_world(int quest_type,int count,int param)
{
	//SLOG_ERROR<<"logic_player::quest_change_from_world quest_type="<<quest_type<<";count="<<count<<";param="<<param;
}

int logic_player::get_roomid()
{
	if (m_room!=nullptr)
		return m_room->get_room_id();
	return 0;
}

GOLD_TYPE dragon_tiger_space::logic_player::get_bet_gold()
{
	GOLD_TYPE total_bet_gold = 0;

	std::list<GOLD_TYPE>::iterator it = m_list_bet_gold.begin();
	for ( ; it != m_list_bet_gold.end(); it++)
	{
		total_bet_gold += *it;
	}

	return total_bet_gold;

}

int dragon_tiger_space::logic_player::get_winner_count()
{
	int winner_count = 0;
	std::list<bool>::iterator it = m_list_winner.begin();
	for ( ; it != m_list_winner.end(); it++)
	{
		if (*it == true)
		{
			winner_count++;
		}
	}

	return winner_count;
}

std::list<GOLD_TYPE> & dragon_tiger_space::logic_player::get_bet_gold_list()
{
	return m_list_bet_gold;
}

std::list<bool>& dragon_tiger_space::logic_player::get_winner_list()
{
	return m_list_winner;
}

void dragon_tiger_space::logic_player::clear_balace_info()
{
	m_balance_bet_gold = 0;
	m_balance_win_or_lose = 0;
	m_win_lose_final_gold = 0;
	m_balance_bet = false;
}

void dragon_tiger_space::logic_player::store_balance_record()
{
	m_list_bet_gold.push_front(m_balance_bet_gold);

	if (m_list_bet_gold.size() > 20 )
		m_list_bet_gold.pop_back();
	
	if(m_win_lose_final_gold > 0)
		m_list_winner.push_front(true);
	else
		m_list_winner.push_front(false);

	if (m_list_winner.size() > 20)
		m_list_winner.pop_back();

	//test
	//int atest = get_bet_gold();
	//int btest = get_winner_count();

	return;
}
