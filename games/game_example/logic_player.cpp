#include "stdafx.h"
#include "logic_player.h"
#include "logic_table.h"

#include "game_db.h"
#include <net\packet_manager.h>
#include "fish_logic.pb.h"
#include "logic_room.h"
#include "game_db_log.h"
#include <time_helper.h>
#include "game_engine.h"
#include "logic_room.h"
#include <enable_random.h>
#include "M_VIPProfitCFG.h"


using namespace boost;

EXAMPLE_SPACE_USING


logic_player::logic_player(void)
:m_table(nullptr)
,m_lobby(nullptr)
,m_logic_gold(0)
,m_change_gold(0)
,m_check_sync(0)
,m_isRobot(false)
,m_log_gold(0)
,m_check_ticket(0)
,m_cur_tickettime(0)
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

	//if( m_table != nullptr)
	//{			
	//	auto sendmsg = PACKET_CREATE(fish_protocols::packetl2c_bc_change_attr, fish_protocols::e_mst_l2c_bc_change_attr);
	//	sendmsg->set_player_id(get_pid());	
	//	if(atype == msg_type_def::e_itd_vip )			
	//		sendmsg->set_change_vip(v);					
	//	else if(atype == msg_type_def::e_itd_gold)		
	//		sendmsg->set_change_gold(v);		
	//	else if(atype == msg_type_def::e_itd_ticket)
	//		sendmsg->set_change_ticket(v);
	//	else
	//		return;

	//	m_table->add_msg_to_list(sendmsg);
	//}
}

void logic_player::on_offline()
{

}

//////////////////////////////////////////////////////////////////////////
void logic_player::heartbeat( double elapsed )
{
	m_check_sync += elapsed;
	if (m_check_sync > 60)//秒  1分钟同步1次
	{
		if(m_change_gold!=0 &&m_player->change_gold(m_change_gold))
		{
			m_change_gold = 0;		
		}

		m_check_sync = 0;
	}

	if(get_table() != nullptr)
		m_cur_tickettime += elapsed;
}

void logic_player::enter_game(logic_lobby* lobby)
{
	m_lobby = lobby;
	if(!load_player())
		create_player();

	m_logic_gold = m_player->get_attribute(msg_type_def::e_itd_gold);
}

bool logic_player::join_table(logic_table* table)
{
	if(m_table != nullptr && table != nullptr)
		return false;

	m_table = table;	
	m_log_gold = 0;

	if(m_check_ticket <=0)
		init_tickettime();

	return true;
}

uint32_t logic_player::get_pid()
{
	return m_player->get_playerid();
}

int logic_player::get_gold()
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

bool logic_player::is_ExperienceVIP()
{
	return m_player->is_ExperienceVIP();
}

bool logic_player::change_gold(int v, bool needbc)
{
	if(-v <= m_logic_gold)
	{
		if(m_logic_gold > MAX_MONEY - v)
			v = MAX_MONEY - m_logic_gold;

		if(v==0)
			return false;

		m_change_gold += v;
		m_logic_gold += v;
		m_log_gold += v;

		//if(needbc && m_table != nullptr)
		//{			
		//	auto sendmsg = PACKET_CREATE(fish_protocols::packetl2c_bc_change_attr, fish_protocols::e_mst_l2c_bc_change_attr);
		//	sendmsg->set_player_id(get_pid());	
		//	sendmsg->set_change_gold(v);
		//	m_table->add_msg_to_list(sendmsg);
		//}
		return true;
	}
	return false;
}

bool logic_player::change_ticket(int v, bool needbc)
{
	//if(needbc && m_table != nullptr)
	//{			
	//	auto sendmsg = PACKET_CREATE(fish_protocols::packetl2c_bc_change_attr, fish_protocols::e_mst_l2c_bc_change_attr);
	//	sendmsg->set_player_id(get_pid());	
	//	sendmsg->set_change_ticket(v);
	//	m_table->add_msg_to_list(sendmsg);
	//}

	return m_player->change_ticket(v);
}

