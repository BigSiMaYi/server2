#include "stdafx.h"
#include "logic_room.h"
#include "Landlord3_RoomCFG.h"
#include "game_db.h"
#include "logic_player.h"


#include "game_engine.h"
#include "i_game_ehandler.h"
#include "enable_random.h"

#include "logic_table.h"

#include "landlord3_def.pb.h"
#include "proc_landlord_logic.h"
#include "logic_player.h"
#include "robot_player.h"

//@add by Big O 2017/01/08;
//@在线统计增加;
#include "game_db_log.h"

LANDLORD_SPACE_USING

logic_room::logic_room(const Landlord3_RoomCFGData* cfg, logic_lobby* _lobby, int child_id)
:m_cfg(cfg)
,m_lobby(_lobby)
,m_checksave(0.0)
, m_child_id(child_id)
, m_killpoint(0)
, m_killpoint_status(false)
, m_server_stauts(0)
, m_req_robot(0)
, m_sync_palyer_times(0)
{
	init_game_object();
	PlayerCount->set_value(0);
	m_pids.clear();
	m_players.clear();
	m_tables.clear();

	RoomID->set_value(cfg->mRoomID);
	// 创建 tables
	int tableCount =  m_cfg->mTableCount;
	if(tableCount<=0 ||  tableCount>=200) tableCount = 100;

	for (uint16_t i=1;i<=tableCount;i++)
	{
		auto table = logic_table::malloc();
		table->init_table(i,this);
		m_tables.insert(std::make_pair(i,table));

        //robot_cfg* robot = new robot_cfg(this);
        //m_table_robots.insert(std::make_pair(i, robot));
	}
	printf("roomid=%d ,tables >>>> %zd \n",RoomID->get_value(),m_tables.size());

	if(!load_room())
		create_room();
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

const Landlord3_RoomCFGData* logic_room::get_roomcfg()
{
	return m_cfg;
}

uint32_t logic_room::get_id()
{
	return m_cfg->mRoomID;
}

void logic_room::heartbeat(double elapsed)
{
//     if (m_players.size() == 0)
//     {
//         robot_heartbeat(elapsed);
//         return;
//     }

    // 刷新某项数据
    m_checksave += elapsed;
    if (m_checksave > 30)
    {
        store_game_object();
        m_checksave = 0.0;
    }
    // 桌子更新
    for (auto& item : m_tables)
    {
        LTablePtr& tb = item.second;
        tb->heartbeat(elapsed);
    }
	if ((m_sync_palyer_times -= elapsed) <= 0)
	{
		m_sync_palyer_times = 120;
		game_engine::instance().sync_room_players(get_id(), m_players.size());
	}
	
    //for (auto& item : m_table_robots)
    //{
    //    item.second->robot_heartbeat(elapsed);
    //}
}

void logic_room::robot_heartbeat(double elapsed)
{
	// T
    auto roomid = RoomID->get_value();
    auto robot_cfg = Landlord3_RobotCFG::GetSingleton();

	static int isOpen = robot_cfg->GetData(roomid)->mIsOpen;
	if (isOpen == 0) return;

	static int minCount = robot_cfg->GetData(roomid)->mRobotMinCount;
	static int maxCount = robot_cfg->GetData(roomid)->mRobotMaxCount;
	static int robotCount = global_random::instance().rand_int(minCount, maxCount);

	//机器人AI
	for (auto it = m_robot_players.begin(); it != m_robot_players.end(); ++it)
	{
		it->second->get_robot()->heartbeat(elapsed);
	}

	//机器人进入管理
	m_robot_elapsed -= elapsed;
	if (m_robot_elapsed <= 0)
	{
		m_robot_elapsed = global_random::instance().rand_double(5, 15);
// 		for (auto it = m_robot_players.begin(); it != m_robot_players.end(); ++it)
// 		{
// 			game_engine::instance().release_robot(it->first);
// 		}
        for (auto& item : m_tables)
        {
            LTablePtr& tb = item.second;
            //if (tb->is_opentable())
            {
                if (tb->is_all_robot())
                {
                    if (tb->get_robot_size() < 3)
                    {
                        request_robot(tb->get_id());
                    }
                }
            }
        }

// 		if (m_players.size() < 10 && m_robot_players.size() < robotCount)
// 		{
// 			request_robot();
// 		}
	}
}

void logic_room::request_robot(int32_t tid)
{
    auto roomid = RoomID->get_value();
    auto robot_cfg = Landlord3_RobotCFG::GetSingleton();
	// T Robot
    static int minVIP = robot_cfg->GetData(roomid)->mRobotMinVip;
    static int maxVIP = robot_cfg->GetData(roomid)->mRobotMaxVip;

    int32_t vip_level = global_random::instance().rand_int(minVIP, maxVIP);

    static int minGold = robot_cfg->GetData(roomid)->mRobotMinTake;
    static int maxGold = robot_cfg->GetData(roomid)->mRobotMaxTake;

    GOLD_TYPE gold_conditon = m_cfg->mGoldMinCondition;
    GOLD_TYPE base_gold = m_cfg->mBaseCondition;

    int32_t rid = roomid;
    int tag = rid + tid * 100;

    GOLD_TYPE enter_gold = base_gold + global_random::instance().rand_int(minGold, maxGold);
 	game_engine::instance().request_robot(tag, enter_gold, vip_level);
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

bool logic_room::has_seat(LPlayerPtr& player, uint16_t& tableid, bool isrobot)
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
		//is_opentable: 桌子上有人，并且有座位 ;
		if (it->second->is_opentable() && !it->second->is_full())
		{
			if (isrobot) //机器人请求桌子;
			{
				if (tableid != 0 && tableid == it->first) //换桌 tableid == it->first，同一桌;
					continue;

				tableid = it->first;
				return true;
			}
			else
			{
				if (it->second->can_enter_table(player))
				{
					if (tableid != 0 && tableid == it->first)
						continue;

					tableid = it->first;
					return true;
				}
			}
		}
	}
#if 0
    // 桌子满了,优先放出机器人
    for (auto it = m_tables.begin(); it != m_tables.end(); ++it)
    {
        if (it->second->is_full() && it->second->robot_counter() > 1)
        { // 保留一个机器人在桌上
            if (tableid != 0 && tableid == it->first)
                continue;
            int32_t nSeat = it->second->release_robot_seat();
            if (nSeat != INVALID_CHAIR)
            {
                tableid = it->first;
                return true;
            }
        }
    }
#endif
	
	//
	for(auto it = m_tables.begin(); it != m_tables.end();++it)
	{
		if(!it->second->is_full())
		{
			if (isrobot) //机器人请求桌子;
			{
				if (tableid != 0 && tableid == it->first)
					continue;

				tableid = it->first;
				return true;
			}
			else
			{
				if (it->second->can_enter_table(player))
				{
					if (tableid != 0 && tableid == it->first)
						continue;

					tableid = it->first;
					return true;
				}
			}
		}
	}
#endif
	return false;
}

