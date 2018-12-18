#include "stdafx.h"
#include "logic_room.h"
#include "GoldFlower_RoomCFG.h"
#include "GoldFlower_RobotCFG.h"
#include "game_db.h"
#include "logic_player.h"
//#include "logic_main.h"

#include "game_engine.h"
#include "i_game_ehandler.h"
#include "enable_random.h"

#include "logic_table.h"


#include "logic_player.h"
#include "robot_player.h"

//@add by Big O 2017/01/08;
//@����ͳ������;
#include "game_db_log.h"

ZJH_SPACE_USING

logic_room::logic_room(const GoldFlower_RoomCFGData* cfg, logic_lobby* _lobby)
:m_cfg(cfg)
,m_lobby(_lobby)
,m_checksave(0.0)
,m_synplayers_elapsed(0.0)
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
	// ���� tables
	int tableCount =  m_cfg->mTableCount;
//	if(tableCount<=0 ||  tableCount>=200) tableCount = 100;
	for (uint16_t i=1;i<=tableCount;i++)
	{
		auto table = logic_table::malloc();
		table->init_table(i,this);
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

const GoldFlower_RoomCFGData* logic_room::get_roomcfg()
{
	return m_cfg;
}


void zjh_space::logic_room::setservice(int service /*= 0*/)
{
	m_service = service;
}

int zjh_space::logic_room::getservice() const
{
	return m_service;
}
void zjh_space::logic_room::setkiller(int killer /*= 0*/, int cutRound /*= 0*/)
{
	m_killer = killer;
	m_cutRound = cutRound;

	for (auto it = m_tables.begin(); it != m_tables.end(); ++it)
	{
		it->second->PlatformKiller(killer, cutRound);
	}

}

int zjh_space::logic_room::getkiller() const
{
	return m_killer;
}

int zjh_space::logic_room::getCutRound() const
{
	return m_cutRound;
}

void zjh_space::logic_room::setRobotSwitch(int rswitch)
{
	m_rSwitch = rswitch;
}

int zjh_space::logic_room::getRobotSwitch() const
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

	// ˢ��ĳ������
	m_checksave += elapsed;
	if (m_checksave > 30)
	{
	
		store_game_object();
		m_checksave=0.0;
	}
	// ���Ӹ���
	for (auto it = m_tables.begin(); it != m_tables.end(); ++it)
	{
		it->second->heartbeat(elapsed);
	}	
	// @ ���5����ͬ����������
	m_synplayers_elapsed += elapsed;
	if (m_synplayers_elapsed >= 300)
	{
		m_synplayers_elapsed = 0.0;
		auto p_gamehandler = game_engine::instance().get_handler();
		if (p_gamehandler)
		{
			p_gamehandler->syncRoomPcnt(m_players.size(), get_id());
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
						// @ ��֤��������
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
			{ // ����һ������ 2-4
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
			{// ����һ������
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
 	int32_t nManCount = m_cfg->mTableManCount;
	for (auto it = m_tables.begin(); it != m_tables.end(); ++it)
	{
		auto tid = it->first;
		if (tableid != 0 && tableid == tid )
			continue;
		auto pTable = it->second;
		if (pTable == nullptr)  continue;
		// @�����˿�������Ч
		if (getRobotSwitch() == 2) nManCount = GAME_PLAYER;
		// @��ʵ�������
		int32_t nMan = pTable->all_man();
		if ( nMan >= nManCount) continue;
		if (nMan+1 >= nManCount)
		{ // @ȱһ
			nPriority = 1;
			if (pTable->is_full())
			{ // @�����������ڳ������˵�λ��
				if (pTable->can_release_robot())
				{
					maptable[nPriority].push_back(it->second);
					SLOG_CRITICAL << boost::format("%1%:%2%") % __FUNCTION__%__LINE__;
				}
				else
				{
					continue;
				}
			}
			else
			{ // @����δ��ֱ�ӽ���
				maptable[nPriority].push_back(it->second);
			}
		}
		else if (nMan+2 >= nManCount)
		{ // @ȱ��
			nPriority = 2;
			if (pTable->is_full())
			{ // @�����������ڳ������˵�λ��
				if (pTable->can_release_robot())
				{
					maptable[nPriority].push_back(it->second);
					SLOG_CRITICAL << boost::format("%1%:%2%") % __FUNCTION__%__LINE__;
				}
				else
				{
					continue;
				}
			}
			else
			{ // @����δ��ֱ�ӽ���
				maptable[nPriority].push_back(it->second);
			}
		}
		else if (nMan > 0)
		{ // @������ʵ���
			nPriority = 3;
			if (pTable->is_full())
			{ // @�����������ڳ������˵�λ��
				if (pTable->can_release_robot())
				{
					maptable[nPriority].push_back(it->second);
					SLOG_CRITICAL << boost::format("%1%:%2%") % __FUNCTION__%__LINE__;
				}
				else
				{
					continue;
				}
			}
			else
			{ // @����δ��ֱ�ӽ���
				maptable[nPriority].push_back(it->second);
			}
		}
		else if (pTable->is_opentable())
		{ // @���������,������δ��
			nPriority = 4;
			maptable[nPriority].push_back(it->second);
		}
		else
		{// @��һ��������
			nPriority = 5;
			maptable[nPriority].push_back(it->second);
		}
	}
	// check table id
	auto findfirst = maptable.begin();
	if (findfirst != maptable.end())
	{
		auto findvec = findfirst->second;
		std::shuffle(findvec.begin(), findvec.end() ,g);
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
	// @2018-3-12
	// ���������˵�����,������δ��
	// ������������
	for(auto it = m_tables.begin(); it != m_tables.end();++it)
	{
		if (tableid != 0 && tableid == it->first)
			continue;
		auto pTable = it->second;
		if(pTable==nullptr)  continue;
		// @�����˿�������Ч
		if (getRobotSwitch() == 2)
		{
			nManCount = GAME_PLAYER;
		}
		if (pTable->all_man() >= nManCount) continue;
		if (pTable->is_full())
		{// @��������
			int32_t nSeat = pTable->release_robot_seat();
			if (nSeat != INVALID_CHAIR)
			{ // @������������
				tableid = it->first;
				return true;
			}
			else
			{ // @���ӵȴ���������Ծ�
#ifdef UsingCling
				tableid = it->first;
				return true;
#else
				continue;
#endif // UsingCling
			}
		}
		else if(it->second->is_opentable())
		{ // @���������˵�����,������δ��
			tableid = it->first;
			return true;
		}
		else
		{// @��һ��������
			tableid = it->first;
			return true;
		}
	}
#endif
	return false;
}

bool zjh_space::logic_room::has_seat_robot(uint16_t & tableid)
{
	// @2018-3-12
	int32_t nRobotCount = m_cfg->mTableRobotCount;
	for(auto it = m_tables.begin(); it != m_tables.end();++it)
	{
		if (tableid != 0 && tableid == it->first)
			continue;
		if(it->second->all_robot() >= nRobotCount)
			continue;
		if(!it->second->is_full())
		{
			tableid = it->first;
			return true;
		}
	}
	return false;
}

bool zjh_space::logic_room::change_table(uint16_t& tableid)
{
	// @2018-3-12
	// ������������
	int32_t nManCount = m_cfg->mTableManCount;
	// ����--���ѡ������
	std::vector<uint16_t> vectable;
	std::vector<uint16_t> vectableEmpty;
	std::srand(unsigned(std::time(0)));
	// @���������˵�����,������δ��
	for(auto it = m_tables.begin(); it != m_tables.end();++it)
	{
		if (tableid != 0 && tableid == it->first)
			continue;
		if(it->second->is_full())
			continue;
		// @�����趨����
		if (it->second->all_man() >= nManCount)
			continue;
		if(it->second->is_opentable())
		{// @�ѿ���
			vectable.push_back(it->first);
		}
		else if (!it->second->is_full())
		{// @��һ��������
			vectableEmpty.push_back(it->first);
		}
	}
	if(vectable.size() > 0)
	{ // @�������
		std::random_shuffle(vectable.begin(),vectable.end());
		tableid = vectable.back();
		return true;
	}
	else if (vectableEmpty.size() > 0)
	{// @�������
		std::random_shuffle(vectableEmpty.begin(), vectableEmpty.end());
		tableid = vectableEmpty.back();
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
	// @2018-3-12 ���ƻ���������
	if (player->is_robot())
	{
		int32_t nRobotCount = m_cfg->mTableRobotCount;
		if (table->second->all_robot() >= nRobotCount)
			return 12;
	}
	int ret = table->second->enter_table(player);
	if(ret==1)
	{
		// �����û�����
		m_players.insert(std::make_pair(player->get_pid(), player));
		
		m_pids.push_back(player->get_pid());

		PlayerCount->add_value(1);

		SLOG_CRITICAL << "logic_room::enter_room Ok!!! :"<<player->get_pid();

		//@���뷿�����;
		if (!player->is_robot())
			db_log::instance().joingame(player->get_pid(), get_room_id());
	}
	return ret;
}

//@���ص�ǰ�����;
uint16_t inline logic_room::get_room_id()
{
	return RoomID->get_value();
}

void logic_room::leave_table(uint32_t pid)
{
	auto it = m_players.find(pid);
	if (it != m_players.end())
	{
		//@�뿪�������;
		auto playerid = pid;
		if (!it->second->is_robot())
			db_log::instance().leavegame(playerid);

		// �����û�
		m_players.erase(it);
		// �����û�����
		PlayerCount->add_value(-1);
	}
	// ���� pid_vec
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

void zjh_space::logic_room::getout_room(uint32_t pid)
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

const GoldFlower_RoomCFGData* logic_room::get_data() const
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
	mongo::BSONObj b = db_game::instance().findone(DB_ZJHROOM, BSON("room_id"<<RoomID->get_value()));
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

	auto err = db_game::instance().update(DB_ZJHROOM, BSON("room_id"<<RoomID->get_value()), BSON("$set"<<to_bson(to_all)));
	if(!err.empty())
	{
		SLOG_ERROR << "logic_room::store_game_object :" <<err;
		return false;
	}

	return true;
}

void zjh_space::logic_room::read_robotcfg()
{
	const boost::unordered_map<int, GoldFlower_RobotRoomCFGData>& list = GoldFlower_RobotRoomCFG::GetSingleton()->GetMapData();
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



