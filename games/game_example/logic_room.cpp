#include "stdafx.h"
#include "logic_room.h"

#include "logic_table.h"
#include "game_db.h"

EXAMPLE_SPACE_USING

logic_room::logic_room(/*const Fish_RoomCFGData* cfg,*/ logic_lobby* _lobby)
:m_player_count(0)
,m_check_rate(0)
{
	init_game_object();
	m_lobby = _lobby;
	//m_cfg = cfg;
	//RoomID->set_value(m_cfg->mRoomID);
	//for (uint16_t i = 1; i<= m_cfg->mTableCount;i++)
	//{
	//	auto table = logic_table::malloc();
	//	table->init_talbe(i, this);

	//	m_tables.insert(std::make_pair(i, table));
	//}

	//m_key = "room_"+ boost::lexical_cast<std::string>(m_cfg->mRoomID);

	
	if(!load_room())
		create_room();
}


logic_room::~logic_room(void)
{
}

uint32_t logic_room::get_id()
{
	return RoomID->get_value();
}


void logic_room::heartbeat( double elapsed )
{
	if(m_player_count==0)
		return;

	m_check_rate += elapsed;
	if(m_check_rate > 30)//30秒刷新1次 盈利率
	{
		reflush_rate();
		store_game_object();
		m_check_rate = 0;
	}

	for (auto it = m_tables.begin(); it != m_tables.end(); ++it)
	{
		it->second->heartbeat(elapsed);
	}
}

uint16_t logic_room::get_cur_cout()
{
	return m_player_count;
}

bool logic_room::has_seat(uint16_t& tableid)
{
	for(auto it = m_tables.begin(); it != m_tables.end();++it)
	{
		if(!it->second->is_full())
		{
			if(tableid != 0 && tableid == it->first)
				continue;

			tableid = it->first;
			return true;
		}
	}
	return false;
}

int logic_room::enter_table(LPlayerPtr player, uint16_t tid)
{
	auto table = m_tables.find(tid);
	if(table == m_tables.end())
		return 2;

	int ret = table->second->enter_table(player);

	if(ret == 1)
	{
		m_player_count++;
		EnterCount->add_value(1);
	}

	return ret;
}

void logic_room::on_leave_table()
{
	m_player_count--;
}

void logic_room::check_rate(uint32_t& rate)
{
	//if (rate < m_cfg->mMinRate)
	//{
	//	rate = m_cfg->mMinRate;
	//}
	//else if (rate > m_cfg->mMaxRate)
	//{
	//	rate = m_cfg->mMaxRate;
	//}
}

int logic_room::get_max_rate()
{
	return 0;//m_cfg->mMaxRate;
}

int logic_room::get_min_rate()
{
	return 0;//m_cfg->mMinRate;
}
//
//const Fish_RoomCFGData* logic_room::get_data() const
//{
//	return m_cfg;
//}

double logic_room::get_earnings_rate()
{
	return EarningsRate->get_value();
}

void logic_room::reflush_rate()
{
	//static auto ff = BSON("EarningsRate"<<1<<"TotalIncome"<<1<<"TotalOutlay"<<1);
	//mongo::BSONObj b = db_game::instance().findone(DB_FISHROOM, BSON("room_id"<<RoomID->get_value()), &ff);
	//if(!b.isEmpty() && b.hasField("EarningsRate"))
	//{
	//	EarningsRate->set_value(b.getField("EarningsRate").Double(), false);

	//	if(b.getField("TotalIncome").Long() <0 && b.getField("TotalOutlay").Long()<0)
	//	{
	//		TotalIncome->set_value(0);
	//		TotalOutlay->set_value(0);
	//	}
	//}	
}

//////////////////////////////////////////////////////////////////////////
void logic_room::create_room()
{
	EarningsRate->set_value(0.05);//默认值
	TotalIncome->set_value(0);
	TotalOutlay->set_value(0);
	EnterCount->set_value(0);
}
bool logic_room::load_room()
{
	//mongo::BSONObj b = db_game::instance().findone(DB_FISHROOM, BSON("room_id"<<RoomID->get_value()));
	//if(b.isEmpty())
	//	return false;	

	//return from_bson(b);
	return false;
}
void logic_room::init_game_object()
{
	RoomID = CONVERT_POINT(Tfield<int16_t>, regedit_tfield(e_got_int16, "room_id"));
	EarningsRate = CONVERT_POINT(Tfield<double>, regedit_tfield(e_got_double, "EarningsRate"));
	TotalIncome = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "TotalIncome"));
	TotalOutlay = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "TotalOutlay"));
	EnterCount = CONVERT_POINT(Tfield<int64_t>, regedit_tfield(e_got_int64, "EnterCount"));
}
bool logic_room::store_game_object(bool to_all)
{
	if(!has_update())
		return true;

	//auto err = db_game::instance().update(DB_FISHROOM, BSON("room_id"<<RoomID->get_value()), BSON("$set"<<to_bson(to_all)));
	//if(!err.empty())
	//{
	//	SLOG_ERROR << "logic_room::store_game_object :" <<err;
	//	return false;
	//}

	return true;
}

int logic_room::get_item_consume(int itemid)
{
	//if(m_cfg != nullptr)
	//{
	//	switch (itemid)
	//	{
	//	case 10://定时
	//		return m_cfg->mTimingConsume;		
	//	case 11://能量
	//		return m_cfg->mPowerConsume;
	//	case 12://激光
	//		return m_cfg->mLaserConsume;
	//	}	
	//}
	//	
	return 1;
}