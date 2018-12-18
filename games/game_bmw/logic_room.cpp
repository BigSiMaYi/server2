#include "stdafx.h"
#include "logic_room.h"
#include "BMW_RoomCFG.h"
#include "BMW_RobotCFG.h"
#include "game_db.h"
#include "logic_player.h"


#include "game_engine.h"
#include "i_game_ehandler.h"
#include "enable_random.h"

#include "logic_table.h"


#include "logic_player.h"
#include "robot_player.h"

//@add by Big O 2017/01/08;
//@在线统计增加;
#include "game_db_log.h"

BMW_SPACE_USING

logic_room::logic_room(const BMW_RoomCFGData* cfg, logic_lobby* _lobby, int childid)
:m_cfg(cfg)
,m_lobby(_lobby)
,m_checksave(0.0)
,m_childid(childid)
,m_service(1)
,m_killer(11)
,m_cutRound(0)
{
	init_game_object();
	PlayerCount->set_value(0);
	m_pids.clear();
	m_players.clear();
	m_tables.clear();

	RoomID->set_value(cfg->mRoomID);
	// 创建 tables
	int tableCount =  m_cfg->mTableCount;
	if(tableCount!=1) tableCount = 1;
	int32_t maxplayers = m_cfg->mMaxPlayers;
	if(maxplayers==0) maxplayers = GAME_PLAYER;
	m_nMaxPlayerCount = maxplayers;
	for (uint16_t i=1;i<=tableCount;i++)
	{
		auto table = logic_table::malloc();
		table->init_table(i,maxplayers,this);
		m_tables.insert(std::make_pair(i,table));
	}
	printf("roomid=%d ,tables >>>> %zd \n",RoomID->get_value(),m_tables.size());

	if(!load_room())
		create_room();

	// read robot cfg
	read_robotcfg();
}

void logic_room::release()
{
	for (auto it = m_tables.begin(); it != m_tables.end(); ++it)
	{
		it->second->release();
	}

	m_tables.clear();
	store_game_object();
	m_cfg = nullptr;
	m_lobby = nullptr;
}

logic_room::~logic_room(void)
{
	
}

const BMW_RoomCFGData* logic_room::get_roomcfg()
{
	return m_cfg;
}

void logic_room::setservice(int service /*= 0*/)
{
	m_service = service;
}

int logic_room::getservice() const
{
	return m_service;
}

void bmw_space::logic_room::setkiller(int killer /*= 0*/,int cutRound/* = 0*/)
{
	m_killer = killer;
	m_cutRound = cutRound;
}

int bmw_space::logic_room::getkiller() const
{
	return m_killer;
}

int bmw_space::logic_room::getCutRound() const
{
	return m_cutRound;
}

uint32_t logic_room::get_id()
{
	return m_cfg->mRoomID;
}

void logic_room::heartbeat( double elapsed )
{
	// add robot
	robot_heartbeat(elapsed);

	if(m_players.size() == 0) 
		return;

	// 刷新某项数据
	m_checksave += elapsed;
	if (m_checksave > 30)
	{
	
		store_game_object();
		m_checksave=0.0;
	}
	// 桌子更新
	for (auto it = m_tables.begin(); it != m_tables.end(); ++it)
	{
		it->second->heartbeat(elapsed);
	}	
}

void logic_room::robot_heartbeat(double elapsed)
{
	int isopen = m_robotcfg.mIsOpen;
	if(0==isopen ) return;
	
	if(m_robot_elapsed>0)
	{
		m_robot_elapsed -= elapsed;
	}
	if (m_robot_elapsed<0)
	{
		m_robot_elapsed = global_random::instance().rand_int(m_robotcfg.mRobotMinEntry, m_robotcfg.mRobotMaxEntry);
		int nRange = global_random::instance().rand_int(m_robotcfg.mRobotMinCount, m_robotcfg.mRobotMaxCount);
		int nCurCount = get_cur_count();
		if( nCurCount<= nRange)
		{
			for(auto it = m_tables.begin(); it != m_tables.end();++it)
			{
				if(it->second->is_full()==false)
				{
					it->second->request_robot();
				}
			}
		}
	}
}

