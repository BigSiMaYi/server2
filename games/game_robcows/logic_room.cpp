#include "stdafx.h"
#include "logic_room.h"
#include "RobCows_RoomCFG.h"
#include "RobCows_RobotCFG.h"
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


ROBCOWS_SPACE_USING

logic_room::logic_room(const RobCows_RoomCFGData* cfg, logic_lobby* _lobby)
:m_cfg(cfg)
,m_lobby(_lobby)
,m_checksave(0.0)
, m_synplayers_elapsed(0.0)
,m_service(1)
,m_rSwitch(2)
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
	//if(tableCount<=0 ||  tableCount>=200) tableCount = 100;
	for (uint16_t i=1;i<=tableCount;i++)
	{
		auto table = logic_table::malloc();
		table->init_table(i,this);
		m_tables.insert(std::make_pair(i,table));
	}
	printf("roomid=%d ,tables >>>> %d \n",RoomID->get_value(),m_tables.size());

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

const RobCows_RoomCFGData* logic_room::get_roomcfg()
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


void logic_room::setkiller(int killer /*= 0*/, int cutRound /*= 0*/)
{
	m_killer = killer;
	m_cutRound = cutRound;

	for (auto it = m_tables.begin(); it != m_tables.end(); ++it)
	{
		it->second->PlatformKiller(killer , cutRound);
	}
}

int logic_room::getkiller() const
{
	return m_killer;
}

int logic_room::getCutRound() const
{
	return m_cutRound;
}

void robcows_space::logic_room::setRobotSwitch(int rswitch)
{
	m_rSwitch = rswitch;
}

int robcows_space::logic_room::getRobotSwitch() const
{
	return m_rSwitch;
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
	// @ 间隔5分钟同步房间人数
	m_synplayers_elapsed += elapsed;
	if (m_synplayers_elapsed >= 300)
	{
		m_synplayers_elapsed = 0.0;
		auto p_gamehandler = game_engine::instance().get_handler();
		if (p_gamehandler)
		{
			p_gamehandler->syncRoomPcnt(m_players.size(),get_id());
		}
	}
}

