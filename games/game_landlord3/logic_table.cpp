#include "stdafx.h"
#include "logic_table.h"
#include "logic_player.h"

#include "logic_room.h"
#include "game_engine.h"
#include "i_game_ehandler.h"
#include "game_db.h"
#include "logic_lobby.h"
#include <net\packet_manager.h>


#include "landlord3_def.pb.h"
#include "landlord3_logic.pb.h"

#include "enable_random.h"

#include "robot_mgr.h"
#include "logic_common.h"


#include <cfloat> // DBL_MAX
#include <cmath> // std::nextafter

using namespace boost;

LANDLORD_SPACE_USING

static const int MAX_TALBE_PLAYER = 5;//桌子人数
static const int MAX_OP_PLAYER = 4;//观战人数

#define MAX_TIME	180

logic_table::logic_table(void)
:m_room(nullptr)
,m_players(GAME_PLAYER)//每个桌子4个人
,m_player_count(0)
,m_elapse(0.0)
,m_checksave(0)
, m_questioned(0)
{
	
	init_game_object();

	initGame();
	// 设置 Chair ID
	for (int32_t i = 0; i < GAME_PLAYER; i++)
	{
		m_VecChairMgr.push_back(i);
	}
	// Robot
	memset(&m_robotcfg,0,sizeof(m_robotcfg));


	// Test
	std::vector<POKER> vecp;
	POKER p1[7] = {
		{5,1},{10,2},{8,0},{11,3},{4,1},{6,2},{7,2}
	};
	for (int i=0;i<7;i++)
	{
		vecp.push_back(p1[i]);
	}
	std::sort(vecp.begin(),vecp.end(),POKER_LESS());
	std::vector<POKER> vecpdel;
	POKER p2[4] = {
		{5,1},{8,0},{11,3},{6,2},
	};
	for (int i=0;i<4;i++)
	{
		vecpdel.push_back(p1[i]);
	}
	
	//boost::posix_time::ptime ;
// 	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
// 	SLOG_CRITICAL<<boost::format("logic_table::[%1%]")%now;
// 	int64_t tick = now.time_of_day().total_seconds();
// 	boost::posix_time::ptime today = now - now.time_of_day();
// 	SLOG_CRITICAL<<boost::format("logic_table::[%1%]")%today;
// 	boost::posix_time::ptime now2 = boost::posix_time::second_clock::local_time();
// 	int64_t tick2 = now2.time_of_day().total_seconds();
// 	SLOG_CRITICAL<<boost::format("logic_table::[%1%]")%now2;
// 	boost::posix_time::time_duration m = now2 - now;
// 	int64_t nn = m.total_seconds();
// 	int64_t nn2 = tick2-tick;

	// [0,10]
	//std::random_device rd;
	//std::uniform_int_distribution<int> d(0, 10);
	//for(int n = 0; n < 100; ++n)
	//{
	//	std::cout << d(rd) << ' ';
	//}
	//std::cout<<std::endl;
	//std::cout<<"============="<<std::endl;
	//
// 	std::random_device rd2;
// 	for(int n = 0; n < 10; ++n)
// 	{
// 		std::cout << rd2()<<' '<<rd2.min()<<","<<rd2.max();
// 	}
// 	std::cout<<std::endl;
// 	//
// 	std::random_device rd3;
// 	std::mt19937_64 mt(rd3());
// 	for(int n = 0; n < 10; n++)
// 	{
// 		std::cout << mt() << std::endl;
// 	}
// 	std::cout<<std::endl;
	//std::cout<<"============="<<std::endl;
	//// 平均分布 [10,20]
	//std::random_device rd4;
	//std::mt19937_64 gen(rd4());
	//std::uniform_int_distribution<> dis(10, 20);
	//for(int n = 0; n < 100; n++)
	//{
	//	 std::cout << dis(gen) << ' ';
	//}
	//std::cout<<std::endl;
	//std::cout<<"============="<<std::endl;
	//// 正太分布
	//std::random_device rd5;
	//std::mt19937_64 gen2(rd5());
	//std::normal_distribution<> dis2(5,2);
	//for(int n = 0; n < 10; n++)
	//{
	//	std::cout << dis2(gen2) << ' ';
	//}
	//std::cout<<std::endl;
	//std::cout<<"============="<<std::endl;
    //
	//// 随机 [10,20]
	//std::default_random_engine re(std::time(0));
	//std::uniform_int_distribution<> dis3(10,20); // 注意默认构造
	//for(int n = 0; n < 10; n++)
	//{
	//	std::cout << dis3(re) << ' ';
	//}
	//std::cout<<std::endl;
	//std::cout<<"============="<<std::endl;
	//// 随机 [0.0,1.0)
	//std::uniform_real_distribution<double> dis4(0.0, 1.0);  // 注意默认构造
	//for(int n = 0; n < 1000; n++)
	//{
	//	std::cout << dis4(re) << ' ';
	//}
	//std::cout<<std::endl;
	//std::cout<<"============="<<std::endl;
}


logic_table::~logic_table(void)
{
}

void logic_table::release()
{
	store_game_object();
}

void logic_table::init_table(uint16_t tid, logic_room* room)
{
	// 保存当前房间信息
	m_room = room;
	// 设置 Table ID
	TableID->set_value(tid);
	
	if(!load_table())
		create_table();

	set_status(eGameState_Free);
	//
	m_ReadyTime = Landlord3_BaseInfo::GetSingleton()->GetData("PreTime")->mValue;
	m_BankerTime = Landlord3_BaseInfo::GetSingleton()->GetData("BankerTime")->mValue;
	m_OperaTime = Landlord3_BaseInfo::GetSingleton()->GetData("OperaTime")->mValue;
	m_ResultTime = Landlord3_BaseInfo::GetSingleton()->GetData("ResultTime")->mValue;
	// Robot
	const boost::unordered_map<int, Landlord3_RobotCFGData>& list = Landlord3_RobotCFG::GetSingleton()->GetMapData();
	for (auto it = list.begin(); it != list.end(); ++it)
	{
		if(it->second.mRoomID == m_room->get_id())
		{
			m_robotcfg = it->second;
			break;
		}
	}
	m_robot_elapsed = global_random::instance().rand_int(m_robotcfg.mRobotMinEntry, m_robotcfg.mRobotMaxEntry);
}

uint32_t logic_table::get_id()
{
	return TableID->get_value();
}

int8_t logic_table::get_status()
{
	return TableStatus->get_value();
}

void logic_table::set_status(int8_t state)
{
    char st = m_analyser.GetStatus();
	TableStatus->set_value(st);
}

void logic_table::inc_dec_count(bool binc/* = true*/)
{
	if(binc)
	{
		m_player_count++;
	}
	else
	{
		m_player_count--;

		if(m_player_count == 0)
			store_game_object();
	}
}