int logic_room::enter_table(LPlayerPtr player,uint16_t tid)
{
    //已经停服务;
    if (m_server_stauts == 100)
    {
        return 84;
    }

	auto it = m_players.find(player->get_pid());
	if (it != m_players.end()) //玩家已经在游戏中，然后又发了enter_table的包，然后外面会调用 reqPlayer_leaveGame，此时当前游戏局将重置;
	{
		auto table = m_tables.find(tid);
		if (table == m_tables.end())
		{
			SLOG_CRITICAL << "logic_room::enter_room table is null id:" << tid;
		}
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
		SLOG_CRITICAL << "+++++ enter_table, playerid: " << player->get_pid() <<", player counter: " << m_players.size() << ", pid counter: " << m_pids.size();
		//@进入房间调用;
		if (!player->is_robot())
			db_log::instance().joingame(player->get_pid(), get_room_id(), tid);
		db_log::instance().joingame2(player->get_pid());
	}
	return ret;
}

//@返回当前房间号;
uint16_t inline logic_room::get_room_id()
{
	return RoomID->get_value();
}

uint16_t logic_room::get_child_id()
{
    return m_child_id;
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

		db_log::instance().leavegame2(playerid);

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
	SLOG_CRITICAL << "----- leave_table, playerid: " << pid << ", player counter: " << m_players.size() << ", pid counter: " << m_pids.size();
}

const Landlord3_RoomCFGData* logic_room::get_data() const
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
	mongo::BSONObj b = db_game::instance().findone(DB_LANDLORD3ROOM, BSON("room_id"<<RoomID->get_value()));
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

	auto err = db_game::instance().update(DB_LANDLORD3ROOM, BSON("room_id"<<RoomID->get_value()), BSON("$set"<<to_bson(to_all)));
	if(!err.empty())
	{
		SLOG_ERROR << "logic_room::store_game_object :" <<err;
		return false;
	}

	return true;
}

uint32_t logic_room::get_robot_size()
{
#if 0
	int robot_size = 0;
	for (auto& item : m_players)
	{
		if (item.second->is_robot())
		{
			++robot_size;
		}
	}
#endif
	return  m_req_robot;
}

void logic_room::record_robot(bool flag)
{
	if (flag)
	{
		++m_req_robot;
	}
	else
	{
		--m_req_robot;
	}
}

void logic_room::service_ctrl(int32_t optype)
{
    m_server_stauts = optype;
    for (auto& item : m_tables)
    {
        item.second->service_ctrl(optype);
    }
}

void logic_room::kill_points(int32_t cutRound, bool status)
{
	 for (auto& item : m_tables)
    {
        item.second->kill_points(cutRound, status);
    }
	 if (status)
	 {
		 SLOG_CRITICAL << "###### Manager Start kill points, cut round: " << cutRound << ", status: " << status;
	 }
	 else
	 {
		 SLOG_CRITICAL << "###### Manager Stop kill points, cut round: " << cutRound << ", status: " << status;
	 }

    m_killpoint = cutRound;
    m_killpoint_status = status;
}

void logic_room::robot_ctrl(bool on_off)
{
	SLOG_ERROR << "--------- Manager ctrl robot, switch: " << on_off << ", roomid: "<< get_id();
	for (auto& item : m_tables)
	{
		item.second->robot_ctrl(on_off);
	}
}

int32_t logic_room::get_cut_round()
{
    if (m_killpoint_status)
    {
        return  m_killpoint;
    }
    return 0;
}

void logic_room::kick_player(uint32_t playerid, int bforce)
{
    auto itr = m_players.find(playerid);
    if (itr != m_players.end())
    {
        auto& player = itr->second;
        if (player)
        {
            if (bforce == 1)
            {
                player->set_kick_status(7);//e_cleanout_gm_kick
            }
            else if (bforce == 2)
            {
                auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_clean_out, landlord3_protocols::e_mst_l2c_clean_out);
                if (sendmsg)
                {
                    sendmsg->set_reason((e_msg_cleanout_def)7);
                    player->send_msg_to_client(sendmsg);
                }

                auto pl = player->getIGamePlayer();
                if (pl)
                {
					SLOG_CRITICAL << "kick_player reqPlayer_leaveGame player id: " << playerid;
                    pl->reqPlayer_leaveGame();
                }
            }
        }
    }
}