void logic_room::request_robot()
{
	// T Robot
// 	static int minVIP = GoldFlower_RobotCFG::GetSingleton()->GetData("RobotMinVip")->mValue;
// 	static int maxVIP = GoldFlower_RobotCFG::GetSingleton()->GetData("RobotMaxVip")->mValue;
// 
// 	int32_t vip_level = global_random::instance().rand_int(minVIP, maxVIP);	
// 
// 	GOLD_TYPE gold_conditon = m_cfg->mGoldCondition;
// 	GOLD_TYPE base_gold = m_cfg->mBaseCondition;
// 
// 	GOLD_TYPE enter_gold = gold_conditon + global_random::instance().rand_int(base_gold*100, base_gold*1000);
// 	game_engine::instance().request_robot(RoomID->get_value(),enter_gold, vip_level);
}


uint32_t logic_room::get_cur_count()
{
	return m_players.size();
}

uint32_t logic_room::get_max_count()
{
	return m_nMaxPlayerCount;
}


LPlayerPtr& logic_room::get_player(uint32_t pid)
{
	auto it = m_players.find(pid);
	if (it != m_players.end())
	{
		return it->second;
	}
	return logic_player::EmptyPtr;
}

LPLAYER_MAP& logic_room::get_players()
{
	return m_players;
}

bool logic_room::is_full()
{
	return get_cur_count() < (m_cfg->mTableCount*m_nMaxPlayerCount);
}

bool logic_room::has_seat(uint16_t& tableid)
{
#if 0
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
#else
	// 优先找有人的桌子,且桌子未满
	for(auto it = m_tables.begin(); it != m_tables.end();++it)
	{
		if(it->second->is_opentable() && !it->second->is_full())
		{
			if(tableid != 0 && tableid == it->first)
				continue;

			tableid = it->first;
			return true;
		}
	}
	// 桌子满了,优先放出机器人
	for(auto it = m_tables.begin(); it != m_tables.end();++it)
	{
		if(it->second->is_full() && it->second->all_robot()>1)
		{ // 保留一个机器人在桌上
			if(tableid != 0 && tableid == it->first)
				continue;
			int32_t nSeat = it->second->release_robot_seat();
			if(nSeat!=INVALID_CHAIR)
			{
				tableid = it->first;
				return true;
			}
		}
	}
	//
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
#endif
	return false;
}

bool bmw_space::logic_room::has_seat_robot(uint16_t & tableid)
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

bool bmw_space::logic_room::change_table(uint16_t& tableid)
{
	// 换桌--随机选择桌子
	std::vector<uint16_t> vectable;
	std::srand(unsigned(std::time(0)));
	// 优先找有人的桌子,且桌子未满
	for(auto it = m_tables.begin(); it != m_tables.end();++it)
	{
		if(it->second->is_opentable() && !it->second->is_full())
		{
			if(tableid != 0 && tableid == it->first)
				continue;
			vectable.push_back(it->first);
		}
	}
	if(vectable.size() > 0)
	{
		std::random_shuffle(vectable.begin(),vectable.end());
		tableid = vectable.back();
		return true;
	}
	// 桌子满了,优先放出机器人
	for(auto it = m_tables.begin(); it != m_tables.end();++it)
	{
		if(it->second->is_full() && it->second->all_robot()>1)
		{ // 保留一个机器人在桌上
			if(tableid != 0 && tableid == it->first)
				continue;
			int32_t nSeat = it->second->release_robot_seat();
			if(nSeat!=INVALID_CHAIR)
			{
				vectable.push_back(it->first);
			}
		}
	}
	if(vectable.size() > 0)
	{
		std::random_shuffle(vectable.begin(),vectable.end());
		tableid = vectable.back();
		return true;
	}
	//
	for(auto it = m_tables.begin(); it != m_tables.end();++it)
	{
		if(!it->second->is_full())
		{
			if(tableid != 0 && tableid == it->first)
				continue;
			vectable.push_back(it->first);
		}
	}
	if(vectable.size() > 0)
	{
		std::random_shuffle(vectable.begin(),vectable.end());
		tableid = vectable.back();
		return true;
	}
	return false;
}