void logic_table::heartbeat( double elapsed )
{
	if(m_player_count==0)
		return;

	// Robot
	robot_heartbeat(elapsed);
	// Player
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if(player != nullptr)
			player->heartbeat(elapsed);	
	}
	//同步协议
	m_elapse += elapsed;
	
	m_checksave+= elapsed;
	if (m_checksave > 30)//桌子信息30s保存1次
	{
		store_game_object();

// 		// 30 秒读取一次
// 		ReadCardRate();
	}

	//////////////////////////////////////////////////////////////////////////
	if(m_duration>0)
	{
		m_duration -= elapsed;
	}
    
	int8_t nStatus = m_analyser.GetStatus();
	switch (nStatus)
	{
	case eGameState_Free:
		{
			// 清理超时未准备玩家;
        std::vector<LPlayerPtr> pids;
			int64_t now = GetCurrentStamp64();
			for(auto user : m_MapTablePlayer)
			{ // 未准备玩家踢出桌子
				if(user.second.p_active == FALSE)
				{
					auto uid = user.second.p_id;
					if(now - user.second.p_selftime>=m_ReadyTime+TIME_LESS+50)
					{
						SLOG_CRITICAL<<boost::format("[Sit-TimeOut] :%d")%uid;
						// 清理通知
						onGameCleanOut(uid,landlord3_protocols::e_msg_cleanout_def::e_cleanout_not_sit_timeout);

                        user.second.p_selftime = now;
                        // 清理
                        auto pl = user.second.p_playerptr->getIGamePlayer();
                        if (pl)
                        {
                            pl->reqPlayer_leaveGame();
                        }
                        pids.push_back(user.second.p_playerptr);
					}
				}
			}
            for (auto& id : pids)
            {
                id->leave_table();
                //leave_table(id);
            }

			int nPlayerCount = GetAllUserCount();
			int nReadyCount = GetActiveUserCount();
			if(nPlayerCount==GAME_PLAYER && nReadyCount==GAME_PLAYER)
			{
                m_analyser.Start();
				onGameStart();
			}
		}
		break;
	case eGameState_FaPai:
		//{
		//	if(m_duration<0)
		//	{
		//		m_duration = 0.0;
		//		set_status(eGameState_Banker);
		//		onNoticeJiao();
		//	}
		//}
		//break;
	//case eGameState_Banker:
		{
			if(m_duration<=0)
			{
				m_duration = 0.0;
                m_curpid =GenLoadlord();
                onNoticeJiao(m_curpid);
				defaultJiao(m_curpid);
			}
		}
		break;
	case eGameState_Play:
		{
			if (m_duration<=0)
			{
				SLOG_CRITICAL<<boost::format("logic_table::heartbeat[TimeOut] first:%d,operator:%d")%m_nFirstID%m_lastpid;
				m_duration = 2.0;
				defaultOutCard();
			}
		}
		break;
	case eGameState_End:
		{
			if (m_duration<0)
			{
				m_duration = 0.0;

				set_status(eGameState_Free);

				// 桌子复位
				repositGame();
                m_analyser.GameOver();
			}
		}
		break;
	default:
		break;
	}

	// TEST

}

bool logic_table::is_full()
{
	return m_player_count >= GAME_PLAYER;
}

bool landlord_space::logic_table::is_opentable()
{
	return m_player_count >= 1;
}

unsigned int logic_table::get_max_table_player()
{
	return m_players.size();
}

LPlayerPtr& logic_table::get_player(int index)
{
	if(IsVaildSeat(index))
		return m_players[index];

	return logic_player::EmptyPtr;

}

LPlayerPtr& logic_table::get_player_byid(uint32_t pid)
{
	auto it = m_MapTablePlayer.find(pid);
	if(it != m_MapTablePlayer.end())
	{
		return it->second.p_playerptr;
	}
	return logic_player::EmptyPtr;

}

logic_room* logic_table::get_room()
{
	return m_room;
}

int32_t logic_table::get_seat_byid(uint32_t pid)
{
	int32_t seat = INVALID_CHAIR;
	auto it = m_MapTablePlayer.find(pid);
	if(it != m_MapTablePlayer.end())
	{
		seat = it->second.p_idx;
	}
	return seat;
}

uint32_t logic_table::get_id_byseat(int32_t seat)
{
	uint32_t uid = 0;
	for (auto user : m_MapTablePlayer)
	{
		if(user.second.p_idx == seat)
		{
			uid = user.second.p_id;
			break;
		}
	}
	return uid;
}

int logic_table::enter_table(LPlayerPtr player)
{
	if(player->get_table()!=nullptr) 
	{
		SLOG_ERROR<<"logic_table enter_table is in table";
		return 2;
	}

	// 随机分配椅子
	if(m_VecChairMgr.size()>0)
	{
		std::random_shuffle(std::begin(m_VecChairMgr),std::end(m_VecChairMgr));
		int32_t seat = m_VecChairMgr.back();
		m_VecChairMgr.pop_back();
		// find seat -- order by 
		bc_enter_seat(seat, player);
		// 
		uint32_t id = player->get_pid();
		m_pids.push_back(player->get_pid());

		inc_dec_count();
		//
		TablePlayer table_player;
		memset(&table_player,0,sizeof(table_player));
		table_player.p_playerptr = player;
		table_player.p_idx = seat;
		uint32_t uid = player->get_pid();
		table_player.p_id  = uid;
		table_player.p_asset = player->get_gold();
		table_player.p_active = FALSE;

		table_player.p_outs = 0;
		table_player.p_isauto = false;
		table_player.p_islast1 = false;

		table_player.p_selftime = GetCurrentStamp64();
		m_MapTablePlayer.insert(std::make_pair(uid,table_player));

		if(get_status()==eGameState_Play)
		{
			player->set_status(eUserState_Wait);
		}
		else if(get_status()==eGameState_FaPai)
		{
			player->set_status(eUserState_Wait);
		}
		else
		{
			player->set_status(eUserState_free);
		}

		SLOG_CRITICAL<<boost::format("logic_table::enter_table:%1%,seatid:%2%")%get_id()%seat;
		return 1;
	}

	player->join_table(nullptr,-1);
	return 12;
}

void logic_table::leave_table(uint32_t pid)
{
	// 清理用户
	auto it = m_MapTablePlayer.find(pid);
	if(it != m_MapTablePlayer.end())
	{
		auto user = it->second;
		int32_t seat = user.p_idx;
		m_VecChairMgr.push_back(seat);
		LPlayerPtr& player = user.p_playerptr;
		// 允许弃牌离开
		if(player && player->get_status() == eUserState_dead)
		{
			int64_t lUserExpend = user.p_expend;
			int64_t lGold = 0;
			if(lUserExpend > 0)
			{
				lGold -= lUserExpend;
				player->write_property(lGold);
				SLOG_CRITICAL<<boost::format("User[%1%]-Leave-Table:expend:%2%")%pid%lGold;
			}
		}
		// 清理用户
		if(player)
		{
			player->set_status(eUserState_null);
			player->clear_round();
			player->set_wait(0);
		}
		m_room->leave_table(pid);
		// 
		inc_dec_count(false);

		bc_leave_seat(pid);	

		m_MapTablePlayer.erase(it);
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

bool logic_table::can_leave_table(uint32_t pid)
{
	auto it = m_MapTablePlayer.find(pid);
	if(it != m_MapTablePlayer.end())
	{
		auto user = it->second;
		LPlayerPtr player = user.p_playerptr;
		if(player)
		{
			if(player->get_status()==eUserState_play)
			{
				return false;
			}
			else if(player->get_status()==eUserState_dead)
			{// 允许弃牌离开
				return true;
			}
			else
			{
				return true;
			}
		}
	}
	// 
	return false;
}

bool logic_table::change_sit(uint32_t pid, uint32_t seat_index)
{

	return true;
}

int logic_table::broadcast_msg_to_client(std::vector<uint32_t>& pids, uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg)
{
//	printf("broadcast_msg_to_client: %d:%d,,%d\n",pids.size(),pids[pids.size()-1],packet_id);

	return game_engine::instance().get_handler()->broadcast_msg_to_client(pids, packet_id, msg);

}


void logic_table::bc_enter_seat(int seat_index, LPlayerPtr& player)
{
	printf("logic_table enter seat ,seat:%d \n",seat_index);

	player->join_table(this,seat_index);
	
	//  notice other
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_user_enter_seat,landlord3_protocols::e_mst_l2c_user_enter_seat);
	auto playerinfo = sendmsg->mutable_playerinfo();
	playerinfo->set_seat_index(seat_index);
	playerinfo->set_player_id(player->get_pid());
	playerinfo->set_player_nickname(player->get_nickname());
	playerinfo->set_player_headframe(player->get_photo_frame());
	playerinfo->set_player_headcustom(player->get_icon_custom());
	playerinfo->set_player_gold(player->get_gold());
	playerinfo->set_player_sex(player->get_sex());
	playerinfo->set_player_viplv(player->get_viplvl());
	playerinfo->set_player_ticket(player->get_ticket());

	broadcast_msg_to_client2(sendmsg);

}

void logic_table::bc_leave_seat(uint32_t player_id)
{
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_user_leave_seat, landlord3_protocols::e_mst_l2c_user_leave_seat);
	sendmsg->set_playerid(player_id);
	broadcast_msg_to_client2(sendmsg);
}