const std::string& logic_player::get_nickname()
{
	return m_player->get_nickname();
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

logic_table* logic_player::get_table()
{
	return m_table;
}

void logic_player::leave_table()
{
	if(m_change_gold != 0)
	{
		if(m_player->change_gold(m_change_gold))
		{
			m_change_gold = 0;		
		}
	}	

	store_game_object();

	if(m_table != nullptr)
	{
		////记录
		//boost::format fmt = boost::format("roomid:%1%, tableid:%2%")%m_table->get_room()->get_id()%m_table->get_id();

		//db_log::instance().property_log(this, game_engine::instance().get_gameid(), 
		//	1,//e_item_type_def::e_itd_ticket/gold
		//	m_log_gold,
		//	11,//PropertyReasonType::type_reason_fish_uplevel
		//	fmt.str()
		//	);


		m_table->leave_table(get_pid());
		m_table = nullptr;
	}	
}

void logic_player::release()
{
	leave_table();
	m_player.reset();
}


//////////////////////////////////////////////////////////////////////////
void logic_player::create_player()
{

}

bool logic_player::load_player()
{
	//mongo::BSONObj b = db_game::instance().findone(DB_FISHLORD, BSON(DB_PLAYER_INDEX << get_pid()));
	//if(b.isEmpty())
	//	return false;

	//return from_bson(b);
	return false;
}

void logic_player::init_game_object()
{	
	KillFishCount = CONVERT_POINT(Tfield<int32_t>, regedit_tfield(e_got_int32, "KillFishCount"));
	KillFishScore = CONVERT_POINT(Tfield<int32_t>, regedit_tfield(e_got_int32, "KillFishScore"));
	Level = CONVERT_POINT(Tfield<int16_t>, regedit_tfield(e_got_int16, "Level"));
	Exp = CONVERT_POINT(Tfield<int32_t>, regedit_tfield(e_got_int32, "Exp"));
	
	TodayTicket = CONVERT_POINT(Tfield<int32_t>, regedit_tfield(e_got_int32, "TodayTicket"));
	LastGetTicket = CONVERT_POINT(Tfield<time_t>, regedit_tfield(e_got_date, "LastGetTicket"));
	ReceiveTicket = CONVERT_POINT(Tfield<bool>, regedit_tfield(e_got_bool, "ReceiveTicket"));
}

bool logic_player::store_game_object(bool to_all)
{
	if(!has_update())
		return true;

	//auto err = db_game::instance().update(DB_FISHLORD, BSON(DB_PLAYER_INDEX << get_pid()), BSON("$set"<<to_bson(to_all)));
	//if(!err.empty())
	//{
	//	SLOG_ERROR << "logic_player::store_game_object :" <<err;
	//	return false;
	//}
	return true;
}

void logic_player::addexp(int exp)
{
	//auto pl = Fish_LevelCFG::GetSingleton()->GetData(Level->get_value());
	//if(pl && pl->mNeedExp>0)
	//{
	//	if(pl->mNeedExp > Exp->get_value() + exp)
	//		Exp->add_value(exp);
	//	else
	//		Exp->set_value(pl->mNeedExp);
	//}
}



void logic_player::bc_game_msg(int money, const std::string& sinfo, int mtype)
{
	//if(m_table)
	//{
	//	auto rcfg = m_table->get_room()->get_data();
	//	m_player->game_broadcast(rcfg->mRoomName, 1, sinfo, money, mtype);
	//}
}

void logic_player::init_tickettime()
{
	//static int MinTicketTime = Fish_BaseInfo::GetSingleton()->GetData("MinTicketTime")->mValue;
	//static int MaxTicketTime = Fish_BaseInfo::GetSingleton()->GetData("MaxTicketTime")->mValue;
	//m_check_ticket = global_random::instance().rand_int(MinTicketTime, MaxTicketTime);
	m_cur_tickettime = 0;
}

int logic_player::check_ticket()
{
	auto today = time_helper::instance().get_cur_date();
	auto lastt = time_helper::convert_to_date(LastGetTicket->get_value());
	if(today != lastt)
	{
		TodayTicket->set_value(0);
		LastGetTicket->set_value(time_helper::convert_from_date(today));
	}

	auto vipdata = M_VIPProfitCFG::GetSingleton()->GetData(get_viplvl());
	if(vipdata == nullptr)
		return 0;

	if(TodayTicket->get_value() < vipdata->mGiveTicket && m_cur_tickettime>m_check_ticket && m_check_ticket>0)
	{
		int temp = global_random::instance().rand_int(0,2);

		int ticket = 0;

		if(temp == 0)
			ticket = 5;
		else if(temp == 1)
			ticket = 10;
		else
			ticket = 15;

		if(TodayTicket->get_value()+ticket > vipdata->mGiveTicket)
			ticket = vipdata->mGiveTicket-TodayTicket->get_value();

		change_ticket(ticket);

		init_tickettime();
		TodayTicket->add_value(ticket);

		return ticket;
	}

	return 0;
}