void logic_room::robot_heartbeat(double elapsed)
{
#if 0
	bool isopen = m_robotcfg.mIsOpen;
	if(!isopen) return;
#else
	if (m_rSwitch == 2) return;
#endif
	if(m_robot_elapsed>0)
	{
		m_robot_elapsed -= elapsed;
	}
	if (m_robot_elapsed<0)
	{
		m_robot_elapsed = m_robotcfg.mElapseTime;

		int32_t nOpenCount = 0, nTableCount = 0;

		for(auto it = m_tables.begin(); it != m_tables.end();++it)
		{
			if(it->second->is_opentable())
			{
				nOpenCount++;
				if(it->second->is_all_robot())
				{
					if(it->second->all_robot()==1)
					{
						// @ 保证凑桌人数
						int reqCount = global_random::instance().rand_int(m_robotcfg.mRobotCountMin, m_robotcfg.mRobotCountMax);
						SLOG_CRITICAL << boost::format("Robot:only one add :%d") % reqCount;
						if (reqCount > 1) reqCount -= 1;
						for (int i = 0; i < reqCount; i++)
						{
							it->second->request_robot();
						}
						nTableCount++;
					}
					else if(it->second->all_robot()>1)
					{
						nTableCount++;
					}
					else
					{

					}
				}
			}
		}
		SLOG_CRITICAL<<boost::format("Robot:open:%d,table:%d")%nOpenCount%nTableCount;
		if(nOpenCount<m_robotcfg.mOpenTableMin)
		{
			if(nTableCount<m_robotcfg.mRobotTableMin)
			{ // 创建一个新桌 2-4
				for(auto it = m_tables.begin(); it != m_tables.end();++it)
				{
					if(!it->second->is_opentable())
					{
						int reqCount = global_random::instance().rand_int(m_robotcfg.mRobotCountMin, m_robotcfg.mRobotCountMax);
						for (int i=0;i<reqCount;i++)
						{
							it->second->request_robot();
						}
						break;
					}
				}
			}
		}
		else if(nOpenCount<m_robotcfg.mOpenTableMax)
		{
			if(nTableCount<m_robotcfg.mRobotTableMax)
			{// 创建一个新桌
				for(auto it = m_tables.begin(); it != m_tables.end();++it)
				{
					if(!it->second->is_opentable())
					{
						int reqCount = global_random::instance().rand_int(m_robotcfg.mRobotCountMin, m_robotcfg.mRobotCountMax);
						for (int i=0;i<reqCount;i++)
						{
							it->second->request_robot();
						}
						break;
					}
				}
			}
		}
		else
		{

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


uint16_t logic_room::get_cur_cout()
{
	assert(PlayerCount->get_value()==m_players.size());

	return PlayerCount->get_value();
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
	return get_cur_cout() < (m_cfg->mTableCount*GAME_PLAYER);
}

bool logic_room::has_seat(uint16_t& tableid)
{
	std::random_device rd;
	std::mt19937_64 g(rd());
	// priority
	int32_t nPriority = 5;
	std::map<int32_t, std::vector<LTablePtr>> maptable;
	int32_t nManCount = m_cfg->mRealCount;
	for (auto it = m_tables.begin(); it != m_tables.end(); ++it)
	{
		auto tid = it->first;
		if (tableid != 0 && tableid == tid)
			continue;
		auto pTable = it->second;
		if (pTable == nullptr)  continue;
		// @机器人开启才有效
		if (getRobotSwitch() == 2) nManCount = GAME_PLAYER;
		// @真实玩家已满
		int32_t nMan = pTable->all_human();
		if (nMan >= nManCount) continue;
		if (nMan + 1 >= nManCount)
		{ // @缺一
			nPriority = 1;
			if (pTable->is_full())
			{ // @桌子已满可腾出机器人的位置
// 				SLOG_CRITICAL << boost::format("%1%:%2%") % __FUNCTION__%__LINE__;
// 				if (pTable->can_release_robot())
// 				{
// 					SLOG_CRITICAL << boost::format("%1%:%2%") % __FUNCTION__%__LINE__;
// 					maptable[nPriority].push_back(it->second);
// 				}
// 				else
				{
					continue;
				}
			}
			else
			{ // @桌子未满直接进入
				maptable[nPriority].push_back(it->second);
			}
		}
		else if (nMan + 2 >= nManCount)
		{ // @缺二
			nPriority = 2;
			if (pTable->is_full())
			{ // @桌子已满可腾出机器人的位置
				SLOG_CRITICAL << boost::format("%1%:%2%") % __FUNCTION__%__LINE__;
// 				if (pTable->can_release_robot())
// 				{
// 					SLOG_CRITICAL << boost::format("%1%:%2%") % __FUNCTION__%__LINE__;
// 					maptable[nPriority].push_back(it->second);
// 				}
// 				else
				{
					continue;
				}
			}
			else
			{ // @桌子未满直接进入
				maptable[nPriority].push_back(it->second);
			}
		}
		else if (nMan > 0)
		{ // @存在真实玩家
			nPriority = 3;
			if (pTable->is_full())
			{ // @桌子已满可腾出机器人的位置
// 				if (pTable->can_release_robot())
// 				{
// 					maptable[nPriority].push_back(it->second);
// 				}
// 				else
				{
					continue;
				}
			}
			else
			{ // @桌子未满直接进入
				maptable[nPriority].push_back(it->second);
			}
		}
		else if (pTable->is_opentable())
		{ // @能玩的桌子,且桌子未满
			nPriority = 4;
			maptable[nPriority].push_back(it->second);
		}
		else
		{// @找一个空桌子
			nPriority = 5;
			maptable[nPriority].push_back(it->second);
		}
	}
	// check table id
	auto findfirst = maptable.begin();
	if (findfirst != maptable.end())
	{
		auto findvec = findfirst->second;
		std::shuffle(findvec.begin(), findvec.end(), g);
		auto findtable = findvec.back();
		tableid = findtable->get_id();
		if (findtable->is_full())
		{
			int32_t seat = findtable->release_robot_seat();
			SLOG_CRITICAL << boost::format("%1% release robot seat:%2%") % __FUNCTION__%seat;
		}
		SLOG_CRITICAL << boost::format("%1% find table id :%2%") % __FUNCTION__%tableid;
		return true;
	}
	else
	{
		SLOG_ERROR << boost::format("%1% find table is null") % __FUNCTION__;
		return false;
	}

#if 0
	int32_t nManCount = m_cfg->mRealCount;
	int32_t nExtraCount = m_cfg->mWaitCount;
	// @分配新的空位子
	for (auto it = m_tables.begin(); it != m_tables.end(); ++it)
	{
		if (tableid != 0 && tableid == it->first)
			continue;
		auto pTable = it->second;
		if(pTable==nullptr) continue;
		int n1 = pTable->all_human();
		int n2 = pTable->all_extraman();
		if (n1 >= nManCount)
		{ // @真实玩家已满
			continue;
		}
		else if (pTable->is_full())
		{  // @优先真实玩家满桌
			if ((n1 > 0) && (n1 < nManCount) && (n2 < nExtraCount))
			{// @有真实玩家且满桌优先匹配
				// @释放当前桌机器人
				int32_t nSeat = pTable->release_robot_seat();
				if (nSeat != INVALID_CHAIR)
				{ // @可以立即进入
					tableid = it->first;
					return true;
				}
				else
				{ // @附加等待结束加入对局
#ifdef UsingCling
					tableid = it->first;
					return true;
#else
					continue;
#endif // UsingCling
				}
			}
			else
			{// @附加玩家达到上限
				continue;
			}
		}
		else if (pTable->is_opentable())
		{// @优先找有人的桌子,且桌子未满
			tableid = it->first;
			return true;
		}
		else
		{// @找一个空桌子
			tableid = it->first;
			return true;
		}
	}
#endif
	return false;
}

bool robcows_space::logic_room::has_seat_robot(uint16_t & tableid)
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

bool robcows_space::logic_room::change_table(uint16_t& tableid)
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

void robcows_space::logic_room::read_robotcfg()
{
	const boost::unordered_map<int, RobCows_RobotRoomCFGData>& list = RobCows_RobotRoomCFG::GetSingleton()->GetMapData();
	for (auto it = list.begin(); it != list.end(); ++it)
	{
		if(it->second.mRoomID == get_id())
		{
			m_robotcfg = it->second;
			break;
		}
	}
	m_robot_elapsed = m_robotcfg.mElapseTime;
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
		SLOG_CRITICAL << "logic_room::enter_room exists player id:"<<player->get_pid();
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
			db_log::instance().joingame(player->get_pid(), get_room_id());
	}
	return ret;
}

//@返回当前房间号;
uint16_t inline logic_room::get_room_id()
{
	return RoomID->get_value();
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

void logic_room::cleanplayer(uint32_t pid)
{
	SLOG_CRITICAL << boost::format("logic_room::cleanplayer :%1%") % pid;
	auto it = m_players.find(pid);
	if (it != m_players.end())
	{
		// @ 清理用户
		auto player = it->second;
		if (player)
		{
			//@离开房间调用;
			auto playerid = pid;
			if (!player->is_robot())
				db_log::instance().leavegame(playerid);
			// 清理用户
			m_players.erase(it);
			// 更改用户数量
			PlayerCount->add_value(-1);
			// 清理用户
			player->leave_table();
		}
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

const RobCows_RoomCFGData* logic_room::get_data() const
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
	mongo::BSONObj b = db_game::instance().findone(DB_ROBCOWSROOM, BSON("room_id"<<RoomID->get_value()));
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

	auto err = db_game::instance().update(DB_ROBCOWSROOM, BSON("room_id"<<RoomID->get_value()), BSON("$set"<<to_bson(to_all)));
	if(!err.empty())
	{
		SLOG_ERROR << "logic_room::store_game_object :" <<err;
		return false;
	}

	return true;
}