//////////////////////////////////////////////////////////////////////////
void logic_table::create_table()
{
	
	

}

void logic_table::init_game_object()
{
	TableID = CONVERT_POINT(Tfield<int16_t>, regedit_tfield(e_got_int16, "table_id"));

	TableStatus = CONVERT_POINT(Tfield<int8_t>, regedit_tfield(e_got_int8,"table_status"));
}

bool logic_table::load_table()
{
	mongo::BSONObj b = db_game::instance().findone(DB_LANDLORD3TABLE, BSON("room_id"<<m_room->get_id() << "table_id" << TableID->get_value()));
	if(b.isEmpty())
		return false;

	return from_bson(b);
	
}

bool logic_table::store_game_object(bool to_all)
{
	if(!has_update())
		return true;

	auto err = db_game::instance().update(DB_LANDLORD3TABLE, BSON("room_id"<<m_room->get_id() << "table_id" << TableID->get_value()), BSON("$set"<<to_bson(to_all)));
	if(!err.empty())
	{
		SLOG_ERROR << "logic_table::store_game_object :" <<err;
		return false;
	}

	m_checksave = 0;

	return true;
}


boost::shared_ptr<landlord3_protocols::packetl2c_scene_info_free> logic_table::get_scene_info_msg_free()
{
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_scene_info_free,landlord3_protocols::e_mst_l2c_scene_info_free);
	auto freeinfo = sendmsg->mutable_freeinfo();
	int8_t nGameState = get_status();
	int32_t nSpareTime = 0;
	
	freeinfo->set_status(nGameState);
	freeinfo->set_roomid(m_room->get_id());
	freeinfo->set_tableid(get_id());
	freeinfo->set_sparetime(nSpareTime);
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto seat = user.p_idx;
		if(player!=nullptr)
		{
			auto playerinfo = freeinfo->add_playerinfo();
			playerinfo->set_seat_index(seat);
			playerinfo->set_player_id(player->get_pid());
			playerinfo->set_player_nickname(player->get_nickname());
			playerinfo->set_player_headframe(player->get_photo_frame());
			playerinfo->set_player_headcustom(player->get_icon_custom());
			playerinfo->set_player_gold(player->get_gold());
			playerinfo->set_player_sex(player->get_sex());
			playerinfo->set_player_viplv(player->get_viplvl());
			playerinfo->set_player_ticket(player->get_ticket());
			playerinfo->set_player_status(player->get_status());
            playerinfo->set_player_region(*player->getIGamePlayer()->GetUserRegion());
			playerinfo->clear_usergold();
			playerinfo->clear_cardinfo();
		}
	}

	return sendmsg;
}

boost::shared_ptr<landlord3_protocols::packetl2c_scene_info_play> landlord_space::logic_table::get_scene_info_msg_play(uint32_t uid)
{
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_scene_info_play,landlord3_protocols::e_mst_l2c_scene_info_play);
	// 
	auto playinfo = sendmsg->mutable_playinfo();
	int8_t nGameState = get_status();
	playinfo->set_status(nGameState);
	int32_t rid = m_room->get_id();
	playinfo->set_roomid(rid);
	playinfo->set_tableid(get_id());
	playinfo->set_bankerid(m_landlordpid);
	playinfo->set_curid(m_lastpid);
	playinfo->set_leftcd(m_duration);


	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto seat = user.p_idx;
		auto usergold = user.p_asset;
		auto userid = user.p_id;
		auto cflag = user.p_cardflag;
		if(player!=nullptr)
		{
			auto playerinfo = playinfo->add_playerinfo();
			playerinfo->set_seat_index(seat);
			playerinfo->set_player_id(userid);
			playerinfo->set_player_nickname(player->get_nickname());
			playerinfo->set_player_headframe(player->get_photo_frame());
			playerinfo->set_player_headcustom(player->get_icon_custom());
			playerinfo->set_player_gold(player->get_gold());
			playerinfo->set_player_sex(player->get_sex());
			playerinfo->set_player_viplv(player->get_viplvl());
			playerinfo->set_player_ticket(player->get_ticket());
			// play wait
			playerinfo->set_player_status(player->get_status());

			playerinfo->set_usergold(usergold);
			// card info 
			
		}
	}

	return sendmsg;
}

template<class T>
void landlord_space::logic_table::broadcast_msg_to_client2(T msg)
{
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if (player != nullptr )
		{
			if(!player->is_robot())
			{
				player->send_msg_to_client(msg);
			}
			else
			{
				// Robot
				robot_mgr::instance().recv_packet(player->get_pid(),msg->packet_id(),msg);
			}
		}
	}
}

int32_t logic_table::GetActiveUserCount()
{
	int32_t nActiveCount = 0;
	for(auto& user : m_MapTablePlayer)
	{
		if(user.second.p_active == TRUE)
			nActiveCount++;
	}

	return nActiveCount;
}

int32_t logic_table::GetAllUserCount()
{
	return m_MapTablePlayer.size();
}

bool logic_table::onEventUserReady(uint32_t playerid)
{
	int32_t nResult = eReadyResult_Succeed;
	//
	auto it = m_MapTablePlayer.find(playerid);
	if(it == m_MapTablePlayer.end())
	{
		SLOG_CRITICAL<<"logic_table::onEventUserReady Can't Ready:nullptr";
		nResult = eReadyResult_Faild_unknow;
	}
	else
	{
		auto& tuser = it->second;
		auto player = tuser.p_playerptr;
		if(player->is_robot())
		{
			SLOG_CRITICAL<<boost::format("onEventUserReady:Robot-id:%1%")%playerid;
		}
		else
		{
			SLOG_CRITICAL<<boost::format("onEventUserReady:User-id:%1%")%playerid;
		}
		if(player->get_status()>eUserState_free)
		{
			SLOG_CRITICAL<<"logic_table::onEventUserReady Can't Ready:usestate";
			nResult = eReadyResult_Faild_userstate;
		}
		else if(get_status()!=eGameState_Free )
		{
			SLOG_CRITICAL<<"logic_table::onEventUserReady Can't Ready:gamestate";
			nResult = eReadyResult_Faild_gamestate;
		}
		else if(player->get_gold()< m_room->get_data()->mGoldMinCondition)
		{
			SLOG_CRITICAL<<"logic_table::onEventUserReady Can't Ready:gold";
			nResult = eReadyResult_Faild_gold;
		}
		else
		{
			// succeed
			tuser.p_active = TRUE;
			nResult = eReadyResult_Succeed;
			player->set_status(eUserState_ready);
		}
	}

	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_ask_ready_result,landlord3_protocols::e_mst_l2c_ask_ready_result);
	sendmsg->set_playerid(playerid);
	sendmsg->set_result(nResult);
	// TODO Bug to fix : can't send msg
//	int ret = broadcast_msg_to_client(sendmsg);
	// TODO send msg ok
//	m_room->broadcast_msg_to_client(sendmsg);

	broadcast_msg_to_client2(sendmsg);
	

	return true;
}

void logic_table::initGame(bool bResetAll/*=false*/)
{
	set_status(eGameState_Free);

	// 初始化游戏参数
	m_landlordpid = 0;

	m_nLastWiner = 0;

	m_nFirstID = 0;

	m_VecActive.clear();


	m_VecRobBank.clear();

	m_nGameRate = 0;

	// 
	m_lastpid = m_curpid = 0;
	m_nFirstID = 0;
	m_nGameRate = 0;


	m_vecCardData.clear();
	m_vecOutCard.clear();
	
}