int logic_room::enter_table(LPlayerPtr player,uint16_t tid)
{
	if (getservice() == 0)
	{
		return 84;
	}
	auto it = m_players.find(player->get_pid());
	if (it != m_players.end())
	{
		SLOG_CRITICAL << "logic_room::enter_room exists player id:"<<player->get_id();
		return 2;
	}
	
	auto table = m_tables.find(tid);
	if(table == m_tables.end())
	{
		SLOG_CRITICAL<<"logic_room::enter_room table is null id:"<<tid;
		return 2;
	}
	int ret = table->second->enter_table(player);
	if(ret==1)
	{
		// 保存用户数据
		m_players.insert(std::make_pair(player->get_pid(), player));
		
		m_pids.push_back(player->get_pid());

		PlayerCount->add_value(1);

		SLOG_CRITICAL << "logic_room::enter_room Ok!!! :"<<player->get_pid();

		//@进入房间调用;
		if (!player->is_robot())
			db_log::instance().joingame(player->get_pid(), get_room_id(),m_childid);
	}
	return ret;
}

//@返回当前房间号;
uint16_t inline logic_room::get_room_id()
{
	return get_id();
	
}

void logic_room::leave_table(uint32_t pid)
{
	auto it = m_players.find(pid);
	if (it != m_players.end())
	{
		//@离开房间调用;
		auto playerid = pid;
		if (!it->second->is_robot())
			db_log::instance().leavegame(playerid);

		// 清理用户
		m_players.erase(it);
		// 更改用户数量
		PlayerCount->add_value(-1);
	}
	// 清理 pid_vec
	for (unsigned int i = 0; i < m_pids.size(); i++)
	{
		if (m_pids[i] == pid)
		{
			m_pids[i] = m_pids[m_pids.size() - 1];
			m_pids.pop_back();
			break;
		}
	}

}

void logic_room::getout_room(uint32_t pid)
{
	auto it = m_players.find(pid);
	if (it != m_players.end())
	{
		for (auto table : m_tables)
		{
			auto ptable = table.second;
			if (ptable) ptable->getout_table(pid);
		}
	}
}

const BMW_RoomCFGData* logic_room::get_data() const
{
	return m_cfg;
}


int logic_room::broadcast_msg_to_client(std::vector<uint32_t>& pids, uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg)
{
	return game_engine::instance().get_handler()->broadcast_msg_to_client(pids, packet_id, msg);
}

//////////////////////////////////////////////////////////////////////////
void logic_room::create_room()
{
	
}

bool logic_room::load_room()
{
	mongo::BSONObj b = db_game::instance().findone(DB_BMWROOM, BSON("room_id"<<RoomID->get_value()));
	if(b.isEmpty())
		return false;	

	return from_bson(b);
}

void logic_room::init_game_object()
{
	RoomID = CONVERT_POINT(Tfield<int16_t>, regedit_tfield(e_got_int16, "room_id"));
	PlayerCount = CONVERT_POINT(Tfield<int16_t>, regedit_tfield(e_got_int16, "player_count"));
}

bool logic_room::store_game_object(bool to_all)
{
	if(!has_update())
		return true;

	auto err = db_game::instance().update(DB_BMWROOM, BSON("room_id"<<RoomID->get_value()), BSON("$set"<<to_bson(to_all)));
	if(!err.empty())
	{
		SLOG_ERROR << "logic_room::store_game_object :" <<err;
		return false;
	}

	return true;
}

void bmw_space::logic_room::read_robotcfg()
{
	const boost::unordered_map<int, BMW_RobotCFGData>& list = BMW_RobotCFG::GetSingleton()->GetMapData();
	for (auto it = list.begin(); it != list.end(); ++it)
	{
		if(it->second.mRoomID == get_id())
		{
			m_robotcfg = it->second;
			break;
		}
	}
	m_robot_elapsed = global_random::instance().rand_int(m_robotcfg.mRobotMinEntry, m_robotcfg.mRobotMaxEntry);
}