void logic_table::repositGame()
{
	SLOG_CRITICAL<<"------------RepositGame------------";
	// 游戏数据初始化
	m_landlordpid = 0;


	m_nLastWiner = 0;

	m_nFirstID = 0;

	m_VecActive.clear();

	m_nGameRate = 0;
	//
	m_VecRobBank.clear();

	// reset
	m_vecCardData.clear();

	// 重置数据
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		user.p_active = FALSE;
		user.p_playing = FALSE;
		user.p_asset = player->get_gold();
		user.p_expend = 0;
		user.p_selftime = GetCurrentStamp64();
		user.p_bankrate = -1;

		user.p_outs = 0;
		user.p_isauto = false;
		user.p_islast1 = false;

		player->set_status(eGameState_Free);

		// 清理机器人
		if(player->is_robot())
		{
			int32_t nRound = global_random::instance().rand_int(m_robotcfg.mRobotMinRound, m_robotcfg.mRobotMaxRound);
			if(player->get_gold()<m_room->get_data()->mGoldMinCondition || 
				player->get_round()>=nRound)
			{
				release_robot(player->get_pid());
			}
		}
	}

	// 清理不足条件用户
	CleanOutPlayer();
}


void logic_table::Wash_Card()
{
	// 洗牌
	std::srand(unsigned(std::time(0)));
	if(m_vecOutCard.size()==GPOKERS)
	{
		POKER *lpHead = &(*m_vecOutCard.begin());
		int32_t nCut = global_random::instance().rand_int(1,GAME_PLAYER);
		if(m_uBomCounter >= 7)
		{
			nCut = 4;
		}
		else if(m_uBomCounter > 3)
		{
			nCut = 3;
		}
		int32_t nStart = 0,nCount = 0;
		for (int32_t i=0;i<nCut;i++)
		{
			nStart = 10+global_random::instance().rand_int(0,GPOKERS-MAXPOKER1);
			if(i==1)
			{
				nCount = GPOKERS - nStart;
			}
			else
			{
				nCount = MAXPOKERDI+global_random::instance().rand_int(0,GPOKERS-nStart-MAXPOKERDI);
			}
			if(nCount==0 || nStart==0) continue;
			POKER *lpSave = new POKER[nCount];
			memcpy(lpSave,lpHead+nStart,sizeof(POKER)*nCount);
			memmove(lpHead+nCount,lpHead,sizeof(POKER)*nStart);
			memcpy(lpHead,lpSave,sizeof(POKER)*nCount);
			delete []lpSave;
		}
		for (auto card : m_vecOutCard)
		{
			m_vecCardData.push_back(card);
		}
		m_vecOutCard.clear();
		m_uBomCounter = 0;
	}
	else
	{
		for (int32_t i=0;i<GPOKERS;i++)
		{
			m_vecCardData.push_back(g_pokers[i]);
		}
		std::random_device rd;
		std::mt19937_64 g(rd());
		std::shuffle(m_vecCardData.begin(),m_vecCardData.end(),g);
	}
}


bool logic_table::Allot_Card(uint32_t playerid,VECPOKER& pokers)
{
	uint32_t nCount = m_vecCardData.size();
	if(nCount==0) return false;
	if(nCount<MAXPOKER1) return false;
	for (int32_t i=0;i<MAXPOKER1;i++)
	{
		POKER pker = m_vecCardData.back();
		m_vecCardData.pop_back();
		pokers.push_back(pker);
	}
	return true;
}


bool logic_table::Allot_BaseCard(VECPOKER &pokers)
{
	uint32_t nCount = m_vecCardData.size();
	if(nCount!=MAXPOKERDI) return false;
	for (int32_t i=0;i<MAXPOKERDI;i++)
	{
		POKER pker = m_vecCardData.back();
		m_vecCardData.pop_back();
		pokers.push_back(pker);
	}
	return true;
}

int logic_table::GenLoadlord()
{
    if (!m_VecActive.empty())
    {
        srand((unsigned)time(nullptr));
        std::random_shuffle(m_VecActive.begin(), m_VecActive.end());
        int pid = m_VecActive.back();
        m_VecActive.pop_back();
        return pid;
    }
    return -1;
}

bool logic_table::onGameStart()
{
	SLOG_CRITICAL<<"------------GameStart------------";
	std::srand(unsigned(std::time(0)));
	// 底注
	int64_t base = m_room->get_data()->mBaseCondition;
	// check play and active
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto playerid = user.p_id;
		if(player==nullptr) continue;
		if(user.p_active==TRUE)
		{
			m_VecActive.push_back(playerid);

			user.p_expend = 0;
			user.p_result = 0;

			user.p_bankrate = -1;

			player->set_status(eUserState_play);
			player->add_round();
			player->set_wait(0);
		}
	}
	// 
	Wash_Card();
	//
	m_MapPlayerVecPoker.clear();
	m_MapPlayerPokerOut.clear();

	// deal msg
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{ // 参与游戏用户
		auto &user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		uint32_t playerid = player->get_pid();
		auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_game_start,landlord3_protocols::e_mst_l2c_game_start);
		sendmsg->set_basegold(base);
		// state
		for (auto it2=m_MapTablePlayer.begin();it2!=m_MapTablePlayer.end();it2++)
		{
			auto &user = it2->second;
			sendmsg->add_state(user.p_active);
			sendmsg->add_stateid(user.p_id);
		}
		// card info
		VECPOKER poker;
		Allot_Card(playerid,poker);
		m_MapPlayerVecPoker.insert(std::make_pair(playerid,poker));
		std::sort(poker.begin(),poker.end(),POKER_GREATER());
        m_analyser.SendCard(user.p_idx, poker, player->is_robot());

		auto cardinfo = sendmsg->mutable_cardinfo();
		if(player->is_robot())
		{
            user.p_isauto = true; //机器人自动出牌;
			// Robot
			robot_mgr::instance().recv_packet(player->get_pid(),sendmsg->packet_id(),sendmsg);
		}
		else
		{
            //给用户发牌;
			cardinfo->mutable_pokers()->Reserve(MAXPOKER1);
			for (int i=0;i<MAXPOKER1;i++)
			{
				auto pokerinfo = cardinfo->add_pokers();
				pokerinfo->set_value(poker[i].value);
				pokerinfo->set_style(poker[i].style);
			}
			cardinfo->set_pokertype(0);
			cardinfo->clear_pokerstatus();
			player->send_msg_to_client(sendmsg);
		}
	}

	set_status(eGameState_FaPai);
	// 计算发牌时间
	float fCardTime = Landlord3_BaseInfo::GetSingleton()->GetData("CardTime")->mValue;
	float fPauseTime = Landlord3_BaseInfo::GetSingleton()->GetData("PauseTime")->mValue;

	m_duration = fCardTime + fPauseTime;

	return true;

}

bool logic_table::onNoticeJiao(uint32_t playerid)
{
	if(playerid==0)  
	{
		SLOG_ERROR<<boost::format("%1%:%2%")%__FUNCTION__%__LINE__;
		return false;
	}
	SLOG_CRITICAL<<boost::format("%1%:%2%,Rob-id:%3%")%__FUNCTION__%__LINE__%playerid;
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_notice_robbanker,landlord3_protocols::e_mst_l2c_notice_robbanker);
	sendmsg->set_playerid(playerid);
	broadcast_msg_to_client2(sendmsg);
	m_duration = m_BankerTime + TIME_LESS;
	return true;
}

bool logic_table::onEventJiao(uint32_t playerid, int32_t robrate)
{
	// 抢庄
	SLOG_CRITICAL<<boost::format("%1%:%2% playerid:%3%,robrate:%4%")%__FUNCTION__%__LINE__%playerid%robrate;
	// check game state
	int32_t nGameState= get_status();
	if( nGameState != eGameState_Banker)
	{
		//SLOG_ERROR<<boost::format("%1%:%2% game state err:%3%")%__FUNCTION__%__LINE__%nGameState;
		//return false;
	}
	// check
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"onGameRobBanker player err";
		return false;
	}
	auto &user =  player_it->second;
	auto player = user.p_playerptr;
	auto &bankrate = user.p_bankrate;
	if(player && player->is_robot())
	{
		SLOG_CRITICAL<<boost::format("%1%:%2% Robot-id:%3%")%__FUNCTION__%__LINE__%playerid;
        //机器人叫地主;
        robrate  = m_analyser.CalcGetLandScore(user.p_idx);
        m_analyser.GetLandlord(user.p_idx, robrate);
	}
    else
    {
        m_analyser.GetLandlord(user.p_idx, robrate);
    }
	// check player
	if(m_curpid != playerid)
	{
		SLOG_ERROR<<"onGameRobBanker OperatorID err";
		return false;
	}
	if(!IsValidRate(robrate))
	{
		SLOG_ERROR<<"onGameRobBanker invalid rate";
		return false;
	}
	if(!CanRobBanker(robrate))
	{
		SLOG_ERROR<<"onGameRobBanker can't robrate";
		return false;
	}
	if(bankrate != -1)
	{
		SLOG_ERROR<<"onGameRobBanker repeat request 1";
		return false;
	}
	// save
	bankrate = robrate;
	// send msg
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_robbank_result,landlord3_protocols::e_mst_l2c_ask_robbank_result);
	sendmsg->set_playerid(playerid);
	sendmsg->set_robrate(robrate);
	broadcast_msg_to_client2(sendmsg);
	// check
	maybe_JiaoEnd(playerid,robrate);

	return true;
}

void logic_table::maybe_JiaoEnd(uint32_t playerid,int32_t robrate)
{
	SLOG_CRITICAL<<boost::format("%1%:%2%,id:%3%,rob:%4%")%__FUNCTION__%__LINE__%playerid%robrate;
    assert(m_curpid == playerid);

    m_curpid = playerid;
    m_questioned++;

    if (robrate == eCallBanker_ThreePoint)
    {
        m_nGameRate = robrate;
        m_landlordpid = m_curpid;
        m_lastpid = 0;
    }
    else if (robrate > m_nGameRate)
	{
		m_nGameRate = robrate;
        m_lastpid = m_curpid;
	}

    if (m_questioned == 3)
    {
        if (m_lastpid != 0)
        {
            m_landlordpid = m_lastpid;
            m_curpid = m_lastpid;
            m_lastpid = 0;
        }
        else if (m_landlordpid == 0)
        {
            m_nGameRate = eCallBanker_OnePoint;
            m_curpid = playerid;
            m_landlordpid = playerid;
            m_lastpid = 0;

            // 都不抢庄默认抢庄
            auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_robbank_result, landlord3_protocols::e_mst_l2c_ask_robbank_result);
            sendmsg->set_playerid(m_curpid);
            sendmsg->set_robrate(eCallBanker_OnePoint);
            broadcast_msg_to_client2(sendmsg);
        }
    }

    if (m_landlordpid != 0)
    {
        m_lastpid = m_landlordpid;
        onEventBaseCard();
    }
}

void logic_table::defaultJiao(uint32_t playerid)
{
	SLOG_CRITICAL<<boost::format("%1%:%2%,playerid:%3%")%__FUNCTION__%__LINE__%playerid;
	if(playerid!=0)
	{
		onEventJiao(playerid,eCallBanker_uncall);
	}
}

bool logic_table::onEventBaseCard()
{
	set_status(eGameState_Play);
	m_duration = TIME_LESS + m_OperaTime;
	// get base card m_nBasePoker
	SLOG_CRITICAL<<boost::format("onEventBaseCard:Banker:%d,GameRate:%d")%m_landlordpid%m_nGameRate;
	m_nBasePoker.clear();
	Allot_BaseCard(m_nBasePoker);
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_base_card,landlord3_protocols::e_mst_l2c_send_base_card);
	sendmsg->set_playerid(m_landlordpid);
	sendmsg->set_gamerate(m_nGameRate);
	auto cardinfo = sendmsg->mutable_cardinfo();
	int32_t nCount = m_nBasePoker.size();
	cardinfo->mutable_pokers()->Reserve(nCount);

    auto lorditr = m_MapPlayerVecPoker.find(m_landlordpid);
	for (int32_t i=0;i<nCount;i++)
	{
		auto pokerinfo = cardinfo->add_pokers();
		pokerinfo->set_value(m_nBasePoker[i].value);
		pokerinfo->set_style(m_nBasePoker[i].style);

        if (lorditr != m_MapPlayerVecPoker.end())
        {
            auto& lordcard = lorditr->second;
            lordcard.push_back(m_nBasePoker[i]);
        }
	}
    //发地主牌;
    m_analyser.SendLandlordCard(m_nBasePoker);
	cardinfo->set_pokertype(0);
	cardinfo->clear_pokerstatus();
	broadcast_msg_to_client2(sendmsg);
	return true;
}

void logic_table::onNoticeChangeRate()
{
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_bombrate,landlord3_protocols::e_mst_l2c_bombrate);
	sendmsg->set_gamerate(m_nGameRate);

	broadcast_msg_to_client2(sendmsg);
}

bool logic_table::onEventAuto(uint32_t playerid,bool bAuto)
{
	// 
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"onEventAuto player err";
		return false;
	}
	auto &user =  player_it->second;
	auto &bauto = user.p_isauto;
	if(bauto == bAuto) 
	{
		SLOG_CRITICAL<<format("Auto repeat");	
		return false;
	}
	// 
	user.p_isauto = bAuto;
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_trustee_result,landlord3_protocols::e_mst_l2c_trustee_result);
	sendmsg->set_playerid(playerid);
	sendmsg->set_bauto(bAuto);
	return true;
}

void logic_table::defaultOutCard()
{
	SLOG_CRITICAL<<boost::format("defaultOutCard first:%d,operator:%d")%m_nFirstID%m_curpid;
	auto itr = m_MapTablePlayer.find(m_curpid);
	if(itr != m_MapTablePlayer.end())
	{
	    auto& cur_user = itr->second;

        if (cur_user.p_isauto)
        {
            VECPOKER pokers;
            auto it = m_MapPlayerVecPoker.find(m_curpid);
            if (it != m_MapPlayerVecPoker.end())
            {
                auto& mypoker = it->second;
                VECPOKER outpoker;
                onEventOutCard(m_curpid, outpoker);
            }
        }
    }
}

void landlord_space::logic_table::PlayerDiscard(uint32_t playerid, const VECPOKER&pokers)
{
    auto player_it = m_MapTablePlayer.find(playerid);
    if (player_it != m_MapTablePlayer.end())
    {
        auto& user = player_it->second;

        m_analyser.PlayerDiscard(user.p_idx, pokers);
    }
}

bool logic_table::onEventOutCard(uint32_t playerid,const VECPOKER&pokers)
{
	// 出牌
	SLOG_CRITICAL<<boost::format("%1%:%2% playerid:%3%")%__FUNCTION__%__LINE__%playerid;
	// check game state
	int32_t nGameState= m_analyser.GetStatus();
	if( nGameState != eGameState_Play)
	{
		SLOG_ERROR<<boost::format("%1%:%2% game state err:%3%")%__FUNCTION__%__LINE__%nGameState;
		return false;
	}
	// check player
	if(m_curpid != playerid)
	{
		SLOG_ERROR<<"onGameOutCard OperatorID err";
		return false;
	}
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<"onGameOutCard player err";
		return false;
	}

    auto &user = player_it->second;
    auto player = user.p_playerptr;

    int cid;
    int next_cid;
    if (player && player->is_robot())
    {
        SLOG_CRITICAL << boost::format("%1%:%2% Robot-id:%3%") % __FUNCTION__%__LINE__%playerid;
        //std::vector<POKER> discard_poker;
        m_analyser.Discard(cid, next_cid, (VECPOKER&)pokers);
        if (user.p_idx != cid)
        {
            int a = 0;
        }
        //m_analyser.Discard(cid, discard_poker);
    }
    else
    {
        std::vector<POKER> discard_poker;
        m_analyser.Discard(cid, next_cid, (VECPOKER&)discard_poker);
        if (user.p_idx != cid)
        {
            int a = 0;
        }
    }
	// check card invalid
	if(CheckPoker(playerid,pokers)==false)
	{
		SLOG_ERROR<<boost::format("%1%:%2% CheckPoker ")%__FUNCTION__%__LINE__;
		//return false;
	}
	// Compare
	char style = m_logiccard.GetCardType(pokers);
	if(style == STYLE_NO)
	{
		SLOG_ERROR<<boost::format("%1%:%2% STYLE_NO ")%__FUNCTION__%__LINE__;
		//return false;
	}
	char val = m_logiccard.GetAValueFromAConstTypePoke(style,pokers);
	char len = pokers.size();
	if(style > STYLE_NO)
	{
		char v = m_logiccard.CompareCard(m_style,m_val,m_len,style,val,len);
		if( v != 1)
		{
			SLOG_ERROR<<boost::format("%1%:%2% Compare Err ")%__FUNCTION__%__LINE__;
			//return false;
		}
	}

	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_outcard_result,landlord3_protocols::e_mst_l2c_outcard_result);
	sendmsg->set_playerid(playerid);
	auto cardinfo = sendmsg->mutable_cardinfo();
	cardinfo->mutable_pokers()->Reserve(MAXPOKER1);
	for (auto p : pokers)
	{
		auto pokerinfo = cardinfo->add_pokers();
		pokerinfo->set_value(p.value);
		pokerinfo->set_style(p.style);
		// save out card
		m_vecOutCard.push_back(p);
	}
	cardinfo->set_pokertype(style);
	cardinfo->clear_pokerstatus();
	// record current out card
	auto outit = m_MapPlayerPokerOut.find(playerid);
	if(outit != m_MapPlayerPokerOut.end())
	{
		auto outpokers = outit->second;
		outpokers.clear();

		outpokers = pokers;

		SLOG_CRITICAL<<format("%d,out card :%d")%playerid%len;
	}
	user.p_outs += 1;
	// remove hand card
	int32_t nCount = RemovePoker(playerid,pokers);
	// check maybe over
	bool bGameOver = false;
	if(nCount == 0)
	{
		sendmsg->clear_nextid();

		bGameOver = true;
	}
	else
	{
		m_nFirstID = m_lastpid;
        m_lastpid = m_curpid;
        m_curpid = GetNextPlayerByID(m_lastpid, next_cid);
		sendmsg->set_nextid(m_curpid);
	}

    bool ispass = m_analyser.IsPass(cid);
    if (ispass)
    {
        defaultPass(m_lastpid, m_curpid);
    }
    else
    {
        broadcast_msg_to_client2(sendmsg);
    }

	// chang rate
	if(style >= STYLE_BOMB)
	{
		m_nGameRate *= 2;
		onNoticeChangeRate();
		m_uBomCounter++;
	}

	m_style = style;
	m_val = val;
	m_len = len;
	
	// 游戏结束
	if(bGameOver)
	{
		// TODO
		// 2017-9-7 11:17:07
		onGameOver(playerid);

		// Save other hand card
		for(auto pit = m_MapPlayerVecPoker.begin();pit!=m_MapPlayerVecPoker.end();pit++)
		{
			for (auto p : pit->second)
			{
				m_vecOutCard.push_back(p);
			}
		}

	}

	return true;
}

bool logic_table::CheckPoker(uint32_t playerid,const VECPOKER&poker)
{
	// 
	bool bfind = false;
	auto it = m_MapPlayerVecPoker.find(playerid);
	if(it != m_MapPlayerVecPoker.end())
	{
		auto& mypoker = it->second;
        for (auto& p2 : poker)
		{
			bfind = false;
            for (auto& p1 : mypoker)
			{
				if(p1==p2)
				{
					bfind = true;
				}
			}
			if(bfind==false) return false;
		}
		return true;
	}
	return false;
}


int32_t logic_table::RemovePoker(uint32_t playerid,const VECPOKER&poker)
{
	int32_t nCount = 0;
	auto it = m_MapPlayerVecPoker.find(playerid);
	if(it != m_MapPlayerVecPoker.end())
	{
		auto& mypoker = it->second;
		
        for (auto& p : poker)
        {
            for (auto myit = mypoker.begin(); myit != mypoker.end(); myit++)
            {
				if( (*myit) == p)
				{
                    mypoker.erase(myit);
					break;
				}
			}
		}
		nCount = mypoker.size();
	}
	return nCount;
}

bool logic_table::IsValidRate(int32_t nRate)
{
	if(nRate==eCallBanker_uncall || nRate==eCallBanker_OnePoint || 
		nRate==eCallBanker_TwoPoint || nRate==eCallBanker_ThreePoint)
		return true;
	return false;
}

bool logic_table::CanRobBanker(int32_t nRate)
{
	if(nRate==eCallBanker_uncall) return true;
	if(nRate <= m_nGameRate) return false;
	return true;
}
uint32_t logic_table::GetNextPlayerByID(uint32_t playerid, int32_t nextseatid)
{
	int32_t seat = get_seat_byid(playerid);
    SLOG_CRITICAL << "------------GetNextPlayerByID playerid:" << playerid << ", seat id: " << seat;
    int32_t nextseat = INVALID_CHAIR;
    int32_t fseat = seat;
    do
    {
        if (nextseat == seat) break;
#if 0
        nextseat = (fseat - 1 + GAME_PLAYER) % GAME_PLAYER;
#else
        nextseat = (fseat + 1) % GAME_PLAYER;
#endif
        auto itfind = std::find(m_VecChairMgr.begin(), m_VecChairMgr.end(), nextseat);
        if (itfind == m_VecChairMgr.end())
        {
            uint32_t findid = get_id_byseat(nextseat);
            auto it2 = std::find(m_VecActive.begin(), m_VecActive.end(), findid);
            if (it2 != m_VecActive.end()) break;
        }
        fseat = nextseat;
    } while (TRUE);

    if (nextseatid != nextseat)
    {
        int a = nextseat;
    }
    uint32_t nextid = get_id_byseat(nextseatid);
    
	SLOG_CRITICAL<<"GetNextPlayerByID nextid:"<<nextid;
	return nextid;
}

bool logic_table::onEventPass(uint32_t playerid,bool bTimeOver/*=false*/)
{
	SLOG_CRITICAL<<boost::format("%1%:%2% playerid:%3%")%__FUNCTION__%__LINE__%playerid;
	// check game state
	int32_t nGameState= get_status();
	if( nGameState !=eGameState_Play)
	{
		SLOG_ERROR<<boost::format("%1%:%2% game state err:%3%")%__FUNCTION__%__LINE__%nGameState;
		return false;
	}
	// check player
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<boost::format("[Danger]%1%:%2% player err:%3%")%__FUNCTION__%__LINE__%playerid;
		return false;
	}
	auto &user =  player_it->second;
	auto player = user.p_playerptr;
	if(player && player->is_robot())
	{
		SLOG_CRITICAL<<boost::format("%1%:%2% Robot-id:%3%")%__FUNCTION__%__LINE__%playerid;
	}
	// pass 
	if(playerid != m_lastpid)
	{
		SLOG_ERROR<<boost::format("%1%:%2% current player err:")%__FUNCTION__%__LINE__;
		return false;
	}
	if(playerid == m_nFirstID)
	{
		SLOG_ERROR<<boost::format("%1%:%2% first player can't pass")%__FUNCTION__%__LINE__;
		return false;
	}
	// 

	// clear
	auto it_out = m_MapPlayerPokerOut.find(playerid);
	if(it_out != m_MapPlayerPokerOut.end())
	{
		auto poker = it_out->second;
		poker.clear();
	}
	if(m_lastpid == m_nFirstID)
	{
		m_style = STYLE_NO;
		m_val = 0;
		m_len = 0;
	}

    int next_cid = m_analyser.Pass(user.p_idx);
    m_nFirstID = m_lastpid;
    m_lastpid = m_curpid;
    m_curpid = GetNextPlayerByID(m_lastpid, next_cid);

	// next
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_pass_result,landlord3_protocols::e_mst_l2c_pass_result);
	sendmsg->set_playerid(playerid);
	sendmsg->set_nextid(m_curpid);
	broadcast_msg_to_client2(sendmsg);

	m_duration = m_OperaTime + TIME_LESS;

	return true;
}

void logic_table::defaultPass(uint32_t playerid, uint32_t next_pid, bool bTimeOver/*=false*/)
{
	SLOG_CRITICAL<<boost::format("%1%:%2%,Pass-id:%3%")%__FUNCTION__%__LINE__%playerid;
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_pass_result,landlord3_protocols::e_mst_l2c_pass_result);
	sendmsg->set_playerid(playerid);
	//uint32_t nNext = GetNextPlayerByID(playerid, 0);
	sendmsg->set_nextid(next_pid);
	//m_lastpid = nNext;
	broadcast_msg_to_client2(sendmsg);

	m_duration = m_OperaTime + TIME_LESS;
}

bool landlord_space::logic_table::onGameOver(uint32_t playerid,bool bRun/*=false*/)
{
	SLOG_CRITICAL<<boost::format("%1%:%2% winid:%3%,bRun:%4%")%__FUNCTION__%__LINE__%playerid%bRun;
	// 计算玩家输赢
	set_status(eGameState_End);
	m_duration = m_ResultTime + TIME_LESS;
	// 
	int64_t base = m_room->get_data()->mBaseCondition;
	int64_t money = base * m_nGameRate;
	// check player
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<boost::format("%1%:%2% player err:%3%")%__FUNCTION__%__LINE__%playerid;
		return false;
	}
	auto &user =  player_it->second;
	auto player = user.p_playerptr;
	int64_t gold = player->get_gold();
	if(bRun)
	{
		if(m_nGameRate==0)
		{
			money = base * 10;
		}
		else
		{
			money = base * m_nGameRate * 10;
		}
		user.p_result -= std::min(money,gold);
		m_vecOutCard.clear();
		m_uBomCounter++;
	}
	else
	{
		char outs = 0,zouts = 0;
		bool isauto = false;
		for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
		{
			auto user = it->second;
			LPlayerPtr& player = user.p_playerptr;
			if(user.p_active==FALSE || player==nullptr) continue;
			auto uid = user.p_id;
			if(uid == m_landlordpid)
			{
				zouts += user.p_outs;
			}
			else
			{
				outs += user.p_outs;
			}
			if(user.p_isauto && user.p_islast1) isauto = true;
		}
		//  春天
		if(outs == 0)
		{
			m_nGameRate *= 2;
			m_chun = GAME_CHUN_YES;
		}
		else if(zouts==1)
		{
			m_nGameRate *= 2;
			m_chun = GAME_CHUN_FAN;
		}

		if(m_chun != GAME_CHUN_NO)
			money *= 2;

		int64_t zmoney = 0;
		if(money == 0)
		{
			SLOG_CRITICAL<<format("[!!!!!!!!!!!!!]");
			money = base * 3;
		}
		// 
		if( playerid == m_landlordpid)
		{ // 地主赢
			for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
			{
				auto user = it->second;
				LPlayerPtr& player = user.p_playerptr;
				if(user.p_active==FALSE || player==nullptr) continue;
				auto uid = user.p_id;
				if(uid == m_landlordpid) continue;
				int64_t ugold = player->get_gold();
				if(isauto != false)
				{
					if(user.p_isauto&&user.p_islast1==false&&money!=0)
					{
						int64_t nRes = std::min(ugold*2,money);
						user.p_result -= nRes;
						zmoney += nRes;
					}
					else
					{
						user.p_result = 0;
					}
				}
			}
			// 地主
			auto banker_it = m_MapTablePlayer.find(m_landlordpid);
			if(banker_it == m_MapTablePlayer.end())
			{
				assert(0);
				return false;
			}
			auto &user =  banker_it->second;
			user.p_result = zmoney;
		}
		else
		{ // 地主输
			auto banker_it = m_MapTablePlayer.find(m_landlordpid);
			if(banker_it == m_MapTablePlayer.end())
			{
				assert(0);
				return false;
			}
			auto &user =  banker_it->second;
			auto player = user.p_playerptr;
			zmoney -= std::min(2*money,player->get_gold());
			for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
			{
				auto user = it->second;
				LPlayerPtr& player = user.p_playerptr;
				if(user.p_active==FALSE || player==nullptr) continue;
				auto uid = user.p_id;
				if(uid == m_landlordpid) 
				{
					user.p_result = zmoney;
				}
				else if(isauto!=false)
				{
					if(user.p_isauto && user.p_islast1==false)
					{
						user.p_result = 0;
					}
					else
					{
						user.p_result = std::abs(zmoney) / 2;
					}
				}
				else
				{
					user.p_result = std::abs(zmoney) / 2;
				}
				
			}
		}
	}

	// write
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_game_result,landlord3_protocols::e_mst_l2c_game_result);
	sendmsg->mutable_resultinfo()->Reserve(GAME_PLAYER);
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto user = it->second;
		LPlayerPtr& player = user.p_playerptr;
		if(user.p_active==FALSE || player==nullptr) continue;

		auto resultinfo = sendmsg->add_resultinfo();
		uint32_t uid = user.p_id;
		int64_t resultgold = user.p_result;

		int32_t nTax = 0;
		int32_t nRate = 0;
		auto p_gamehandler = game_engine::instance().get_handler();
		if(p_gamehandler && m_room)
		{
			nRate = p_gamehandler->GetRoomCommissionRate(m_room->get_id());
		}
		if(resultgold > 0 && nRate > 0)
		{
			nTax = std::ceil(resultgold * nRate / 100.0);
		}
		resultgold -= nTax;
		int64_t selfgold = player->get_gold() + resultgold;

		resultinfo->set_playerid(uid);
		resultinfo->set_player_gold(selfgold);
		resultinfo->set_player_result(resultgold);
		// save
		player->write_property(resultgold,nTax);
		SLOG_CRITICAL<<boost::format("[Save DB] %1%:usergold:%2%,result:%3%,tax:%4%")%uid%selfgold%resultgold%nTax;

	}
	broadcast_msg_to_client2(sendmsg);

	return true;
}



void logic_table::CleanOutPlayer()
{
	std::map<uint32_t,TablePlayer>  tMapTablePlayer = m_MapTablePlayer;
	for(auto user : tMapTablePlayer)
	{
		auto player = user.second.p_playerptr;
		uint32_t playerid = user.second.p_id;
		int32_t noaction = user.second.p_noaction;
		int64_t lGold = user.second.p_asset;
		if(player && !player->is_robot())
		{// e_msg_cleanout_def
			// wait
			int32_t nReason = 0;
			if(player->get_wait()>=3)
			{
				nReason = 3;
			}
			else if(noaction >= 2)
			{
				nReason = 4;
			}
			else if(lGold<m_room->get_data()->mGoldMinCondition)
			{
				nReason = 5;
			}

			if(nReason > 0)
			{
				auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_clean_out, landlord3_protocols::e_mst_l2c_clean_out);
				sendmsg->set_reason((landlord3_protocols::e_msg_cleanout_def)nReason);
				player->send_msg_to_client(sendmsg);

#if 0
				// clear table
				player->leave_table();
#endif
			}
		}
	}
}

void logic_table::CleanNoAction(uint32_t playerid)
{
	for(auto& user : m_MapTablePlayer)
	{
		uint32_t uid = user.second.p_id;
		if(uid == playerid)
		{
			user.second.p_noaction = 0;
		}
	}
}

void logic_table::onGameCleanOut(uint32_t playerid,int32_t nReason)
{
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_clean_out, landlord3_protocols::e_mst_l2c_clean_out);
	sendmsg->set_reason((landlord3_protocols::e_msg_cleanout_def)nReason);
#if 1
	auto player = get_player_byid(playerid);
	if (player != nullptr)
	{
		player->send_msg_to_client(sendmsg);
	}
#else
	// broadcast
	broadcast_msg_to_client2(sendmsg);
#endif
}


//////////////////////////////////////////////////////////////////////////
void logic_table::robot_heartbeat(double elapsed)
{
	if (m_robotcfg.mIsOpen == 0) return;
	
	int minCount = m_robotcfg.mRobotMinCount;
	int maxCount = m_robotcfg.mRobotMaxCount;
	/*static*/ int requireCount = global_random::instance().rand_int(minCount, maxCount);

	m_robot_elapsed -= elapsed;
	if (m_robot_elapsed <= 0)
	{
		m_robot_elapsed = global_random::instance().rand_int(m_robotcfg.mRobotMinEntry, m_robotcfg.mRobotMaxEntry);
		
		int player_count = 0;
		int robot_count = 0;
		for (auto user : m_MapTablePlayer)
		{
			LPlayerPtr& player = user.second.p_playerptr;
			if(player!=nullptr)
			{
				if(player->is_robot())
					robot_count++;
				else
					player_count++;
			}
		}
		int all_count = player_count + robot_count;
		if(all_count==GAME_PLAYER) return;
		// 只剩机器人了
		if(player_count==0 && robot_count>0)
		{
			for (auto user : m_MapTablePlayer)
			{
				LPlayerPtr& player = user.second.p_playerptr;
				// 游戏中不离开
				if(player->get_status()==eUserState_play || player->get_status()==eUserState_dead) 
					continue;
				if(player && player->is_robot())
				{
                    //暂且不清理机器人;
					//release_robot(player->get_pid());
				}
			}
		}
		/*else*/ if (all_count<GAME_PLAYER && robot_count < requireCount)
		{
			request_robot();
		}
		else
		{
			// 金币不足
			for (auto user : m_MapTablePlayer)
			{
				LPlayerPtr& player = user.second.p_playerptr;
				// 游戏中不离开
				if(player->get_status()==eUserState_play || player->get_status()==eUserState_dead) 
					continue;
				if(player && player->is_robot() &&
					player->get_gold()<m_room->get_data()->mGoldMinCondition)
				{
					release_robot(player->get_pid());
				}
			}
		}

		// 清理
		for (auto user : m_MapTablePlayer)
		{
			LPlayerPtr& player = user.second.p_playerptr;
			// 游戏中不离开
			if(player->get_status()==eUserState_play || player->get_status()==eUserState_dead) 
				continue;
			if(player && player->is_robot())
			{
				int32_t nRound = global_random::instance().rand_int(m_robotcfg.mRobotMinRound, m_robotcfg.mRobotMaxRound);
				if(player->get_round()>=nRound)
				{
					release_robot(player->get_pid());
				}
			}
		}
	}
}

void landlord_space::logic_table::request_robot()
{
	// Robot
	static int minVIP = m_robotcfg.mRobotMinVip;
	static int maxVIP = m_robotcfg.mRobotMaxVip;
	int32_t vip_level = global_random::instance().rand_int(minVIP, maxVIP);	
	GOLD_TYPE gold_min = m_robotcfg.mRobotMinTake;
	GOLD_TYPE gold_max = m_robotcfg.mRobotMaxTake;

	GOLD_TYPE enter_gold = global_random::instance().rand_int(gold_min,gold_max);
	int32_t rid = m_room->get_id();
	int32_t tid = get_id();
	//int tag = (rid<<4)|tid;
    int tag = rid + tid * 100;

	SLOG_CRITICAL<<boost::format("request_robot::rid:%1% tid:%2%,tag:%3%")%rid%tid%tag;
	game_engine::instance().request_robot(tag,enter_gold, vip_level);
}

void landlord_space::logic_table::release_robot(int32_t playerid)
{
	// 
	SLOG_CRITICAL<<boost::format("release_robot:playerid:%1%")%playerid;
	bc_leave_seat(playerid);
	m_room->get_lobby()->robot_leave(playerid);
	game_engine::instance().release_robot(playerid);
}

int32_t landlord_space::logic_table::release_robot_seat()
{
	int32_t seat = INVALID_CHAIR;
	SLOG_CRITICAL<<boost::format("%1%:%2%")%__FUNCTION__%__LINE__;
	int32_t nRobotCount = robot_counter();
	if(nRobotCount<=1) return seat;
	std::map<uint32_t,TablePlayer>  tMapTablePlayer = m_MapTablePlayer;
	for(auto it=tMapTablePlayer.begin();it!=tMapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto uid = user.p_id;
		if(player && player->is_robot())
		{
			bool bRet = player->can_leave_table();
			if(bRet)
			{
				seat = player->get_seat();
#if 0
				// 
				player->leave_table();
#else
				// 先剔除桌子
				player->leave_table();
				// 再释放
				release_robot(uid);
#endif

				return seat;
			}
		}
	}
	return seat;
}

void landlord_space::logic_table::reverse_result(uint32_t reqid,uint32_t resid)
{
	SLOG_CRITICAL<<boost::format("%1%:%2%")%__FUNCTION__%__LINE__;
	int32_t nRobotCount = robot_counter();
	if(nRobotCount==0) return;
	uint32_t robot_req = 0,robot_res = 0;
	robot_req = robot_id(reqid);
	robot_res = robot_id(resid);
	if(robot_req==0 && robot_res==0) return;
	SLOG_CRITICAL<<boost::format("reverse_result:RobotCount:%1%,%2%,%3%")%nRobotCount%robot_req%robot_res;
	int32_t rate = robot_rate();
	int nRandom = global_random::instance().rand_int(1,100);
	if(nRandom <= rate)
	{
		if (nRobotCount>1)
		{// 此时机器是可以输的
			if(robot_req!=0)
			{
				robot_switch(robot_req,global_random::instance().rand_int(1,100));
			}
			if(robot_res!=0)
			{
				robot_switch(robot_res,global_random::instance().rand_int(1,100));
			}
		}
		else if(nRobotCount==1)
		{
			// 此时机器人是不能输的
			if(robot_req!=0)
			{
				robot_switch(robot_req);
			}
			if(robot_res!=0)
			{
				robot_switch(robot_res);
			}
		}
		else
		{
			// 此时无意义
		}
	}
	else
	{
		// 此时机器是可输可赢
// 		if(robot_req!=0)
// 		{
// 			robot_switch(robot_res,global_random::instance().rand_int(1,100));
// 		}
// 		if(robot_res!=0)
// 		{
// 			robot_switch(robot_res,global_random::instance().rand_int(1,100));
// 		}
	}
}

void landlord_space::logic_table::reverse_result()
{
	SLOG_CRITICAL<<boost::format("%1%:%2%")%__FUNCTION__%__LINE__;
	int32_t nRobotCount = robot_counter();
	if(nRobotCount==0) return;
	SLOG_CRITICAL<<boost::format("reverse_result:RobotCount:%1%")%nRobotCount;
	int32_t rate = robot_rate();
	int nRandom = global_random::instance().rand_int(1,100);
	if (nRandom <= rate)
	{
		for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
		{ // 活着的机器人
			auto user = it->second;
			uint32_t uid = user.p_id;
			auto player = user.p_playerptr;
			if(user.p_playing==FALSE) continue;
			if( player && player->is_robot())
			{
				if(nRobotCount==1)
					robot_switch(uid);
				else
					robot_switch(uid,global_random::instance().rand_int(1,100));
			}	
		}
	}
}

void landlord_space::logic_table::robot_switch(uint32_t uid,int nRandom/*=100*/)
{
	
}

int32_t landlord_space::logic_table::robot_counter()
{
	int32_t nCount = 0;
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{ // 活着的机器人
		auto user = it->second;
		auto player = user.p_playerptr;
		if(user.p_playing==FALSE || player==nullptr) continue;
		if(player->is_robot())
		{
			nCount++;
		}
	}
	return nCount;
}

uint32_t landlord_space::logic_table::robot_id(uint32_t uid)
{
	uint32_t robotid = 0;
	auto it = m_MapTablePlayer.find(uid);
	if(it == m_MapTablePlayer.end()) return robotid;
	auto player = it->second.p_playerptr;
	if(player && player->is_robot())
	{
		robotid = uid;
	}

	return robotid;
}

int32_t landlord_space::logic_table::robot_rate()
{
	int32_t nRate = 0;
	

	return nRate;
}

int32_t logic_table::get_robot_size()
{
    int32_t nCount = 0;
    for (auto& item : m_MapTablePlayer)
    {
        LPlayerPtr& player = item.second.p_playerptr;
        if (player && player->is_robot())
            nCount++;
    }
    return nCount;
}

bool logic_table::is_all_robot()
{
    int32_t nCount = 0;
    for (auto& item : m_MapTablePlayer)
    {
        LPlayerPtr& player = item.second.p_playerptr;
        if (player && !player->is_robot())
            return false;
    }
    return true;
}