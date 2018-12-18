#include "stdafx.h"
#include "logic_table.h"
#include "logic_player.h"

#include "logic_room.h"
#include "game_engine.h"
#include "i_game_ehandler.h"
#include "game_db.h"
#include "logic_lobby.h"
#include <net/packet_manager.h>


#include "bmw_def.pb.h"
#include "bmw_logic.pb.h"

#include "enable_random.h"

#include "robot_mgr.h"

#include "time_helper.h"


BMW_SPACE_USING

	static const int MAX_TALBE_PLAYER = 5;//桌子人数
static const int MAX_OP_PLAYER = 4;//观战人数
int64_t logic_table::m_lAndroidWinScore = 0;
#define MAX_TIME	180

logic_table::logic_table(void)
	:m_room(nullptr)
	,m_nMaxPlayerCount(0)
	,m_player_count(0)
	,m_elapse(0.0)
	,m_checksave(0)
	,m_roomcfg(nullptr)
{

	init_game_object();
	// Robot
	memset(&m_robotcfg,0,sizeof(m_robotcfg));

}


logic_table::~logic_table(void)
{
}

void logic_table::release()
{
	store_game_object();

}

void bmw_space::logic_table::init_table(uint16_t tid,int32_t nPlayerCount ,logic_room* room)
{
	// 保存当前房间信息
	m_room = room;
	m_nMaxPlayerCount = nPlayerCount;
	// 设置 Chair ID
	for (int32_t i = 0; i < nPlayerCount; i++)
	{
		m_VecChairMgr.push_back(i);
	}
	// 设置 Table ID
	TableID->set_value(tid);

	if(!load_table())
		create_table();
	// init
	initGame();
	//
	m_fFreeTime = BMW_BaseInfo::GetSingleton()->GetData("FreeTime")->mValue;
	m_fPlaceTime = BMW_BaseInfo::GetSingleton()->GetData("BetTime")->mValue;
	m_fAheadTime = BMW_BaseInfo::GetSingleton()->GetData("AheadTime")->mValue;
	m_fRunTime = BMW_BaseInfo::GetSingleton()->GetData("RunTime")->mValue;
	m_fResultTime = BMW_BaseInfo::GetSingleton()->GetData("ResultTime")->mValue;
	// 
	m_duration = m_fFreeTime;
	//
	m_roomcfg = m_room->get_data();
	m_VecJetton = m_roomcfg->mPlaceJetton;
	//
	m_nRate = 0;
	m_lAllTax = 0;
	m_lAITax = 0;
	// 
	ReadLotteryRate();
	// Robot
	const boost::unordered_map<int, BMW_RobotCFGData>& list = BMW_RobotCFG::GetSingleton()->GetMapData();
	for (auto it = list.begin(); it != list.end(); ++it)
	{
		if(it->second.mRoomID == m_room->get_id())
		{
			m_robotcfg = it->second;
			break;
		}
	}
	m_robot_elapsed = global_random::instance().rand_int(m_robotcfg.mRobotMinEntry, m_robotcfg.mRobotMaxEntry);
	m_banker_elapsed = global_random::instance().rand_int(m_robotcfg.mBankerMinEntry, m_robotcfg.mBankerMaxEntry);
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
	TableStatus->set_value(state);
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

	//
	banker_heartbeat(elapsed);

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
	int8_t nStatus = get_status();
	switch (nStatus)
	{
	case eGameState_Free:
		{
			// 空闲时，清理一下
			CleanKickedPlayer();

			if(m_duration<0)
			{
				m_duration = 0;

				onGameNoticeStart();
			}
		}
		break;
	case eGameState_Place:
		{
			if(m_duration < 0)
			{
				m_duration = 0;

				onEventGameStart();
			}
		}
		break;
	case eGameState_Play:
		{
			if (m_duration<0)
			{
				m_duration = 0;
				onGameEnd();
			}
		}
		break;
	case eGameState_End:
		{
			if (m_duration<0)
			{
				m_duration = m_fFreeTime;
				set_status(eGameState_Free);
				// 检测 玩家是否还可以做庄
				CheckApply();
				// 更换庄家
				CheckNextBanker();
				// 桌子复位
				repositGame();
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
	return m_player_count >= m_nMaxPlayerCount;
}

bool bmw_space::logic_table::is_opentable()
{
	return m_player_count >= 1;
}


bool bmw_space::logic_table::is_all_robot()
{
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if(player && !player->is_robot()) 
			return false;
	}
	return true;
}

int32_t bmw_space::logic_table::all_robot()
{
	int32_t nCount = 0;
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if(player && player->is_robot()) 
			nCount++;
	}
	return nCount;
}

unsigned int logic_table::get_max_table_player()
{
	return m_nMaxPlayerCount;
}

LPlayerPtr& logic_table::get_player(int index)
{
	assert(0);
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

uint32_t bmw_space::logic_table::get_id_byseat(int32_t seat)
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
		//	bc_enter_seat(seat, player);
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
		table_player.p_leave = false;
		table_player.p_time = time_helper::instance().get_tick_count();
		table_player.p_runaway = false;
		table_player.p_background = false;
		table_player.p_kick = false;
		tagPlayerCtrlAttr attr;
		bool bRet = player->get_player_ctrl(attr);
		int nTag = Tag_UserA;
		if(bRet)
		{
			nTag = attr.nTag;
		}
		table_player.p_tag = nTag;
		table_player.p_percent = attr.fwinPercent;
		m_MapTablePlayer.insert(std::make_pair(uid,table_player));

		if(get_status()==eGameState_Play)
		{
			player->set_status(eUserState_Wait);
		}
		else
		{
			player->set_status(eUserState_free);
		}

		SLOG_CRITICAL<<boost::format("logic_table::enter_table:%1%,seatid:%2%")%get_id()%seat;
		SLOG_CRITICAL<<boost::format("logic_table::enter_table:[Tag:%d,Win:%f]")%attr.nTag%attr.fwinPercent;

		bc_enter_seat(table_player);
		return 1;
	}

	player->join_table(nullptr,-1);
	return 12;
}

bool logic_table::leave_table(uint32_t pid)
{
	// 清理用户
	auto it = m_MapTablePlayer.find(pid);
	if(it != m_MapTablePlayer.end())
	{
		if (can_leave_table(pid) == false)
		{ // 游戏已经开始不能离开
			auto& tableuser = it->second;
			tableuser.p_runaway = true;
			return false;
		}
		// clear
		for (auto applyit=m_ApplyBanker.begin();applyit!=m_ApplyBanker.end();applyit++)
		{
			if((*applyit) == pid)
			{
				m_ApplyBanker.erase(applyit);
				break;
			}
		}
		//
		auto user = it->second;
		int32_t seat = user.p_idx;
		m_VecChairMgr.push_back(seat);
		LPlayerPtr& player = user.p_playerptr;
		// 清理用户
// 		if(player->is_robot())
// 		{
// 			
// 		}
// 		else
// 		{
// 			if(can_leave_table(pid)==false)
// 			{ // 游戏已经开始不能离开
// 				auto& tableuser = it->second;
// 				tableuser.p_runaway = true;
// 				return false;
// 			}
// 		}
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

		// clear
		m_MapTablePlayer.erase(it);

		// clear chip history
		auto history = m_MapAreaChipHistory.find(pid);
		if(history != m_MapAreaChipHistory.end())
		{
			m_MapAreaChipHistory.erase(history);
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

	return true;
}

bool logic_table::can_leave_table(uint32_t pid)
{
	auto it = m_MapTablePlayer.find(pid);
	if(it != m_MapTablePlayer.end())
	{
		auto user = it->second;
		LPlayerPtr player = user.p_playerptr;
		auto uid = user.p_id;
		if(player)
		{
			// 当前庄 或者 已下注
			if(uid == m_nBankerID)
				return false;
			if(IsPlace(pid)) return false;
			// 清理上庄列表
			auto applyit = std::find(m_ApplyBanker.begin(),m_ApplyBanker.end(),uid);
			if(applyit!=m_ApplyBanker.end())
			{
				m_ApplyBanker.erase(applyit);
				// 更新上庄列表
				onSendUnApplyBankerResult(uid,1);
			}
			return true;
		}
	}
	// 
	return false;
}

void bmw_space::logic_table::getout_table(uint32_t pid)
{
	auto it = m_MapTablePlayer.find(pid);
	if (it != m_MapTablePlayer.end())
	{
		auto& tuser = it->second;
		auto player = tuser.p_playerptr;
		tuser.p_kick = true;
	}
}

bool bmw_space::logic_table::IsPlace(uint32_t playerid)
{
	int64_t lAllPlace = 0;
	auto it =  m_MapAreaChip.find(playerid);
	if(it != m_MapAreaChip.end())
	{
		tagAreaChip chip;
		chip = it->second;

		lAllPlace = chip.small_vw + chip.small_benz + chip.small_bmw + chip.small_porsche
			+ chip.big_vw + chip.big_benz + chip.big_bmw + chip.big_porsche;
	}
	if(lAllPlace != 0)
		return true;
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
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_user_enter_seat,bmw_protocols::e_mst_l2c_user_enter_seat);
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
	playerinfo->set_player_region(player->GetUserRegion());
	broadcast_msg_to_client2(sendmsg);

}


void bmw_space::logic_table::bc_enter_seat(TablePlayer& tabplayer)
{
	LPlayerPtr player = tabplayer.p_playerptr;
	if(!player) return;
	int seat_index = tabplayer.p_idx;
	player->join_table(this,seat_index);
	int32_t nTag = tabplayer.p_tag;
	double fPercent = tabplayer.p_percent;
	//  notice other
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_user_enter_seat,bmw_protocols::e_mst_l2c_user_enter_seat);
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
	playerinfo->set_player_region(player->GetUserRegion());

	//auto playertag = playerinfo->mutable_playertag();
	//playertag->set_type(nTag);
	//playertag->set_percent(fPercent);

#if 0
	broadcast_msg_to_client2(sendmsg);
#else
	broadcast_msg_to_client3(sendmsg);
#endif

}

void logic_table::bc_leave_seat(uint32_t player_id)
{
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_user_leave_seat, bmw_protocols::e_mst_l2c_user_leave_seat);
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
	mongo::BSONObj b = db_game::instance().findone(DB_BMWTABLE, BSON("room_id"<<m_room->get_id() << "table_id" << TableID->get_value()));
	if(b.isEmpty())
		return false;

	return from_bson(b);

}

bool logic_table::store_game_object(bool to_all)
{
	if(!has_update())
		return true;

	auto err = db_game::instance().update(DB_BMWTABLE, BSON("room_id"<<m_room->get_id() << "table_id" << TableID->get_value()), BSON("$set"<<to_bson(to_all)));
	if(!err.empty())
	{
		SLOG_ERROR << "logic_table::store_game_object :" <<err;
		return false;
	}

	m_checksave = 0;

	return true;
}


void bmw_space::logic_table::req_scene(uint32_t playerid)
{
	// 重置 Home键
	auto player_it = m_MapTablePlayer.find(playerid);
	if (player_it != m_MapTablePlayer.end())
	{
		auto &user = player_it->second;
		user.p_background = false;
		auto player = user.p_playerptr;
		if (player) player->set_serverflag(0);
	}

}

boost::shared_ptr<bmw_protocols::packetl2c_scene_info_free> logic_table::get_scene_info_msg_free()
{
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_scene_info_free,bmw_protocols::e_mst_l2c_scene_info_free);
	//
	uint32_t &nBanker = m_nBankerID;
	int64_t &nGold = m_nBankerGold;
	int32_t &nRound = m_nBankerRound;
	int64_t &nResult = m_nBankerResult;
	//
	auto freeinfo = sendmsg->mutable_freeinfo();
	int8_t nGameState = get_status();
	int32_t nSpareTime = m_duration;
	freeinfo->set_status(nGameState);
	freeinfo->set_roomid(m_room->get_id());
	freeinfo->set_sparetime(nSpareTime);
	// banker info
	auto bankerinfo = freeinfo->mutable_bankerinfo();
	bankerinfo->set_id(nBanker);
	bankerinfo->set_money(nGold);
	bankerinfo->set_round(nRound);
	bankerinfo->set_result(nResult);
	// banker list
	for(auto id : m_ApplyBanker)
	{
		freeinfo->add_bankerlist(id);
	}
	// player info
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto seat = user.p_idx;
		auto nTag = user.p_tag;
		auto fPercent = user.p_percent;
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
			playerinfo->set_player_region(player->GetUserRegion());

			/*auto playertag = playerinfo->mutable_playertag();
			playertag->set_type(nTag);
			playertag->set_percent(fPercent);*/

			playerinfo->clear_usergold();
		}
	}

	return sendmsg;
}

boost::shared_ptr<bmw_protocols::packetl2c_scene_info_play> bmw_space::logic_table::get_scene_info_msg_play(uint32_t uid)
{
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_scene_info_play,bmw_protocols::e_mst_l2c_scene_info_play);
	//
	uint32_t &nBanker = m_nBankerID;
	int64_t &nGold = m_nBankerGold;
	int32_t &nRound = m_nBankerRound;
	int64_t &nResult = m_nBankerResult;
	//
	auto playinfo = sendmsg->mutable_playinfo();
	int8_t nGameState = get_status();
	playinfo->set_status(nGameState);
	int32_t rid = m_room->get_id();
	playinfo->set_roomid(rid);
	// banker info
	auto bankerinfo = playinfo->mutable_bankerinfo();
	bankerinfo->set_id(nBanker);
	bankerinfo->set_money(nGold);
	bankerinfo->set_round(nRound);
	bankerinfo->set_result(nResult);
	// banker list
	for(auto id : m_ApplyBanker)
	{
		playinfo->add_bankerlist(id);
	}
	// left time
	playinfo->set_lefttime(m_duration);
	// area info
	tagAreaChip areachip;
	auto it = m_MapAreaChip.find(uid);
	if(it != m_MapAreaChip.end())
	{
		areachip = it->second;
	}
	playinfo->mutable_arearinfo()->Reserve(PLACE_AREA);
	for (int32_t i=1;i<=PLACE_AREA;i++)
	{
		auto areainfo = playinfo->add_arearinfo();
		areainfo->set_areaid(i);
		if(i==ID_SMALL_VW)
		{
			areainfo->set_areatotal(m_lAllSmallVW);
			areainfo->set_areamoney(areachip.small_vw);
		}
		else if(i==ID_BIG_VW)
		{
			areainfo->set_areatotal(m_lAllBigVW);
			areainfo->set_areamoney(areachip.big_vw);
		}
		else if(i==ID_SMALL_Benz)
		{
			areainfo->set_areatotal(m_lAllSmallBenz);
			areainfo->set_areamoney(areachip.small_benz);
		}
		else if(i==ID_BIG_Benz)
		{
			areainfo->set_areatotal(m_lAllBigBenz);
			areainfo->set_areamoney(areachip.big_benz);
		}
		else if(i==ID_SMALL_BMW)
		{
			areainfo->set_areatotal(m_lAllSmallBMW);
			areainfo->set_areamoney(areachip.small_bmw);
		}
		else if(i==ID_BIG_BMW)
		{
			areainfo->set_areatotal(m_lAllBigBMW);
			areainfo->set_areamoney(areachip.big_bmw);
		}
		else if(i==ID_SMALL_Porsche)
		{
			areainfo->set_areatotal(m_lAllSmallPorsche);
			areainfo->set_areamoney(areachip.small_porsche);
		}
		else if(i==ID_BIG_Porsche)
		{
			areainfo->set_areatotal(m_lAllBigPorsche);
			areainfo->set_areamoney(areachip.big_porsche);
		}
	}
	// player info
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto seat = user.p_idx;
		auto usergold = user.p_asset;
		auto userid = user.p_id;
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
			playerinfo->set_player_region(player->GetUserRegion());

			// play wait
			playerinfo->set_player_status(player->get_status());

			playerinfo->set_usergold(usergold);

		}
	}

	return sendmsg;
}

template<class T>
void bmw_space::logic_table::broadcast_msg_to_client2(T msg)
{
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if (player != nullptr )
		{
#if 0
			player->send_msg_to_client(msg);
#else
			if(!player->is_robot())
			{
				player->send_msg_to_client(msg);
			}
			else
			{
				// Robot
				robot_mgr::instance().recv_packet(player->get_pid(),msg->packet_id(),msg);
			}
#endif
		}
	}
}

template<class T>
void bmw_space::logic_table::broadcast_msg_to_client3(T msg)
{
	Vec_UserID userpids;
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if (player != nullptr)
		{
			auto uid = player->get_pid();
			if (!player->is_robot())
			{
				userpids.push_back(uid);
			}
			else
			{
				robot_mgr::instance().recv_packet(uid, msg->packet_id(), msg);
			}
		}
	}
	if (userpids.size() > 0)
	{
		game_engine::instance().get_handler()->broadcast_msg_to_client(userpids, msg->packet_id(), msg);
	}
}

int32_t bmw_space::logic_table::GetAllUserCount()
{

	return m_MapTablePlayer.size();
}

bool bmw_space::logic_table::onEventUserReady(uint32_t playerid)
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
			if(tuser.p_leave==true)
			{
				SLOG_CRITICAL<<boost::format("onEventUserReady:Robot-id:%1%,leave")%playerid;
				return true;
			}
		}
		else
		{
			SLOG_CRITICAL<<boost::format("onEventUserReady:Player-id:%1%")%playerid;
		}
		if(player->get_status()>eUserState_free)
		{
			SLOG_CRITICAL<<"logic_table::onEventUserReady Can't Ready:usestate";
			nResult = eReadyResult_Faild_userstate;
		}
		else if(get_status()!=eGameState_Free && get_status()!=eGameState_Place)
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
			SLOG_CRITICAL<<boost::format("onEventUserReady succeed:id:%1%")%playerid;
		}
	}

	auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_ask_ready_result,bmw_protocols::e_mst_l2c_ask_ready_result);
	sendmsg->set_playerid(playerid);
	sendmsg->set_result(nResult);
	// TODO Bug to fix : can't send msg
	//	int ret = broadcast_msg_to_client(sendmsg);
	// TODO send msg ok
	//	m_room->broadcast_msg_to_client(sendmsg);

	broadcast_msg_to_client2(sendmsg);


	return true;
}

void bmw_space::logic_table::initGame(bool bResetAll/*=false*/)
{
	set_status(eGameState_Free);

	// 初始化游戏参数
	m_bActiveUserCnt = 0;

	m_bPlayingCnt = 0;

	m_nBankerID = 0;
	m_nBankerGold = 0;
	m_nBankerRound = 0;
	m_nBankerResult = 0;

	m_nCurOperatorID = 0;

	m_nLastWiner = 0;

	m_lCurJetton = 0;

	m_lTotalSliver = 0;
	m_lInventPool = 0;
	m_lEqualize = 0;

	m_nFirstID = 0;
	m_MapCompareNexus.clear();
	m_VecActive.clear();
	m_VecPlaying.clear();
	m_VecAllin.clear();

	m_nShowHandCnt = 0;

	m_nGameRound = 0;

	m_nEndType = eGameEndType_null;

	//////////////////////////////////////////////////////////////////////////
	m_dwLastRandTick = GetSecondsCount();
	m_MapAreaChip.clear();
	m_MapAreaChipHistory.clear();
	m_lAllBigBenz = 0;
	m_lAllSmallBenz = 0;
	m_lAllBigBMW = 0;
	m_lAllSmallBMW = 0;
	m_lAllBigVW = 0;
	m_lAllSmallVW = 0;
	m_lAllBigPorsche = 0;
	m_lAllSmallPorsche = 0;

	m_lAllBigBenz2 = 0;
	m_lAllSmallBenz2 = 0;
	m_lAllBigBMW2 = 0;
	m_lAllSmallBMW2 = 0;
	m_lAllBigVW2 = 0;
	m_lAllSmallVW2 = 0;
	m_lAllBigPorsche2 = 0;
	m_lAllSmallPorsche2 = 0;

	m_ListHistory.clear();

	m_cbResultIndex = 0;

	m_lFleeScore = 0;

	m_MapWiner.clear();

	m_unApply.clear();

	m_enterback.clear();
}


void bmw_space::logic_table::repositGame()
{
	SLOG_CRITICAL<<"------------RepositGame------------";
	// 游戏数据初始化
	m_nCurOperatorID = 0;

	m_lCurJetton = 0;

	m_lTotalSliver = 0;
	m_lInventPool = 0;
	m_lEqualize = 0;

	m_nFirstID = 0;
	m_MapCompareNexus.clear();
	m_VecActive.clear();
	m_VecPlaying.clear();

	m_nGameRound = 0;

	m_nEndType = eGameEndType_null;

	m_nShowHandCnt = 0;
	m_VecAllin.clear();
	// 下注
// 	m_MapAreaChipHistory.clear();
	for (auto chip : m_MapAreaChip)
	{
		// 迭代下注的记录
		if(IsPlace(chip.first)==false) continue;
	//	m_MapAreaChipHistory.insert(std::make_pair(chip.first,chip.second));
		m_MapAreaChipHistory[chip.first] = chip.second;
	}
	m_MapAreaChip.clear();
	m_lAllBigBenz = 0;
	m_lAllSmallBenz = 0;
	m_lAllBigBMW = 0;
	m_lAllSmallBMW = 0;
	m_lAllBigVW = 0;
	m_lAllSmallVW = 0;
	m_lAllBigPorsche = 0;
	m_lAllSmallPorsche = 0;

	m_lAllBigBenz2 = 0;
	m_lAllSmallBenz2 = 0;
	m_lAllBigBMW2 = 0;
	m_lAllSmallBMW2 = 0;
	m_lAllBigVW2 = 0;
	m_lAllSmallVW2 = 0;
	m_lAllBigPorsche2 = 0;
	m_lAllSmallPorsche2 = 0;

	m_cbResultIndex = 0;
	m_lFleeScore = 0;
	m_MapWiner.clear();
	// 重置数据
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		auto uid = user.p_id;
		user.p_active = FALSE;
		user.p_playing = FALSE;
		user.p_asset = player->get_gold();
		user.p_expend = 0;
		user.p_result = 0;
		user.p_tax = 0;
		player->set_status(eGameState_Free);

		// 清理机器人
		if(player->is_robot())
		{
			int32_t nRound = global_random::instance().rand_int(m_robotcfg.mRobotMinRound, m_robotcfg.mRobotMaxRound);
			int32_t nRRound = player->get_round();
			SLOG_CRITICAL<<boost::format("repositGame:[uid:%d,round:%d,robot:%d]")%uid%nRRound%nRound;
			if(player->get_gold()<m_room->get_data()->mGoldMinCondition || 
				nRRound>=nRound)
			{
				release_robot(uid);
			}
		}
	}

	// 清理不足条件用户
//	CleanOutPlayer();

	// 清理逃跑用户
	CleanRunaway();

	// 清理进入后台用户
	OnDealEnterBackground();

}

bool bmw_space::logic_table::onNoticeBanker()
{
	SLOG_CRITICAL<<boost::format("%1%")%__FUNCTION__;
	uint32_t &nBanker = m_nBankerID;
	int64_t &nGold = m_nBankerGold;
	int32_t &nRound = m_nBankerRound;
	int64_t &nResult = m_nBankerResult;
	if(nBanker==0)
	{ // 系统
		nGold = m_roomcfg->mBankerGold;
		nRound = 0;
	}
	else
	{ // 
		auto player_it = m_MapTablePlayer.find(nBanker);
		if(player_it == m_MapTablePlayer.end())
		{ // 
			if(m_ApplyBanker.size()>0)
			{ // 玩家做庄
				uint32_t id  = m_ApplyBanker.front();
				m_ApplyBanker.pop_front();
				auto it = m_MapTablePlayer.find(id);
				if(it!=m_MapTablePlayer.end())
				{
					auto player = it->second.p_playerptr;
					if(player)
					{
						nBanker = id;
						nGold = player->get_gold();
						nRound = 0;
					}
				}
			}
			else
			{ // 系统做庄
				nBanker = 0;
				nGold = m_roomcfg->mBankerGold;
				nRound = 0;
			}
		}
	}
	SLOG_CRITICAL<<boost::format("Banker info:bankerid:%d,money:%d,round:%d")%nBanker%nGold%nRound;
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_change_banker, bmw_protocols::e_mst_l2c_notice_change_banker);
	auto banker = sendmsg->mutable_bankerinfo();
	banker->set_id(nBanker);
	banker->set_money(nGold);
	banker->set_round(nRound);
	banker->set_result(nResult);
#if 0
	broadcast_msg_to_client2(sendmsg);
#else
	broadcast_msg_to_client3(sendmsg);
#endif
	return true;
}


bool bmw_space::logic_table::onNoticeChangeBanker()
{
	SLOG_CRITICAL<<boost::format("%1%")%__FUNCTION__;
	uint32_t &nBanker = m_nBankerID;
	int64_t &nGold = m_nBankerGold;
	int32_t &nRound = m_nBankerRound;
	int64_t &nResult = m_nBankerResult;
	if(nBanker==0)
	{ // 系统
		nGold = m_roomcfg->mBankerGold;
		nRound = 0;
		nResult = 0;
	}
	else
	{
		auto player = get_player_byid(nBanker);
		if(player)
		{
			nGold = player->get_gold();
		}
		else
		{
			assert(0);
		}
	}
	SLOG_CRITICAL<<boost::format("Change Banker:bankerid:%d,money:%d,round:%d")%nBanker%nGold%nRound;
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_change_banker, bmw_protocols::e_mst_l2c_notice_change_banker);
	auto banker = sendmsg->mutable_bankerinfo();
	banker->set_id(nBanker);
	banker->set_money(nGold);
	banker->set_round(nRound);
	banker->set_result(nResult);
#if 0
	broadcast_msg_to_client2(sendmsg);
#else
	broadcast_msg_to_client3(sendmsg);
#endif
	return true;
}

bool logic_table::onGameNoticeStart()
{
	SLOG_CRITICAL<<"------------onGameNoticeStart------------";
	onNoticeBanker();
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_notice_start, bmw_protocols::e_mst_l2c_notice_start);
#if 0
	broadcast_msg_to_client2(sendmsg);
#else
	broadcast_msg_to_client3(sendmsg);
#endif

	set_status(eGameState_Place);
	m_duration = m_fPlaceTime /*+ TIME_LESS*/;
	return true;
}

bool bmw_space::logic_table::onGameBetArea(uint32_t playerid,int32_t nArea,int64_t lGold)
{
	SLOG_CRITICAL<<boost::format("%1%:playerid:%2%,area:%3%,gold:%4%")%__FUNCTION__%playerid%nArea%lGold;
	// check player
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<boost::format("[DANGER]%1%: Can't find this user:%2%")%__FUNCTION__%playerid;
		return false;
	}
	auto &user =  player_it->second;
	auto player = user.p_playerptr;

	if(player && player->is_robot())
	{
		SLOG_CRITICAL<<boost::format("%1%:Robot-id:%2%")%__FUNCTION__%playerid;
	}
	if (user.p_leave == true)
	{
		SLOG_WARNING << boost::format("Waring: leave bet :%1%")%__FUNCTION__;
		return false;
	}
	// check money
	int32_t nBetCondition = m_roomcfg->mBetCondition;
	if (player->is_payer() == false && player->get_gold()< nBetCondition)
	{
		SLOG_CRITICAL << boost::format("%1%:not pay: %2%") % __FUNCTION__%playerid;
		onSendBetResult(playerid, nArea, nBetCondition, 5);
		return false;
	}
	// check game state
	int32_t nGameState= get_status();
	if( nGameState != eGameState_Place)
	{
		SLOG_CRITICAL<<boost::format("%1%:game state err:%2%")%__FUNCTION__%nGameState;
		onSendBetResult(playerid,nArea,lGold,4);
		return false;
	}
	if(playerid == m_nBankerID)
	{
		SLOG_ERROR<<boost::format("%1%:banker can't bet:%2%")%__FUNCTION__%m_nBankerID;
		return false;
	}
	// check invalid
	if(!isVaildArea(nArea))
	{
		SLOG_ERROR<<boost::format("[DANGER]%1%:In-valid area:%2%")%__FUNCTION__%nArea;
		return false;
	}
	if(lGold<=0)
	{
		SLOG_ERROR<<boost::format("[DANGER]%1%:err gold:%2%")%__FUNCTION__%lGold;
		return false;
	}
	if(!isVaildGold(lGold))
	{
		SLOG_ERROR<<boost::format("[DANGER]%1%:In-valid gold:%2%")%__FUNCTION__%lGold;
		return false;
	}

	int64_t user_gold = player->get_gold();
	int64_t lAllJetton = CountPlayerAllChip(playerid);
	if(user_gold < lAllJetton + lGold)
	{
		SLOG_CRITICAL<<boost::format("%1%:not enough gold:%2%")%__FUNCTION__%user_gold;
		onSendBetResult(playerid,nArea,lGold,3);
		return false;
	}
	// area limit check
	//区域倍率
	int32_t cbMultiple[] = {0,5,5,5,5,10,20,30,40};
	int64_t lAreaJetton = CountAreaAllChip(nArea);
	if(((lAreaJetton+lGold)*cbMultiple[nArea]) > m_nBankerGold)
	{
		SLOG_CRITICAL<<boost::format("%1%:out of area range")%__FUNCTION__;
		onSendBetResult(playerid,nArea,lGold,2);
		return false;
	}
	// user limit check
	if(m_roomcfg->mPlayerLimit < lAllJetton + lGold)
	{
		SLOG_CRITICAL<<boost::format("%1%:out of user range")%__FUNCTION__;
		onSendBetResult(playerid,nArea,lGold,2);
		return false;
	}
	// check area
	if(CheckBetArea(playerid,nArea,lGold))
	{
		SLOG_CRITICAL<<boost::format("Place Succeed:playerid:%d,money:%d,area:%d")%playerid%lGold%nArea;
		onSendBetResult(playerid,nArea,lGold,1);

		// clear history
// 		auto it_history = m_MapAreaChipHistory.find(playerid);
// 		if(it_history != m_MapAreaChipHistory.end())
// 		{
// 			m_MapAreaChipHistory.erase(it_history);
// 		}
	}
	// check is full
	if(CheckPlaceFull())
	{
		auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_notice_placefull, bmw_protocols::e_mst_l2c_notice_place_full);
		sendmsg->set_lefttime(m_fAheadTime);
		m_duration = m_fAheadTime;
	}

	return true;
}

void bmw_space::logic_table::onSendBetResult(uint32_t playerid,int32_t nArea,int64_t lGold,int32_t nRet)
{
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_ask_place_bet_result, bmw_protocols::e_mst_l2c_place_bet_result);
	sendmsg->set_betid(playerid);
	sendmsg->set_betarea(nArea);
	sendmsg->set_betvalue(lGold);
	sendmsg->set_betres((bmw_protocols::e_msg_bet_def)nRet);
	if(nRet == 1)
	{
#if 0
		broadcast_msg_to_client2(sendmsg);
#else
		broadcast_msg_to_client3(sendmsg);
#endif
	}
	else
	{ // 失败单发
		auto it = m_MapTablePlayer.find(playerid);
		if(it != m_MapTablePlayer.end())
		{
			auto player = it->second.p_playerptr;
			if(player) player->send_msg_to_client(sendmsg);
		}
	}

}

bool bmw_space::logic_table::onGameApplyBanker(uint32_t playerid)
{
	SLOG_CRITICAL<<boost::format("%1%:playerid:%2%")%__FUNCTION__%playerid;
	// check player
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<boost::format("[DANGER]%1%: Can't find this user:%2%")%__FUNCTION__%playerid;
		return false;
	}
	auto &user =  player_it->second;
	auto player = user.p_playerptr;
	if(player && player->is_robot())
	{
		SLOG_CRITICAL<<boost::format("%1%:Robot-id:%2%")%__FUNCTION__%playerid;
	}
	if (user.p_leave == true)
	{
		SLOG_WARNING << boost::format("Waring: leave apply :%1%") % __FUNCTION__;
		return false;
	}
	// check
	if(player==nullptr) return false;
	//
	auto usergold = player->get_gold();
	if(usergold < m_roomcfg->mBankerGold)
	{
		SLOG_CRITICAL<<boost::format("%1%:not enough gold:%2%")%__FUNCTION__%usergold;
		onSendApplyBankerResult(playerid,2);
		return false;
	}
	// exist
	if(isApplyBank(playerid))
	{
		SLOG_CRITICAL<<boost::format("%1%:is exist apply")%__FUNCTION__;
		onSendApplyBankerResult(playerid,3);
		return false;
	}
	// 
	m_ApplyBanker.push_back(playerid);
	onSendApplyBankerResult(playerid,1);
	SLOG_CRITICAL<<boost::format("ApplyBanker Succeed:playerid:%d")%playerid;
	/*
	tips: 如在空闲状态且没有庄则切换庄家
	*/
	if(get_status()==eGameState_Free && m_nBankerID==0)
	{
		ChangeBanker(false);
	}

	return true;
}

void bmw_space::logic_table::onSendApplyBankerResult(uint32_t playerid , int32_t nRet)
{
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_ask_apply_banker_result, bmw_protocols::e_mst_l2c_apply_banker_result);
	sendmsg->set_bankid(playerid);
	sendmsg->set_bankres((bmw_protocols::e_msg_bank_def)nRet);
	for(auto id : m_ApplyBanker)
	{
		sendmsg->add_banklist(id);
	}

	if (nRet==1)
	{
#if 0
		broadcast_msg_to_client2(sendmsg);
#else
		broadcast_msg_to_client3(sendmsg);
#endif
	}
	else
	{// 失败单发
		auto it = m_MapTablePlayer.find(playerid);
		if(it != m_MapTablePlayer.end())
		{
			auto player = it->second.p_playerptr;
			if(player) player->send_msg_to_client(sendmsg);	
		}
	}

}

bool bmw_space::logic_table::onGameUnApplyBanker(uint32_t playerid)
{
	/*
	1、当前庄
	游戏状态决定是否可以离开
	2、庄队列里面
	任何时段均可离开
	如果下庄成功则检测下一个庄、没有则系统做庄
	*/
	SLOG_CRITICAL<<boost::format("%1%:playerid:%2%")%__FUNCTION__%playerid;
	// check player
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR<<boost::format("[DANGER]%1%: Can't find this user:%2%")%__FUNCTION__%playerid;
		return false;
	}
	auto &user =  player_it->second;
	auto player = user.p_playerptr;
	if(player && player->is_robot())
	{
		SLOG_CRITICAL<<boost::format("%1%:Robot-id:%2%")%__FUNCTION__%playerid;
	}
	// check
	if(player==nullptr) return false;
	//
	if(playerid==m_nBankerID)
	{
		int nGameState = get_status();
		if(nGameState!=eGameState_Free)
		{
			auto unit = std::find(m_unApply.begin(),m_unApply.end(),playerid);
			if(unit!=m_unApply.end())
			{
				SLOG_CRITICAL<<boost::format("%1%:repeat apply %2%")%__FUNCTION__%playerid;
				return true;
			}
			//	SLOG_CRITICAL<<boost::format("%1%:game state can't %2%")%__FUNCTION__%playerid;
			onSendUnApplyBankerResult(playerid,4);
			return true;
		}

		ChangeBanker(true);
	}
	else
	{
		// exist
		if(!isApplyBank(playerid))
		{
			SLOG_CRITICAL<<boost::format("%1%:is not exist apply")%__FUNCTION__;
			onSendUnApplyBankerResult(playerid,0);
			return true;
		}

		for (auto it=m_ApplyBanker.begin();it!=m_ApplyBanker.end();it++)
		{
			if((*it) == playerid)
			{
				m_ApplyBanker.erase(it);
				break;
			}
		}
		// e_bank_succed
		onSendUnApplyBankerResult(playerid,1);
		SLOG_CRITICAL<<boost::format("UnApplyBanker Succeed:playerid:%d")%playerid;
	}

	return true;
}

void bmw_space::logic_table::onSendUnApplyBankerResult(uint32_t playerid , int32_t nRet)
{
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_ask_unapply_banker_result, bmw_protocols::e_mst_l2c_unapply_banker_result);
	sendmsg->set_bankid(playerid);
	sendmsg->set_bankres((bmw_protocols::e_msg_bank_def)nRet);
	for(auto id : m_ApplyBanker)
	{
		sendmsg->add_banklist(id);
	}
	if(nRet==1)
	{
#if 0
		broadcast_msg_to_client2(sendmsg);
#else
		broadcast_msg_to_client3(sendmsg);
#endif
	}
	else if(nRet==4)
	{ // 延时下庄
		auto player = get_player_byid(playerid);
		if(player) player->send_msg_to_client(sendmsg);

		m_unApply.push_back(playerid);
	}
	else
	{// 失败单发
		auto player = get_player_byid(playerid);
		if(player) player->send_msg_to_client(sendmsg);
	}

}

bool bmw_space::logic_table::onGameRequestHistory(uint32_t playerid)
{
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_respone_history, bmw_protocols::e_mst_l2c_history);
	for(auto id : m_ListHistory)
	{
		sendmsg->add_areaid(id);
	}

	auto player = get_player_byid(playerid);
	if(player) player->send_msg_to_client(sendmsg);

	// 	auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_respone_history, bmw_protocols::e_mst_l2c_history);
	// 	for(auto id : m_ListHistory)
	// 	{
	// 		sendmsg->add_areaid(id);
	// 	}
	// 	broadcast_msg_to_client2(sendmsg);

	return true;
}

bool bmw_space::logic_table::onGamePlaceAgain(uint32_t playerid)
{
	auto player = get_player_byid(playerid);
	if(!player)
	{
		SLOG_ERROR<<boost::format("[DANGER]%1%: Can't find this user:%2%")%__FUNCTION__%playerid;
		return false;
	}
	// check game state
	int32_t nGameState= get_status();
	if( nGameState != eGameState_Place)
	{
		SLOG_ERROR<<boost::format("%1%:game state err:%2%")%__FUNCTION__%nGameState;
		return false;
	}
	if(playerid == m_nBankerID)
	{
		SLOG_ERROR<<boost::format("%1%:banker can't bet:%2%")%__FUNCTION__%playerid;
		return false;
	}
	// 
	int64_t lUserGold = player->get_gold();
	int64_t lAllJetton = CountPlayerAllChip(playerid);
	auto it = m_MapAreaChipHistory.find(playerid);
	if(it!=m_MapAreaChipHistory.end())
	{
		SLOG_CRITICAL<<boost::format("onGamePlaceAgain:playerid:%d")%playerid;
		auto chip = it->second;

		int64_t nPlaceArea[PLACE_AREA+1] = {0,
			chip.small_vw,chip.small_benz, chip.small_bmw ,chip.small_porsche ,
			chip.big_vw,  chip.big_benz, chip.big_bmw ,chip.big_porsche ,
		};

		int64_t nAll = 0 ;
		for (int32_t i=0;i<PLACE_AREA+1;i++)
		{
			nAll += nPlaceArea[i];
		}
		if(nAll==0) return true;
		if(lUserGold < nAll+lAllJetton)
		{
			// 金币不足
			onSendPlaceAgainResult(playerid,2);
			return false;
		}
		
		if(CheckPlaceFull())
		{
			// 已达上限
			onSendPlaceAgainResult(playerid,3);
			return false;
		}

		// 成功
		onSendPlaceAgainResult(playerid,1);
		// 
		SLOG_CRITICAL<<" [Place Again] ";
		SLOG_CRITICAL<<"===================";
		for (int32_t i=1;i<PLACE_AREA+1;i++)
		{
			int64_t lPlace = nPlaceArea[i] ;
			// 拆分
			while (lPlace>0)
			{
				for (auto it=m_VecJetton.rbegin();it!=m_VecJetton.rend();it++)
				{
					int64_t lJetton = (*it);
					if(lPlace >= lJetton)
					{
						lPlace -= lJetton;
						onGameBetArea(playerid,i,lJetton);
						break;
					}
				}
			}
		}
		SLOG_CRITICAL<<"===================";

	}

	// clear chip history
	auto history = m_MapAreaChipHistory.find(playerid);
	if(history != m_MapAreaChipHistory.end())
	{
		m_MapAreaChipHistory.erase(history);
	}

	return true;
}


void bmw_space::logic_table::onSendPlaceAgainResult(uint32_t playerid , int32_t nRet)
{
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_ask_place_again_result, bmw_protocols::e_mst_c2l_ask_place_again_result);
	sendmsg->set_ret((bmw_protocols::e_msg_placeagain_def)nRet);
	auto player = get_player_byid(playerid);
	if(player) player->send_msg_to_client(sendmsg);

}

bool bmw_space::logic_table::CheckBetArea(uint32_t playerid,int32_t nArea,int64_t lGold)
{
	// area check
	bool bPlaceSuccess = true;
	switch (nArea)
	{
	case ID_SMALL_VW:
		{
			if(CountSmallVWChip(playerid,lGold) >= 0)
			{
				auto it = m_MapAreaChip.find(playerid);
				if(it!=m_MapAreaChip.end())
				{
					auto & area = it->second;
					area.small_vw += lGold;
				}
				else
				{
					tagAreaChip chip;
					chip.small_vw = lGold;
					m_MapAreaChip.insert(std::make_pair(playerid,chip));
				}
				m_lAllSmallVW += lGold;
				// 记录用户下注情况
				auto player = get_player_byid(playerid);
				if (player && player->is_robot() == false)
				{
					m_lAllSmallVW2 += lGold;
				}
			}
			else
			{
				bPlaceSuccess = false;
			}
		}
		break;
	case ID_BIG_VW:
		{
			if(CountBigVWChip(playerid,lGold) >= 0)
			{
				auto it = m_MapAreaChip.find(playerid);
				if(it!=m_MapAreaChip.end())
				{
					auto & area = it->second;
					area.big_vw += lGold;
				}
				else
				{
					tagAreaChip chip;
					chip.big_vw = lGold;
					m_MapAreaChip.insert(std::make_pair(playerid,chip));
				}
				m_lAllBigVW += lGold;
				// 记录用户下注情况
				auto player = get_player_byid(playerid);
				if (player && player->is_robot() == false)
				{
					m_lAllBigVW2 += lGold;
				}
			}
			else
			{
				bPlaceSuccess = false;
			}
		}
		break;
	case ID_SMALL_Benz:
		{
			if(CountSmallBenzChip(playerid,lGold) >= 0)
			{
				auto it = m_MapAreaChip.find(playerid);
				if(it!=m_MapAreaChip.end())
				{
					auto & area = it->second;
					area.small_benz += lGold;
				}
				else
				{
					tagAreaChip chip;
					chip.small_benz = lGold;
					m_MapAreaChip.insert(std::make_pair(playerid,chip));
				}
				m_lAllSmallBenz += lGold;
				// 记录用户下注情况
				auto player = get_player_byid(playerid);
				if (player && player->is_robot() == false)
				{
					m_lAllSmallBenz2 += lGold;
				}
			}
			else
			{
				bPlaceSuccess = false;
			}
		}
		break;
	case ID_BIG_Benz:
		{
			if(CountBigBenzChip(playerid,lGold) >= 0)
			{
				auto it = m_MapAreaChip.find(playerid);
				if(it!=m_MapAreaChip.end())
				{
					auto & area = it->second;
					area.big_benz += lGold;
				}
				else
				{
					tagAreaChip chip;
					chip.big_benz = lGold;
					m_MapAreaChip.insert(std::make_pair(playerid,chip));
				}
				m_lAllBigBenz += lGold;
				// 记录用户下注情况
				auto player = get_player_byid(playerid);
				if (player && player->is_robot() == false)
				{
					m_lAllBigBenz2 += lGold;
				}
			}
			else
			{
				bPlaceSuccess = false;
			}
		}
		break;
	case ID_SMALL_BMW:
		{
			if(CountSmallBMWChip(playerid,lGold) >= 0)
			{
				auto it = m_MapAreaChip.find(playerid);
				if(it!=m_MapAreaChip.end())
				{
					auto & area = it->second;
					area.small_bmw += lGold;
				}
				else
				{
					tagAreaChip chip;
					chip.small_bmw = lGold;
					m_MapAreaChip.insert(std::make_pair(playerid,chip));
				}
				m_lAllSmallBMW += lGold;
				// 记录用户下注情况
				auto player = get_player_byid(playerid);
				if (player && player->is_robot() == false)
				{
					m_lAllSmallBMW2 += lGold;
				}
			}
			else
			{
				bPlaceSuccess = false;
			}
		}
		break;
	case ID_BIG_BMW:
		{
			if(CountBigBMWChip(playerid,lGold) >= 0)
			{
				auto it = m_MapAreaChip.find(playerid);
				if(it!=m_MapAreaChip.end())
				{
					auto & area = it->second;
					area.big_bmw += lGold;
				}
				else
				{
					tagAreaChip chip;
					chip.big_bmw = lGold;
					m_MapAreaChip.insert(std::make_pair(playerid,chip));
				}
				m_lAllBigBMW += lGold;
				// 记录用户下注情况
				auto player = get_player_byid(playerid);
				if (player && player->is_robot() == false)
				{
					m_lAllBigBMW2 += lGold;
				}
			}
			else
			{
				bPlaceSuccess = false;
			}
		}
		break;
	case ID_SMALL_Porsche:
		{
			if(CountSmallPorscheChip(playerid,lGold) >= 0)
			{
				auto it = m_MapAreaChip.find(playerid);
				if(it!=m_MapAreaChip.end())
				{
					auto & area = it->second;
					area.small_porsche += lGold;
				}
				else
				{
					tagAreaChip chip;
					chip.small_porsche = lGold;
					m_MapAreaChip.insert(std::make_pair(playerid,chip));
				}
				m_lAllSmallPorsche += lGold;
				// 记录用户下注情况
				auto player = get_player_byid(playerid);
				if (player && player->is_robot() == false)
				{
					m_lAllSmallPorsche2 += lGold;
				}
			}
			else
			{
				bPlaceSuccess = false;
			}
		}
		break;
	case ID_BIG_Porsche:
		{
			if(CountBigPorscheChip(playerid,lGold) >= 0)
			{
				auto it = m_MapAreaChip.find(playerid);
				if(it!=m_MapAreaChip.end())
				{
					auto & area = it->second;
					area.big_porsche += lGold;
				}
				else
				{
					tagAreaChip chip;
					chip.big_porsche = lGold;
					m_MapAreaChip.insert(std::make_pair(playerid,chip));
				}
				m_lAllBigPorsche += lGold;
				// 记录用户下注情况
				auto player = get_player_byid(playerid);
				if (player && player->is_robot() == false)
				{
					m_lAllBigPorsche2 += lGold;
				}
			}
			else
			{
				bPlaceSuccess = false;
			}
		}
		break;
	default:
		break;
	}

	return bPlaceSuccess;
}


bool bmw_space::logic_table::CheckPlaceFull()
{
	bool bPlaceFull = true;
	int64_t lAreaLimit = m_roomcfg->mAreaLimit;
	if(m_VecJetton.size()==0)
	{
		assert(0);
		return false;
	}
	int64_t nJetton = m_VecJetton[0];
	if((m_lAllBigBenz < lAreaLimit - nJetton) 
		||(m_lAllSmallBenz < lAreaLimit - nJetton) 
		||(m_lAllBigPorsche < lAreaLimit - nJetton) 
		||(m_lAllSmallPorsche < lAreaLimit - nJetton) 
		||(m_lAllBigBMW < lAreaLimit - nJetton) 
		||(m_lAllSmallBMW < lAreaLimit - nJetton) 
		||(m_lAllBigVW < lAreaLimit - nJetton) 
		||(m_lAllSmallVW < lAreaLimit - nJetton) )
	{
		bPlaceFull = false;
	}

	return bPlaceFull;
}

bool logic_table::onEventGameStart()
{
	SLOG_CRITICAL<<"------------GameStart------------";
	auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_run_result, bmw_protocols::e_mst_l2c_run_result);

	RandGameIndex();
	//	m_cbResultIndex = 1;
	int32_t nAreaid = m_cbResultIndex;
	sendmsg->set_areaid(nAreaid);

#if 0
	broadcast_msg_to_client2(sendmsg);
#else
	broadcast_msg_to_client3(sendmsg);
#endif

	set_status(eGameState_Play);
	m_duration = m_fRunTime + TIME_LESS;
	// read tax rate
	auto p_gamehandler = game_engine::instance().get_handler();
	if (p_gamehandler && m_room)
	{
		m_nRate = p_gamehandler->GetRoomCommissionRate(m_room->get_id());
	}
	if (m_nRate <= 0) m_nRate = 0;
	// Add Clean
	for (auto& user : m_MapTablePlayer)
	{
		auto player = user.second.p_playerptr;
		auto&nCount = user.second.p_noaction;
		uint32_t playerid = user.second.p_id;
		if (playerid == m_nBankerID)
		{
			nCount = 0;
			continue;
		}
		if (isApplyBank(playerid) == true)
		{
			nCount = 0;
			continue;
		}
		if (IsPlace(playerid) == true)
		{
			nCount = 0;
			continue;
		}
		nCount++;
	}

	return true;
}

uint32_t bmw_space::logic_table::GetNextPlayerByID(uint32_t playerid)
{
	SLOG_CRITICAL<<"GetNextPlayerByID playerid:"<<playerid;
	uint32_t nextid = 0;
	int32_t seat = get_seat_byid(playerid);
	int32_t nextseat = INVALID_CHAIR;
	int32_t fseat = seat;
	do 
	{
		if(nextseat == seat) break;
#if 0
		nextseat = (fseat-1+GAME_PLAYER)%GAME_PLAYER;
#else
		nextseat = (fseat+1)%GAME_PLAYER;
#endif
		auto itfind = std::find(m_VecChairMgr.begin(),m_VecChairMgr.end(),nextseat);
		if(itfind==m_VecChairMgr.end())
		{
			uint32_t findid = get_id_byseat(nextseat);
			auto it2 = std::find(m_VecPlaying.begin(),m_VecPlaying.end(),findid);
			if(it2!=m_VecPlaying.end()) break;
		}
		fseat = nextseat;
	} while (TRUE);

	nextid = get_id_byseat(nextseat);
	SLOG_CRITICAL<<"GetNextPlayerByID nextid:"<<nextid;
	return nextid;
}


bool bmw_space::logic_table::onGameEnd()
{
	set_status(eGameState_End);
	m_duration = m_fResultTime + TIME_LESS;
	// 正常结算
	uint32_t &nBanker = m_nBankerID;
	int64_t &nGold = m_nBankerGold;
	int32_t &nRound = m_nBankerRound;
	//	RandGameIndex();
	int64_t lBankerWinScore = CalculateScore();
	// 次数增加
	nRound++;
	// Why
	m_nBankerResult += lBankerWinScore;
	// 结算
	int64_t lAndroidWinScore = 0;
	for (auto &tableplayer : m_MapTablePlayer)
	{
		auto& user = tableplayer.second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		auto uid = user.p_id;
		auto& res = user.p_result;
		if(uid==nBanker)
		{
			int64_t nBankerTax = user.p_tax;
			player->write_property(res, nBankerTax);
			SLOG_CRITICAL << boost::format("[Save DB][Banker] %1%,result:%2%,tax:%3%") % uid%res%nBankerTax;
		}
		else
		{
			if (IsPlace(uid) == true)
			{
				if (player->is_robot())
				{
					lAndroidWinScore += res;
				}
				player->add_round();
				// tax
				int64_t nTax = user.p_tax;
				player->write_property(res, nTax);
				SLOG_CRITICAL << boost::format("[Save DB] %1%,result:%2%,tax:%3%") % uid%res%nTax;

			}
		}
		
	}
	// save 
	//WriteSystemScore(lAndroidWinScore);

	auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_game_result, bmw_protocols::e_mst_l2c_game_result);
	// banker info
	auto bankresinfo = sendmsg->mutable_bankres();
	bankresinfo->set_playerid(nBanker);
	bankresinfo->set_playerjoin(true);
	bankresinfo->set_player_result(lBankerWinScore);
	// win first
	for (auto tableplayer : m_MapTablePlayer)
	{
		auto user = tableplayer.second;
		auto uid = user.p_id;
		if(uid == nBanker) continue;
		if(user.p_result > 0)
		{
			m_MapWiner.insert(std::make_pair(uid,user.p_result));
		}
	}
	std::vector<BMWPAIR> vec_pair(m_MapWiner.begin(),m_MapWiner.end());
	std::sort(vec_pair.begin(),vec_pair.end(),[](const BMWPAIR& lhs, const BMWPAIR& rhs) {
		return lhs.second > rhs.second;
	});
	// top winer
	int32_t nTopCount = 0;
	for (auto winer : vec_pair)
	{
		auto resinfo = sendmsg->add_resultinfo();
		resinfo->set_playerid(winer.first);
		resinfo->set_playerjoin(true);
		resinfo->set_player_result(winer.second);
		nTopCount++;
		if(nTopCount==TOP_WINER)break;
	}
	// self info
	for (auto tableplayer : m_MapTablePlayer)
	{
		auto selfinfo = sendmsg->mutable_selfres();
		auto user = tableplayer.second;
		auto player = user.p_playerptr;
		auto res = user.p_result;
		auto uid = user.p_id;
		int64_t lAllPlace = 0;
		auto it =  m_MapAreaChip.find(uid);
		if(it != m_MapAreaChip.end())
		{
			tagAreaChip chip;
			chip = it->second;

			lAllPlace = chip.small_vw + chip.small_benz + chip.small_bmw + chip.small_porsche
				+ chip.big_vw + chip.big_benz + chip.big_bmw + chip.big_porsche;
		}
		if(player)
		{
			if(lAllPlace!=0)
			{
				selfinfo->set_playerjoin(true);	
			}
			else
			{
				selfinfo->set_playerjoin(false);	
			}
			selfinfo->set_playerid(uid);
			selfinfo->set_player_result(res);
			if(player->is_robot())
			{
				robot_mgr::instance().recv_packet(player->get_pid(),sendmsg->packet_id(),sendmsg);
			}
			else
			{
				player->send_msg_to_client(sendmsg);
			}
		}
	}

	// 发送历史
	auto msghistory = PACKET_CREATE(bmw_protocols::packetl2c_respone_history, bmw_protocols::e_mst_l2c_history);
	for(auto id : m_ListHistory)
	{
		msghistory->add_areaid(id);
	}
#if 0
	broadcast_msg_to_client2(msghistory);
#else
	broadcast_msg_to_client3(msghistory);
#endif

	return true;
}



void bmw_space::logic_table::CleanOutPlayer()
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
				auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_clean_out, bmw_protocols::e_mst_l2c_clean_out);
				sendmsg->set_reason((bmw_protocols::e_msg_cleanout_def)nReason);
				player->send_msg_to_client(sendmsg);

#if 1
				// clear table
				player->leave_table();
#endif
			}
		}
	}
}

void bmw_space::logic_table::CleanNoAction(uint32_t playerid)
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


bool bmw_space::logic_table::isNewRound()
{
	SLOG_CRITICAL<<boost::format("isNewRound cur:%1%,round:%2%")%m_nCurOperatorID%m_nGameRound;
	if(m_nFirstID==m_nCurOperatorID)
	{
		m_nGameRound++;
		return true;
	}
	return false;
}

bool bmw_space::logic_table::isVaildGold(int64_t lGold)
{
	for (std::vector<int>::iterator it=m_VecJetton.begin();it!=m_VecJetton.end();it++)
	{
		if (lGold==(*it))
		{
			return true;
		}
	}
	return false;
}


bool bmw_space::logic_table::isVaildArea(int32_t nArea)
{
	if(nArea>=ID_SMALL_VW  && nArea<=ID_BIG_Porsche)
		return true;
	return false;
}

bool bmw_space::logic_table::isApplyBank(uint32_t playerid)
{
	auto it = std::find(m_ApplyBanker.begin(),m_ApplyBanker.end(),playerid);
	if(it!=m_ApplyBanker.end())
		return true;
	return false;
}


//////////////////////////////////////////////////////////////////////////
void bmw_space::logic_table::robot_heartbeat(double elapsed)
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
		if(all_count==m_nMaxPlayerCount) return;
		// 只剩机器人了
		if(player_count==0 && robot_count>0)
		{

		}
		else if (all_count<m_nMaxPlayerCount && robot_count < requireCount)
		{
			// 添加上庄机器人


			// 有玩家才加入机器人
			if(player_count>0)
			{
				request_robot();
			}
		}
		else
		{
			// 金币不足
// 			for (auto user : m_MapTablePlayer)
// 			{
// 				LPlayerPtr& player = user.second.p_playerptr;
// 				uint32_t uid = user.second.p_id;
// 				if(!player) continue;
// 				if(!player->is_robot()) continue;
// 				// 游戏中不离开
// 				if(IsPlace(uid)) continue;
// 				if(get_status()>=eGameState_Place && uid==m_nBankerID) continue;
// 				if( player->get_gold()<m_room->get_data()->mGoldMinCondition)
// 				{
// 					release_robot(player->get_pid());
// 				}
// 			}
		}

		// 清理
// 		for (auto user : m_MapTablePlayer)
// 		{
// 			LPlayerPtr& player = user.second.p_playerptr;
// 			uint32_t uid = user.second.p_id;
// 			if(!player) continue;
// 			if(!player->is_robot()) continue;
// 			// 游戏中不离开
// 			if(IsPlace(uid)) continue;
// 			if(get_status()>=eGameState_Place && uid==m_nBankerID) continue;
// 			int32_t nRound = global_random::instance().rand_int(m_robotcfg.mRobotMinRound, m_robotcfg.mRobotMaxRound);
// 			if(player->get_round()>=nRound)
// 			{ // 超出限定局数
// 				release_robot(player->get_pid());
// 			}
// 			// 			else if(player->get_gold()>m_robotcfg.mRobotMaxGold)
// 			// 			{ // 超出携带数量
// 			// 				release_robot(player->get_pid());
// 			// 			}
// 			else
// 			{
// 
// 			}
// 		}
	}

}

void bmw_space::logic_table::banker_heartbeat(double elapsed)
{
	// ADD 2017-8-23 
	if (m_robotcfg.mIsOpen == 0) return;
	m_banker_elapsed -= elapsed;
	if (m_banker_elapsed <= 0)
	{
		m_banker_elapsed = global_random::instance().rand_int(m_robotcfg.mBankerMinEntry, m_robotcfg.mBankerMaxEntry);

		static int32_t nApplyCount = m_robotcfg.mBankerCount;
		// 系统上庄
		if(m_nBankerID ==0 )
		{
			if(m_ApplyBanker.size() <= nApplyCount)
			{
				// 请求一个机器人
				request_banker();
			}
		}
		else
		{
			if(m_ApplyBanker.size() <= nApplyCount-1)
			{
				// 请求一个机器人
				request_banker();
			}
		}

		// 机器人下庄
	}
}

void bmw_space::logic_table::request_robot()
{
	// Robot
	/*static*/ int minVIP = m_robotcfg.mRobotMinVip;
	/*static*/ int maxVIP = m_robotcfg.mRobotMaxVip;
	int32_t vip_level = global_random::instance().rand_int(minVIP, maxVIP);	
	GOLD_TYPE gold_min = m_robotcfg.mRobotMinTake;
	GOLD_TYPE gold_max = m_robotcfg.mRobotMaxTake;

	GOLD_TYPE enter_gold = global_random::instance().rand_int(gold_min,gold_max);
	int32_t rid = m_room->get_id();
	int32_t tid = get_id();
	int tag = (rid<<4)|tid;
	SLOG_CRITICAL<<boost::format("request_robot::rid:%1% tid:%2%,tag:%3%")%rid%tid%tag;
	game_engine::instance().request_robot(tag,enter_gold, vip_level);

}

void bmw_space::logic_table::request_banker()
{
	/*static*/ int minVIP = m_robotcfg.mRobotMinVip;
	/*static*/ int maxVIP = m_robotcfg.mRobotMaxVip;
	int32_t vip_level = global_random::instance().rand_int(minVIP, maxVIP);	
	int minRate = m_robotcfg.mBankerMinRate;
	int maxRate = m_robotcfg.mBankerMaxRate;
	GOLD_TYPE gold_min = m_roomcfg->mBankerGold*minRate;
	GOLD_TYPE gold_max = m_roomcfg->mBankerGold*maxRate;

	GOLD_TYPE enter_gold = global_random::instance().rand_int(gold_min,gold_max);
	int32_t rid = m_room->get_id();
	int32_t tid = get_id();
	int tag = (rid<<4)|tid;
	SLOG_CRITICAL<<boost::format("request_banker::rid:%1% tid:%2%,tag:%3%")%rid%tid%tag;
	game_engine::instance().request_robot(tag,enter_gold, vip_level);
}

void bmw_space::logic_table::release_robot(int32_t playerid)
{
	// 
	SLOG_CRITICAL<<boost::format("release_robot:playerid:%1%")%playerid;
	// setting robot leave
	auto it = m_MapTablePlayer.find(playerid);
	if(it == m_MapTablePlayer.end()) return;
	auto & user = it->second;
	user.p_leave = true;
	// clear
	for (auto applyit=m_ApplyBanker.begin();applyit!=m_ApplyBanker.end();applyit++)
	{
		if((*applyit) == playerid)
		{
			m_ApplyBanker.erase(applyit);
			break;
		}
	}
//	bc_leave_seat(playerid);
	m_room->get_lobby()->robot_leave(playerid);
	game_engine::instance().release_robot(playerid);
}

int32_t bmw_space::logic_table::release_robot_seat()
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

void bmw_space::logic_table::reverse_result(uint32_t reqid,uint32_t resid)
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



void bmw_space::logic_table::reverse_resultEx(uint32_t reqid,uint32_t resid)
{
	int32_t nRobotCount = robot_counter(reqid,resid);
	SLOG_CRITICAL<<boost::format("%1%:%2%,RobotCount:%3%")%__FUNCTION__%__LINE__%nRobotCount;
	if(nRobotCount==0)
	{
		// 玩家比牌
	}
	if(nRobotCount==1)
	{
		uint32_t playerid = 0,robotid = 0;
		tagPlayerCtrlAttr attr;
		uint32_t robot_req = 0,robot_res = 0;
		robot_req = robot_id(reqid);
		robot_res = robot_id(resid);
		if(robot_req==0 && robot_res==0) return;
		if(robot_req==0)
		{
			playerid = reqid;
			robotid = resid;
		}
		else
		{
			playerid = resid;
			robotid = reqid;
		}
		SLOG_CRITICAL<<boost::format("[Switch]Robot:%d,Player:%d")%robotid%playerid;
		auto it = m_MapTablePlayer.find(playerid);
		if(it == m_MapTablePlayer.end()) return ;
		auto user = it->second;
		auto player = user.p_playerptr;
		player->get_player_ctrl(attr,m_lTotalSliver,user.p_expend);
		double fRandom = global_random::instance().rand_01();
		double fWinPercent = attr.fwinPercent;
		SLOG_CRITICAL<<boost::format("[Switch]Robot:%d,Player:%d,PlayerWin:%f")%robotid%playerid%fWinPercent;
		SLOG_CRITICAL<<boost::format("Random:%f,playerid:%d,Tag:%d,Win:%f")%fRandom%playerid%attr.nTag%fWinPercent;
		if(fRandom <= fWinPercent)
		{ // 玩家赢

		}
		else
		{// 机器人赢
			robot_switch(robotid);
		}

	}
	else
	{
		// 机器人比牌
	}
}

void bmw_space::logic_table::reverse_result()
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


void bmw_space::logic_table::robot_switch(uint32_t uid,int nRandom/*=100*/)
{

}

int32_t bmw_space::logic_table::robot_counter()
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

int32_t bmw_space::logic_table::robot_counter(uint32_t reqid,uint32_t resid)
{
	int32_t nCount = 0;
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{ // 活着的机器人
		auto user = it->second;
		auto player = user.p_playerptr;
		if(user.p_playing==FALSE || player==nullptr) continue;
		auto uid = user.p_id;
		if(player->is_robot())
		{
			if(uid == reqid || uid==resid) nCount++;
		}
	}
	return nCount;
}

uint32_t bmw_space::logic_table::robot_id(uint32_t uid)
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

int32_t bmw_space::logic_table::robot_rate()
{
	int32_t nRate = 0;
	int32_t nLabel = GetMaxLabel();
	int32_t nRound = m_nGameRound;
	if(nLabel==Label_UserC)
	{
		nRate = 100;
	}
	else if(nLabel==Label_UserB)
	{
		nRate = (80-nRound*4);
	}
	else if(nLabel==Label_UserA)
	{
		nRate = (44-nRound*4);
	}
	else
	{
		nRate = 50;
	}

	return nRate;
}

int32_t bmw_space::logic_table::GetLabelByValue(int64_t nValue)
{


	return Label_UserC;	
}

void bmw_space::logic_table::CalcUserLabel()
{
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto & user = it->second;
		if(user.p_playing==FALSE) continue;
		uint32_t uid = user.p_id;
		auto& player = user.p_playerptr;
		if(player)
		{
			int32_t nTag= player->get_player_tag();
			if(nTag==Tag_Supper)
			{
				user.p_label = Label_Supper;
			}
			else if(nTag==Tag_Robot)
			{
				user.p_label = Label_Robot;
			}
			else
			{
				int64_t nLabelValue = nTag + m_lTotalSliver - user.p_expend;
				user.p_label = GetLabelByValue(nLabelValue);
			}
		}
	}
}

int32_t bmw_space::logic_table::GetMaxLabel()
{
	CalcUserLabel();
	for(auto user: m_MapTablePlayer)
	{
		if(user.second.p_label == Label_UserC)
			return Label_UserC;
	}
	for(auto user: m_MapTablePlayer)
	{
		if(user.second.p_label == Label_UserB)
			return Label_UserB;
	}
	for(auto user: m_MapTablePlayer)
	{
		if(user.second.p_label == Label_UserA)
			return Label_UserA;
	}

	return Label_Robot;
}


void bmw_space::logic_table::ReadLotteryRate()
{
	const boost::unordered_map<int, BMW_LotteryCFGData>& list = BMW_LotteryCFG::GetSingleton()->GetMapData();
	for (auto it = list.begin(); it != list.end(); ++it)
	{
		if(it->second.mRoomID == m_room->get_id())
		{
			m_lotterycfg = it->second;
		}
	}

}

void bmw_space::logic_table::DisbandPlayer()
{
	SLOG_CRITICAL<<"------------DissBand All------------";
	m_duration = 0.0;
	set_status(eGameState_Free);
	// 游戏数据初始化
	m_nBankerID = 0;

	m_nCurOperatorID = 0;
	m_nLastWiner = 0;
	m_lCurJetton = 0;

	m_lTotalSliver = 0;
	m_lInventPool = 0;

	m_nFirstID = 0;
	m_MapCompareNexus.clear();
	m_VecActive.clear();
	m_VecPlaying.clear();

	m_nGameRound = 0;

	m_nEndType = eGameEndType_null;

	m_nShowHandCnt = 0;
	m_VecAllin.clear();

	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		auto uid = user.p_id;

		if(player->is_robot())
		{
			release_robot(uid);
		}
		else
		{
			auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_clean_out, bmw_protocols::e_mst_l2c_clean_out);
			sendmsg->set_reason((bmw_protocols::e_msg_cleanout_def)1);
			player->send_msg_to_client(sendmsg);
		}

	}

}

bool bmw_space::logic_table::onPlayerRunaway(uint32_t playerid)
{
	SLOG_CRITICAL<<boost::format("%1%:%2%,playerid:%3%")%__FUNCTION__%__LINE__%playerid;

	// playing
	auto it = std::find(m_VecPlaying.begin(),m_VecPlaying.end(),playerid);
	if(it != m_VecPlaying.end())
	{
		m_VecPlaying.erase(it);
	}
	// check game over
	int32_t nCount = m_VecPlaying.size();
	if(nCount>=PLAY_MIN_COUNT)
	{
		// 游戏继续
	}
	else if(nCount==1)
	{
		// 游戏结束

	}
	else
	{
		// 桌子没人
		repositGame();
	}
	return true;
}

int64_t bmw_space::logic_table::CountPlayerAllChip(uint32_t playerid)
{
	int64_t nCount = 0;
	auto it = m_MapAreaChip.find(playerid);
	if(it!=m_MapAreaChip.end())
	{
		auto chip = it->second;
		nCount = chip.big_benz + chip.small_benz +
			chip.big_vw + chip.small_vw + 
			chip.big_bmw + chip.small_bmw +
			chip.big_porsche + chip.small_porsche ;
	}
	return nCount;
}

// 计算玩家当前最大筹码值
int64_t bmw_space::logic_table::CountPlayerMaxChip(uint32_t playerid)
{
	// 已下注筹码值
	int64_t lNowChip = CountPlayerAllChip(playerid);

	// 玩家金币
	int64_t lUserGold = 0;
	auto it = m_MapTablePlayer.find(playerid);
	if(it!=m_MapTablePlayer.end())
	{
		auto player = it->second.p_playerptr;
		if(player)
			lUserGold = player->get_gold();
	}
	int64_t lMaxChip = 0;
	// 获取设置限定值
	int64_t lPlayerLimit = m_roomcfg->mPlayerLimit;
	lMaxChip = std::min(lUserGold,lPlayerLimit);
	lMaxChip -= lNowChip;
	lMaxChip = std::max(lMaxChip,(int64_t)0);
	return lMaxChip;
}


int64_t bmw_space::logic_table::CountAreaAllChip(int32_t nArea)
{
	int64_t nCount = 0;
	if(isVaildArea(nArea)==false) return 0;
	for (auto &tableplayer : m_MapTablePlayer)
	{
		int64_t lUserScore = 0;
		int64_t lReturnScore = 0;
		auto& user = tableplayer.second;
		auto uid = user.p_id;
		int64_t lScore[9] = {0};
		auto it =  m_MapAreaChip.find(uid);
		if(it != m_MapAreaChip.end())
		{
			tagAreaChip chip;
			chip = it->second;
			lScore[1] = chip.small_vw;
			lScore[2] = chip.small_benz;
			lScore[3] = chip.small_bmw;
			lScore[4] = chip.small_porsche;
			lScore[5] = chip.big_vw;
			lScore[6] = chip.big_benz;
			lScore[7] = chip.big_bmw;
			lScore[8] = chip.big_porsche;

			nCount += lScore[nArea];
		}
	}

	return nCount;
}

int64_t bmw_space::logic_table::CountBigBenzChip(uint32_t playerid,int64_t lJetton)
{
	// 玩家限制
	int64_t lUserMaxJetton = CountPlayerMaxChip(playerid);
	// 区域限制
	int64_t lAreaLimit = m_roomcfg->mAreaLimit - m_lAllBigBenz;
	// 最大下注
	int64_t lMaxJetton = 0;
	// 庄家判断
	if(m_nBankerID!=0)
	{
		// 其他区域
		int64_t lOtherAreaGold = m_lAllBigBMW+m_lAllBigPorsche+m_lAllBigVW+
			m_lAllSmallBenz+m_lAllSmallBMW+m_lAllSmallPorsche+m_lAllSmallVW;
		// 庄限制
		int64_t lMaxPlayerGold = m_nBankerGold + lOtherAreaGold;
		lMaxPlayerGold -= (m_lAllBigBenz+lJetton)*(BIG_Benz-1);
		// 最大下注
		lMaxJetton = std::min(lMaxPlayerGold,lUserMaxJetton);
	}
	else
	{ // 系统做庄
		lMaxJetton = std::min(lUserMaxJetton,lAreaLimit);
	}
	return lMaxJetton;
}

int64_t bmw_space::logic_table::CountSmallBenzChip(uint32_t playerid,int64_t lJetton)
{
	// 玩家限制
	int64_t lUserMaxJetton = CountPlayerMaxChip(playerid);
	// 区域限制
	int64_t lAreaLimit = m_roomcfg->mAreaLimit - m_lAllSmallBenz;
	// 最大下注
	int64_t lMaxJetton = 0;
	// 庄家判断
	if(m_nBankerID!=0)
	{
		// 其他区域
		int64_t lOtherAreaGold = m_lAllBigBenz+m_lAllBigBMW+m_lAllBigPorsche+m_lAllBigVW+
			m_lAllSmallBMW+m_lAllSmallPorsche+m_lAllSmallVW;
		// 庄限制
		int64_t lMaxPlayerGold = m_nBankerGold + lOtherAreaGold;
		lMaxPlayerGold -= (m_lAllSmallBenz+lJetton)*(SMALL_Benz-1);
		// 最大下注
		lMaxJetton = std::min(lMaxPlayerGold,lUserMaxJetton);
	}
	else
	{ // 系统做庄
		lMaxJetton = std::min(lUserMaxJetton,lAreaLimit);
	}
	return lMaxJetton;
}

int64_t bmw_space::logic_table::CountBigBMWChip(uint32_t playerid,int64_t lJetton)
{
	// 玩家限制
	int64_t lUserMaxJetton = CountPlayerMaxChip(playerid);
	// 区域限制
	int64_t lAreaLimit = m_roomcfg->mAreaLimit - m_lAllBigBMW;
	// 最大下注
	int64_t lMaxJetton = 0;
	// 庄家判断
	if(m_nBankerID!=0)
	{
		// 其他区域
		int64_t lOtherAreaGold = m_lAllBigBenz+m_lAllBigPorsche+m_lAllBigVW+
			m_lAllSmallBenz+m_lAllSmallBMW+m_lAllSmallPorsche+m_lAllSmallVW;
		// 庄限制
		int64_t lMaxPlayerGold = m_nBankerGold + lOtherAreaGold;
		lMaxPlayerGold -= (m_lAllBigBMW+lJetton)*(BIG_BMW-1);
		// 最大下注
		lMaxJetton = std::min(lMaxPlayerGold,lUserMaxJetton);
	}
	else
	{ // 系统做庄
		lMaxJetton = std::min(lUserMaxJetton,lAreaLimit);
	}
	return lMaxJetton;
}

int64_t bmw_space::logic_table::CountSmallBMWChip(uint32_t playerid,int64_t lJetton)
{
	// 玩家限制
	int64_t lUserMaxJetton = CountPlayerMaxChip(playerid);
	// 区域限制
	int64_t lAreaLimit = m_roomcfg->mAreaLimit - m_lAllSmallBMW;
	// 最大下注
	int64_t lMaxJetton = 0;
	// 庄家判断
	if(m_nBankerID!=0)
	{
		// 其他区域
		int64_t lOtherAreaGold = m_lAllBigBenz+m_lAllBigBMW+m_lAllBigPorsche+m_lAllBigVW+
			m_lAllSmallBenz+m_lAllSmallPorsche+m_lAllSmallVW;
		// 庄限制
		int64_t lMaxPlayerGold = m_nBankerGold + lOtherAreaGold;
		lMaxPlayerGold -= (m_lAllSmallBMW+lJetton)*(SMALL_BMW-1);
		// 最大下注
		lMaxJetton = std::min(lMaxPlayerGold,lUserMaxJetton);
	}
	else
	{ // 系统做庄
		lMaxJetton = std::min(lUserMaxJetton,lAreaLimit);
	}
	return lMaxJetton;
}

int64_t bmw_space::logic_table::CountBigVWChip(uint32_t playerid,int64_t lJetton)
{
	// 玩家限制
	int64_t lUserMaxJetton = CountPlayerMaxChip(playerid);
	// 区域限制
	int64_t lAreaLimit = m_roomcfg->mAreaLimit - m_lAllBigVW;
	// 最大下注
	int64_t lMaxJetton = 0;
	// 庄家判断
	if(m_nBankerID!=0)
	{
		// 其他区域
		int64_t lOtherAreaGold = m_lAllBigBenz+m_lAllBigBMW+m_lAllBigPorsche+
			m_lAllSmallBenz+m_lAllSmallBMW+m_lAllSmallPorsche+m_lAllSmallVW;
		// 庄限制
		int64_t lMaxPlayerGold = m_nBankerGold + lOtherAreaGold;
		lMaxPlayerGold -= (m_lAllBigVW+lJetton)*(BIG_VW-1);
		// 最大下注
		lMaxJetton = std::min(lMaxPlayerGold,lUserMaxJetton);
	}
	else
	{ // 系统做庄
		lMaxJetton = std::min(lUserMaxJetton,lAreaLimit);
	}
	return lMaxJetton;
}

int64_t bmw_space::logic_table::CountSmallVWChip(uint32_t playerid,int64_t lJetton)
{
	// 玩家限制
	int64_t lUserMaxJetton = CountPlayerMaxChip(playerid);
	// 区域限制
	int64_t lAreaLimit = m_roomcfg->mAreaLimit - m_lAllSmallVW;
	// 最大下注
	int64_t lMaxJetton = 0;
	// 庄家判断
	if(m_nBankerID!=0)
	{
		// 其他区域
		int64_t lOtherAreaGold = m_lAllBigBenz+m_lAllBigBMW+m_lAllBigPorsche+m_lAllBigVW+
			m_lAllSmallBenz+m_lAllSmallPorsche+m_lAllSmallBMW;
		// 庄限制
		int64_t lMaxPlayerGold = m_nBankerGold + lOtherAreaGold;
		lMaxPlayerGold -= (m_lAllSmallVW+lJetton)*(SMALL_VW-1);
		// 最大下注
		lMaxJetton = std::min(lMaxPlayerGold,lUserMaxJetton);
	}
	else
	{ // 系统做庄
		lMaxJetton = std::min(lUserMaxJetton,lAreaLimit);
	}
	return lMaxJetton;
}

int64_t bmw_space::logic_table::CountBigPorscheChip(uint32_t playerid,int64_t lJetton)
{
	// 玩家限制
	int64_t lUserMaxJetton = CountPlayerMaxChip(playerid);
	// 区域限制
	int64_t lAreaLimit = m_roomcfg->mAreaLimit - m_lAllBigPorsche;
	// 最大下注
	int64_t lMaxJetton = 0;
	// 庄家判断
	if(m_nBankerID!=0)
	{
		// 其他区域
		int64_t lOtherAreaGold = m_lAllBigBenz+m_lAllBigBMW+m_lAllBigVW+
			m_lAllSmallBenz+m_lAllSmallBMW+m_lAllSmallPorsche+m_lAllSmallVW;
		// 庄限制
		int64_t lMaxPlayerGold = m_nBankerGold + lOtherAreaGold;
		lMaxPlayerGold -= (m_lAllBigPorsche+lJetton)*(BIG_Porsche-1);
		// 最大下注
		lMaxJetton = std::min(lMaxPlayerGold,lUserMaxJetton);
	}
	else
	{ // 系统做庄
		lMaxJetton = std::min(lUserMaxJetton,lAreaLimit);
	}
	return lMaxJetton;
}

int64_t bmw_space::logic_table::CountSmallPorscheChip(uint32_t playerid,int64_t lJetton)
{
	// 玩家限制
	int64_t lUserMaxJetton = CountPlayerMaxChip(playerid);
	// 区域限制
	int64_t lAreaLimit = m_roomcfg->mAreaLimit - m_lAllSmallPorsche;
	// 最大下注
	int64_t lMaxJetton = 0;
	// 庄家判断
	if(m_nBankerID!=0)
	{
		// 其他区域
		int64_t lOtherAreaGold = m_lAllBigBenz+m_lAllBigBMW+m_lAllBigPorsche+m_lAllBigVW+
			m_lAllSmallBenz+m_lAllSmallVW+m_lAllSmallBMW;
		// 庄限制
		int64_t lMaxPlayerGold = m_nBankerGold + lOtherAreaGold;
		lMaxPlayerGold -= (m_lAllSmallPorsche+lJetton)*(SMALL_Porsche-1);
		// 最大下注
		lMaxJetton = std::min(lMaxPlayerGold,lUserMaxJetton);
	}
	else
	{ // 系统做庄
		lMaxJetton = std::min(lUserMaxJetton,lAreaLimit);
	}
	return lMaxJetton;
}


bool bmw_space::logic_table::ChangeBanker(bool bCancelCurrentBanker)
{
	//切换标识
	bool bChangeBanker=false;
	int32_t nBankRound = m_roomcfg->mBankerRound;
	//
	uint32_t &nBanker = m_nBankerID;
	int64_t &nGold = m_nBankerGold;
	int32_t &nRound = m_nBankerRound;
	int64_t &nResult = m_nBankerResult;
	// 取消当前
	if(bCancelCurrentBanker)
	{
		// 默认系统
		nBanker = 0;
		nGold = m_roomcfg->mBankerGold;
		nRound = 0;
		//轮换判断
		if(get_status()==eGameState_Free && m_ApplyBanker.size()>0)
		{
			uint32_t id  = m_ApplyBanker.front();
			m_ApplyBanker.pop_front();
			auto it = m_MapTablePlayer.find(id);
			if(it!=m_MapTablePlayer.end())
			{
				auto player = it->second.p_playerptr;
				if(player)
				{
					nBanker = id;
					nGold = player->get_gold();
					nRound = 0;
					nResult = 0;
				}
			}
		}
		//设置变量
		bChangeBanker=true;
	}
	//轮庄判断
	else if(nBanker!=0)
	{
		// 次数判断
		if(nRound<nBankRound)
		{
			bChangeBanker = false;
		}
		else
		{
			bChangeBanker = true;
		}
		// 金币判断
		if(nGold>=m_roomcfg->mBankerGold)
		{
			bChangeBanker = false;
		}
		else
		{
			bChangeBanker = true;
		}
		if(bChangeBanker)
		{
			if(m_ApplyBanker.size()>0)
			{ // 玩家做庄
				uint32_t id  = m_ApplyBanker.front();
				m_ApplyBanker.pop_front();
				auto it = m_MapTablePlayer.find(id);
				if(it!=m_MapTablePlayer.end())
				{
					auto player = it->second.p_playerptr;
					if(player)
					{
						nBanker = id;
						nGold = player->get_gold();
						nRound = 0;
					}
				}
			}
			else
			{ // 系统做庄
				nBanker = 0;
				nGold = m_roomcfg->mBankerGold;
				nRound = 0;
			}
		}
	}
	// 系统做庄
	else if(nBanker==0)
	{
		if(m_ApplyBanker.size()>0)
		{ // 玩家做庄
			uint32_t id  = m_ApplyBanker.front();
			m_ApplyBanker.pop_front();
			auto it = m_MapTablePlayer.find(id);
			if(it!=m_MapTablePlayer.end())
			{
				auto player = it->second.p_playerptr;
				if(player)
				{
					nBanker = id;
					nGold = player->get_gold();
					nRound = 0;
				}
			}
		}
		bChangeBanker = true;
	}

	//切换判断
	if (bChangeBanker)
	{
		onNoticeChangeBanker();
	}
	return bChangeBanker;
}

bool bmw_space::logic_table::IsContinueBanker(int64_t lBankerScore)
{
	// 连续做庄
	if(lBankerScore >= m_roomcfg->mBankerGold)
	{
		return true;
	}
	return false;
}

void bmw_space::logic_table::RandGameIndex()
{
#if 1
	RandomIndexByStock();
	// 将结果转换成 [1,32]
	int32_t& nAreaid = m_cbResultIndex;
	int nParam = global_random::instance().rand_int(0, 3);
	nAreaid = nAreaid + nParam * PLACE_AREA;
	SLOG_WARNING << boost::format("HHHHHHHHHHHH:%d") % nAreaid;
#else
	m_cbResultIndex = RandGameResult();
#endif
	// 记录信息
	int32_t nHistory = m_cbResultIndex;
	SLOG_CRITICAL<<boost::format("[HHHH]:%d")%nHistory;
	if(m_ListHistory.size() < m_roomcfg->mRecordHistory)
	{
		m_ListHistory.push_back(nHistory);
	}
	else
	{
		m_ListHistory.pop_front();
		m_ListHistory.push_back(nHistory);
	}
}

void bmw_space::logic_table::RandomIndexByStock()
{
	int32_t cbIndex[4][8] = {0};
	int32_t cbType = 0;
	int32_t nIndexCount[4] = {0};
	int32_t nRate[PLACE_AREA+1] = {0,24,24,24,24,12,6,4,3};
	//获取随机概率
	GetRandRate(nRate);

	int64_t lAndroidScore[9] = { 0 };
	int nRand = global_random::instance().rand_100();
	//循环获取每个索引的摘要
	for(int i=ID_SMALL_VW;i<=ID_BIG_Porsche;i++)
	{
		cbType = GetIndexSummary(i, nRand, lAndroidScore[i]);
		int32_t nIdx = nIndexCount[cbType]++;
		cbIndex[cbType][nIdx] = i;
	}

	m_cbResultIndex = 0;
	for(int i=0;i<1/*4*/;i++)
	{
		if(nIndexCount[i] > 0)
		{
			int nTotalRate = 0;
			for(int j=0;j<nIndexCount[i];j++)
			{
				nTotalRate += nRate[cbIndex[i][j]];
			}

			if(nTotalRate == 0)
			{
				int cbRand = 0;
				if(nIndexCount[i]-1 > 0)
				{
					cbRand = global_random::instance().rand_int(0,nIndexCount[i]-1);
				}
				m_cbResultIndex = cbIndex[i][cbRand];
				assert(m_cbResultIndex>=ID_SMALL_VW && m_cbResultIndex<=ID_BIG_Porsche);
				return;
			}
			else
			{
				int cbRand = global_random::instance().rand_int(0,nTotalRate-1);
				for(int j=0;j<nIndexCount[i];j++)
				{
					if(cbRand < nRate[cbIndex[i][j]])
					{
						m_cbResultIndex = cbIndex[i][j];
						assert(m_cbResultIndex>=ID_SMALL_VW && m_cbResultIndex<=ID_BIG_Porsche);
						return;
					}
					cbRand -= nRate[cbIndex[i][j]];
				}	
			}
		}
	}
	// 如果万一机器人都是输的,开奖机器人收获最大的结果
#if 1
	std::vector<int32_t> VecIdx;
	for (int i = ID_SMALL_VW ; i <= ID_BIG_Porsche; i++)
	{
		if ( lAndroidScore[i] > 0)
		{
			VecIdx.push_back(i);
		}
	}
	int nCount = VecIdx.size();
	if (nCount > 0)
	{
		std::mt19937_64 g(std::time(0));
		std::uniform_int_distribution<> dis(1, 10000);
		m_cbResultIndex = VecIdx.at(dis(g) % nCount);
	}
	else
	{ // 如果都是输找一个最大的吧
		int64_t lAndroidMaxScore = lAndroidScore[ID_SMALL_VW];
		m_cbResultIndex = ID_SMALL_VW;
		for (int i = ID_SMALL_VW + 1; i <= ID_BIG_Porsche; i++)
		{
			if (lAndroidMaxScore < lAndroidScore[i])
			{
				lAndroidMaxScore = lAndroidScore[i];
			}
		}
		// 没有玩家下注，随机出一个
		if (lAndroidMaxScore == 0)
		{
			std::random_device rd;
			std::mt19937_64 g(rd());
			std::uniform_int_distribution<> dis(1, 10000);
			int32_t nResIndex = dis(g) % PLACE_AREA + 1;
			SLOG_CRITICAL << boost::format("No-Anyone to place ,rand:%1%") % nResIndex;
			m_cbResultIndex = nResIndex;
		}
		else
		{
			// 如果存在相同的情况
			std::vector<int32_t> VecIdx;
			for (int32_t i = ID_SMALL_VW; i <= ID_BIG_Porsche; i++)
			{
				if (lAndroidMaxScore == lAndroidScore[i])
				{
					VecIdx.push_back(i);
				}
			}
			int nCount = VecIdx.size();
			if (nCount > 0)
			{
				std::mt19937_64 g(std::time(0));
				std::uniform_int_distribution<> dis(1, 10000);
				m_cbResultIndex = VecIdx.at(dis(g) % nCount);
			}
		}


	}
#else
	int64_t lAndroidMaxScore = lAndroidScore[ID_SMALL_VW];
	m_cbResultIndex = ID_SMALL_VW;
	for(int i=ID_SMALL_VW+1;i<=ID_BIG_Porsche;i++)
	{
		if(lAndroidMaxScore < lAndroidScore[i])
		{
			lAndroidMaxScore = lAndroidScore[i];
		}
	}
	// 没有玩家下注，随机出一个
	if (lAndroidMaxScore == 0)
	{
		std::random_device rd;
		std::mt19937_64 g(rd());
		std::uniform_int_distribution<> dis(1, 10000);
		int32_t nResIndex = dis(g) % PLACE_AREA + 1;
		SLOG_CRITICAL << boost::format("No-Anyone to place ,rand:%1%") % nResIndex;
		m_cbResultIndex = nResIndex;
	}
	else
	{
		// 如果存在相同的情况
		std::vector<int32_t> VecIdx;
		for (int32_t i = ID_SMALL_VW; i <= ID_BIG_Porsche; i++)
		{
			if (lAndroidMaxScore == lAndroidScore[i])
			{
				VecIdx.push_back(i);
			}
		}
		int nCount = VecIdx.size();
		if (nCount > 0)
		{
			std::mt19937_64 g(std::time(0));
			std::uniform_int_distribution<> dis(1, 10000);
			m_cbResultIndex = VecIdx.at(dis(g) % nCount);
		}
	}
#endif
	//
	assert(m_cbResultIndex >= ID_SMALL_VW && m_cbResultIndex <= ID_BIG_Porsche);

}

void bmw_space::logic_table::GetRandRate(int nRate[])
{
	int nInitRate[] = {0,24,24,24,24,12,6,4,3};	
	int nBigUpRate = m_lotterycfg.mBigRate;
	int dwRandTypeMinute = m_lotterycfg.mRefreshMinute;
	int nRandType = m_lotterycfg.mRandType;
	int64_t nSeconds = GetSecondsCount();
	if(nSeconds - m_dwLastRandTick >= dwRandTypeMinute*60)
	{
		nRandType = global_random::instance().rand_int(0,2);
		m_dwLastRandTick = nSeconds;
	}
	/*
	0  固定概率
	1  可变概率
	2  大概率
	3  机器人必胜
	*/
	//固定概率
	if(nRandType == 0)
	{
		for(int i=0;i<=8;i++)
		{
			nRate[i] = nInitRate[i];
		}
	}
	else
	{
		//随机概率（上下浮动100%）
		nRate[0] = 0;
		for(int i=1;i<= PLACE_AREA;i++)
		{
			nRate[i] = global_random::instance().rand_int(0,2*nInitRate[i]);
		}
		//大概率上浮50%
		if(nRandType == 2)
		{
			for(int i=1;i<=PLACE_AREA;i++)
			{
				if(i>=ID_BIG_VW)
				{
					nRate[i] += (global_random::instance().rand_int(0,nRate[i]))*nBigUpRate/100;
				}
			}
		}
	}

}

int32_t bmw_space::logic_table::GetIndexSummary(int32_t nIndex, int32_t nRand, int64_t& lAndroidScore)
{
	int32_t cbType = 0;
	int64_t lUserScore,lSystemScore = 0;
	bool bIsSystemBanker = false;//机器人坐庄
	//区域倍率
	int32_t cbMultiple[] = {0,5,5,5,5,10,20,30,40};
	//是系统做庄.
	uint32_t &nBanker = m_nBankerID;
	if(nBanker==0)
	{
		bIsSystemBanker = true;
	}
	else
	{
		auto it = m_MapTablePlayer.find(nBanker);
		if(it!=m_MapTablePlayer.end())
		{
			auto player = it->second.p_playerptr;
			if(player && player->is_robot())
			{
				bIsSystemBanker = true;
			}
		}
	}
	//获取
#if 0
	int64_t lAndroidWinScore = BMW_ResultCtrl::GetSingleton()->GetData("AndroidWinScore")->mValue;
#else
	int64_t lAndroidWinScore = m_lAndroidWinScore;
#endif
	int64_t lAndroidSafeScore = BMW_ResultCtrl::GetSingleton()->GetData("AndroidSafeScore")->mValue;
	// Killer-Ctrl
	int64_t lAllAndroidWinScore = m_lAndroidWinScore;
	int killopt = get_room()->getkiller();
	int cutround = get_room()->getCutRound();
	if (killopt == 10)
	{ // start
		if ( cutround > 0 && cutround <= 10000)
		{
			std::random_device rd;
			std::mt19937_64 g(rd());
			std::uniform_int_distribution<> dis(1, 10000);
			if (dis(g) <= cutround)
			{
				lAllAndroidWinScore = 0;
			}
		}
	}
	else if(killopt == 11)
	{ // stop
		
	}
	//玩家成绩
	for (auto tableplayer : m_MapTablePlayer)
	{
		auto user = tableplayer.second;
		auto uid = user.p_id;
		if (uid == nBanker) continue;
		int64_t lScore[9] = {0};
		auto it =  m_MapAreaChip.find(uid);
		if(it != m_MapAreaChip.end())
		{
			tagAreaChip chip;
			chip = it->second;
			lScore[1] = chip.small_vw;
			lScore[2] = chip.small_benz;
			lScore[3] = chip.small_bmw;
			lScore[4] = chip.small_porsche;
			lScore[5] = chip.big_vw;
			lScore[6] = chip.big_benz;
			lScore[7] = chip.big_bmw;
			lScore[8] = chip.big_porsche;
		}
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		for (int32_t i = ID_SMALL_VW;i<=ID_BIG_Porsche;i++)
		{
			if(nIndex == i)
			{
				lUserScore = lScore[i] * (cbMultiple[i]-1);
			}
			else
			{
				lUserScore = -lScore[i];
			}
			//记录机器人输赢
			if(player->is_robot())
			{
				lSystemScore += lUserScore;
			}
			if(bIsSystemBanker)
			{
				lSystemScore -= lUserScore;
			}
		}
	}
	//机器人不能输
	if (lSystemScore + lAllAndroidWinScore/*lAndroidWinScore + m_lAITax*/ < lAndroidSafeScore)
	{
		cbType = 3;
	}

	lAndroidScore = lSystemScore;

	return cbType;
}

int64_t bmw_space::logic_table::CalculateScore()
{
	//庄家总量
	int64_t lBankerWinScore = 0;
	//下注机器人
	int64_t lAndroidWinScore = 0;
	//区域倍率
	int32_t cbMultiple[] = {0, 5, 5, 5, 5, 10, 20, 30, 40};
	//玩家成绩
	uint32_t &nBanker = m_nBankerID;
	for (auto &tableplayer : m_MapTablePlayer)
	{
		int64_t lUserScore = 0;
		int64_t lReturnScore = 0;
		int32_t nTax = 0;
		auto& user = tableplayer.second;
		auto uid = user.p_id;
		if (uid == nBanker) continue;
		int64_t lScore[9] = {0};
		auto it =  m_MapAreaChip.find(uid);
		if(it != m_MapAreaChip.end())
		{
			tagAreaChip chip;
			chip = it->second;
			lScore[1] = chip.small_vw;
			lScore[2] = chip.small_benz;
			lScore[3] = chip.small_bmw;
			lScore[4] = chip.small_porsche;
			lScore[5] = chip.big_vw;
			lScore[6] = chip.big_benz;
			lScore[7] = chip.big_bmw;
			lScore[8] = chip.big_porsche;
		}
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		for (int32_t i = ID_SMALL_VW;i<=ID_BIG_Porsche;i++)
		{
			if(lScore[i]==0) continue;
			if((m_cbResultIndex%PLACE_AREA) == (i%PLACE_AREA))
			{
				lUserScore += lScore[i] * (cbMultiple[i]);
				lReturnScore += lScore[i];
				lBankerWinScore -= lScore[i] * (cbMultiple[i]);

				// Add 区域税收
// 				if (lUserScore > 0)
// 				{
// 					// tax
// 					nTax = std::ceil(lUserScore * m_nRate / 100.0);
// 					lUserScore -= nTax;
// 				}
			}
			else
			{
				lUserScore -= lScore[i];
			}
			lBankerWinScore += lScore[i];
		}
		lUserScore -= lReturnScore;
		// 记录下注机器人
		if (player->is_robot())
		{
			lAndroidWinScore += lUserScore;
		}
		// tax
		if (lUserScore > 0)
		{
			nTax = std::ceil(lUserScore * m_nRate / 100.0);
			lUserScore -= nTax;
		}
		user.p_result = lUserScore;
		user.p_tax = nTax;
		// 统计税收
		m_lAllTax += nTax;
		if (player->is_robot())
		{
			m_lAITax += nTax;
		}
	}
	// -系统
	WriteSystemScore(lAndroidWinScore);
	// 庄家成绩
	lBankerWinScore += m_lFleeScore;
	if(nBanker==0)
	{ // 系统庄
		WriteSystemScore(lBankerWinScore);
		SLOG_CRITICAL<<boost::format("[Result] System Banker :%d")%lBankerWinScore;
	}
	else
	{ // 玩家 包含机器人
		auto it = m_MapTablePlayer.find(nBanker);
		if(it!=m_MapTablePlayer.end())
		{
			auto& user = it->second;
			auto player = user.p_playerptr;
			int64_t nBankerTax = 0;
			// 记录系统输赢
			if (player && player->is_robot())
			{
				WriteSystemScore(lBankerWinScore);
				SLOG_CRITICAL << boost::format("[Result] Robot Banker :%d") % lBankerWinScore;
			}
			// 计算庄的税收
			if (lBankerWinScore > 0)
			{
				nBankerTax = std::ceil(lBankerWinScore * m_nRate / 100.0);
				lBankerWinScore -= nBankerTax;
			}
			user.p_result = lBankerWinScore;
			user.p_tax = nBankerTax;
			// 记录税收
			m_lAllTax += nBankerTax;
			if (player && player->is_robot())
			{
				m_lAITax += nBankerTax;
// 				WriteSystemScore(lBankerWinScore);
// 				SLOG_CRITICAL << boost::format("[Result] Robot Banker :%d") % lBankerWinScore;
			}
		}
	}
	return lBankerWinScore;
}

bool bmw_space::logic_table::CheckApply()
{
	for (auto applyit = m_ApplyBanker.begin(); applyit != m_ApplyBanker.end(); applyit++)
	{
		auto id = *applyit;
		auto player = get_player_byid(*applyit);
		if(player && player->get_gold()<m_roomcfg->mBankerGold)
		{
			m_ApplyBanker.erase(applyit);
			onSendApplyBankerResult(id, 2);
			break;
		}
	}
	return true;
}

bool bmw_space::logic_table::CheckNextBanker()
{
	//切换标识
	bool bChangeBanker=false;
	int32_t nBankRound = m_roomcfg->mBankerRound;
	//
	uint32_t &nBanker = m_nBankerID;
	int64_t &nGold = m_nBankerGold;
	int32_t &nRound = m_nBankerRound;
	int64_t &nResult = m_nBankerResult;
	// 先判断已经有下庄请求的玩家
	for(auto unid : m_unApply)
	{
		if(unid == nBanker)
		{
			onSendUnApplyBankerResult(unid,1);
			// 下庄成功--默认系统上庄
			nBanker = 0;
			nGold = m_roomcfg->mBankerGold;
			nRound = 0;
			nResult = 0;
			break;
		}
	}
	// clear
	m_unApply.clear();
	//轮庄判断
	if(nBanker!=0)
	{
		// 庄数据
		auto player = get_player_byid(nBanker);
		if(player)
		{
			nGold = player->get_gold();
		}
		else
		{
			SLOG_ERROR << boost::format("find banker:%1% err")%nBanker;
		//	assert(0);
		}
		// robot
		if(player && player->is_robot())
		{
			int32_t nbankerround = global_random::instance().rand_int(BANKER_MIN_ROUND, nBankRound);
			if(nRound >= nbankerround)
			{
				bChangeBanker = true;
			}
			else if(nGold < m_roomcfg->mBankerGold)
			{
				bChangeBanker = true;
			}
			else
			{
				bChangeBanker = false;
			}
			
			// ADD 2017-8-29 
// 			if(bChangeBanker)
// 			{
// 				release_robot(nBanker);
// 			}
		}
		else
		{
			// 次数判断
			if(nRound >= nBankRound)
			{
				bChangeBanker = true;
			}
			else if(nGold < m_roomcfg->mBankerGold)
			{
				bChangeBanker = true;
			}
			else
			{
				bChangeBanker = false;
			}

		}
		//
		if (player == nullptr)
		{
			bChangeBanker = true;
		}
		if(bChangeBanker)
		{
			// 释放机器人
			if (player != nullptr && player->is_robot())
			{
				release_robot(nBanker);
			}
			// 更换新的庄
			if(m_ApplyBanker.size()>0)
			{ // 玩家做庄
				uint32_t id  = m_ApplyBanker.front();
				m_ApplyBanker.pop_front();
				auto player = get_player_byid(id);
				if(player)
				{
					nBanker = id;
					nGold = player->get_gold();
					nRound = 0;
					nResult = 0;
				}
				else
				{
					assert(0);
				}
			}
			else
			{ // 系统做庄
				nBanker = 0;
				nGold = m_roomcfg->mBankerGold;
				nRound = 0;
				nResult = 0;
			}
		}
	}
	// 系统做庄
	else if(nBanker==0)
	{
		if(m_ApplyBanker.size()>0)
		{ // 玩家做庄
			uint32_t id  = m_ApplyBanker.front();
			m_ApplyBanker.pop_front();
			auto player = get_player_byid(id);
			if(player)
			{
				nBanker = id;
				nGold = player->get_gold();
				nRound = 0;
				nResult = 0;
				bChangeBanker = true;
			}
		}
		else
		{
			bChangeBanker = false;
		}

	}
	//切换判断
	if (bChangeBanker)
	{
		onNoticeChangeBanker();
	}

	return bChangeBanker;
}

void bmw_space::logic_table::CleanRunaway()
{
	// 清理逃跑
	
}

void bmw_space::logic_table::WriteSystemScore(int64_t lSystemScore)
{
	// 杀分模式不记录
	if (get_room()->getkiller() == 10) return;
	if (lSystemScore != 0)
	{
		SLOG_CRITICAL << boost::format("[WriteSystemScoreA]WinScore:%d,Score:%d") % m_lAndroidWinScore%lSystemScore;
		m_lAndroidWinScore += lSystemScore;
		SLOG_CRITICAL << boost::format("[WriteSystemScoreB]WinScore:%d,Score:%d") % m_lAndroidWinScore%lSystemScore;

		int64_t lUserTax = m_lAllTax - m_lAITax;
		SLOG_CRITICAL << boost::format("[WriteSystemScore]WinScore:%d,PlayerTax:%d,RobotTax:%d") % m_lAndroidWinScore%lUserTax%m_lAITax;
	}
}

int32_t bmw_space::logic_table::RandGameResult()
{
	std::random_device rd;
	int32_t nResIndex = -1;
	// 没有玩家下注，随机出一个
	int64_t lAll = m_lAllSmallVW2 + m_lAllSmallBenz2 + m_lAllSmallBMW2 + m_lAllSmallPorsche2 + 
		m_lAllBigVW2   + m_lAllBigBenz2   + m_lAllBigBMW2   + m_lAllBigPorsche2;
	if (lAll==0)
	{
		// 平均分布
		std::mt19937_64 g(rd());
		std::uniform_int_distribution<> dis(1, 10000);
		nResIndex = dis(g) % PLACE_AREA + 1;
		SLOG_CRITICAL<<boost::format("No-Anyone to place ,rand:%1%")%nResIndex;
		return nResIndex;
	}
	// 默认概率
	int32_t nSrvRate[PLACE_AREA] = {24,24,24,24,12,6,4,3};
	int32_t nDesRate[PLACE_AREA] = {24,24,24,24,12,6,4,3};
	// 概率调整	
	int nBigUpRate = m_lotterycfg.mBigRate;
	int dwRandTypeMinute = m_lotterycfg.mRefreshMinute;
	int nRandType = m_lotterycfg.mRandType;
	/*
	0  固定概率
	1  可变概率
	2  大概率
	*/
	for (int i = 0; i < PLACE_AREA; i++)
	{
		std::mt19937_64 g(rd());
		std::uniform_int_distribution<> dis(0, 10000);
		if (nRandType == 0)
		{
			// 
		}
		else if (nRandType == 1)
		{
			nDesRate[i] = dis(g) % (2 * nSrvRate[i]);
		}
		else if (nRandType == 2)
		{
			nDesRate[i] = dis(g) % (2 * nSrvRate[i]);
			if (i >= ID_BIG_VW - 1)
			{
				nDesRate[i] += nDesRate[i] * nBigUpRate / 100;
			}
		}
	}
	// 获取
#if 0
	int64_t lAndroidWinScore = BMW_ResultCtrl::GetSingleton()->GetData("AndroidWinScore")->mValue;
#else
	int64_t lAndroidWinScore = m_lAndroidWinScore;
#endif
	int64_t lAndroidSafeScore = BMW_ResultCtrl::GetSingleton()->GetData("AndroidSafeScore")->mValue;

	// 分析区域
	int64_t lAreaScore[PLACE_AREA] = {0};
	for (int32_t i=0;i<PLACE_AREA;i++)
	{
		AnalysisAreaScore(i,lAreaScore[i]);
	}
	// 系统输赢安全分计算
	if (lAndroidWinScore + m_lAITax <= lAndroidSafeScore)
	{ // 系统输了
#if 0
		//  1、检测是否全输
		bool bAllLost = true;
		for (int32_t i = 0; i < PLACE_AREA; i++)
		{
			if (lAreaScore[i] + lAndroidWinScore > lAndroidSafeScore)
			{
				bAllLost = false;
				break;
			}
		}
		// 2、全输 选择一个最少支出
		// 3、赢 选择最大赢面补充
		if (bAllLost)
		{  // 输 选择一个最小
			int64_t lLostMinScore = lAreaScore[0];
			nResIndex = 0;
			for (int32_t i = 1; i < PLACE_AREA; i++)
			{
				if (lLostMinScore > lAreaScore[i])
				{
					lLostMinScore = lAreaScore[i];
					nResIndex = i;
				}
			}
		}
		else
		{
			int64_t lWinMaxScore = lAreaScore[0];
			nResIndex = 0;
			for (int32_t i = 1; i < PLACE_AREA; i++)
			{
				if (lWinMaxScore > lAreaScore[i])
				{
					lWinMaxScore = lAreaScore[i];
					nResIndex = i;
				}
			}
		}
#else
		std::vector<int32_t> VecIdx;
		// 取对系统有利
		int64_t lMaxScore = lAreaScore[0];
		for (int32_t i = 1; i < PLACE_AREA; i++)
		{
			if (lMaxScore <= lAreaScore[i])
			{
				lMaxScore = lAreaScore[i];
			}
		}
		// 如果存在相同的情况
		for (int32_t i = 0; i < PLACE_AREA; i++)
		{
			if (lMaxScore == lAreaScore[i])
			{
				VecIdx.push_back(i);
			}
		}
		int nCount = VecIdx.size();
		if (nCount > 0)
		{
			std::mt19937_64 g(std::time(0));
			std::uniform_int_distribution<> dis(1, 10000);
			nResIndex = VecIdx.at(dis(g) % nCount);
		}
#endif
	}
	else
	{ // 系统赢了	
		std::uniform_int_distribution<> dis(0, 10000);
		int nRand = dis(rd) % 100;
		if (nRand <= 80)
		{// 1、根据目标概率开奖
			std::vector<int32_t> VecIdx;
			int DesRand = dis(rd) % 100;
			for (int32_t i = 0; i < PLACE_AREA; i++)
			{
				if (DesRand <= nDesRate[i])
				{
					VecIdx.push_back(i);
				}
			}
			// 如果存在相同的情况
			int nCount = VecIdx.size();
			if (nCount > 0)
			{
				std::mt19937_64 g(std::time(0));
				std::uniform_int_distribution<> dis(1, 10000);
				nResIndex = VecIdx.at(dis(g) % nCount);
			}
		}
		else
		{// 2、根据下注最多区域排除机器人下注
			// 基本是开小的
			// TODO
			
		}
	}
	SLOG_WARNING << boost::format("[RandA]==============[%1%]") % nResIndex;
	if( nResIndex==-1)
	{
		std::uniform_int_distribution<> d(1, 10000);
		nResIndex =	 d(rd) % PLACE_AREA + 1;
		SLOG_CRITICAL << boost::format("Can't find proper index ,rand:%1%") % nResIndex;
		return nResIndex;
	}
	// 0-7
	nResIndex++;
	SLOG_WARNING << boost::format("[RandB]==============[%1%]") % nResIndex;
	return nResIndex;
}

void bmw_space::logic_table::AnalysisAreaScore(int32_t nIndex,int64_t& lAndroidScore)
{
	int64_t lUserScore,lSystemScore = 0;
	bool bIsSystemBanker = false;//机器人坐庄
	//区域倍率
	int32_t cbMultiple[] = {5,5,5,5,10,20,30,40};
	//是系统做庄.
	uint32_t &nBanker = m_nBankerID;
	if(nBanker==0)
	{
		bIsSystemBanker = true;
	}
	else
	{
		auto it = m_MapTablePlayer.find(nBanker);
		if(it!=m_MapTablePlayer.end())
		{
			auto player = it->second.p_playerptr;
			if(player && player->is_robot())
			{
				bIsSystemBanker = true;
			}
		}
	}

	//玩家成绩
	for (auto tableplayer : m_MapTablePlayer)
	{
		auto user = tableplayer.second;
		auto uid = user.p_id;
		int64_t lScore[PLACE_AREA] = {0};
		auto it =  m_MapAreaChip.find(uid);
		if(it != m_MapAreaChip.end())
		{
			tagAreaChip chip;
			chip = it->second;
			lScore[0] = chip.small_vw;
			lScore[1] = chip.small_benz;
			lScore[2] = chip.small_bmw;
			lScore[3] = chip.small_porsche;
			lScore[4] = chip.big_vw;
			lScore[5] = chip.big_benz;
			lScore[6] = chip.big_bmw;
			lScore[7] = chip.big_porsche;
		}
		if(uid == nBanker) continue;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		for (int32_t i = 0;i<PLACE_AREA;i++)
		{
			if(nIndex == i)
			{
				lUserScore = lScore[i] * (cbMultiple[i]-1);
			}
			else
			{
				lUserScore = -lScore[i];
			}
			//记录机器人输赢
			if(player->is_robot())
			{
				lSystemScore += lUserScore;
			}
			if(bIsSystemBanker)
			{
				lSystemScore -= lUserScore;
			}
		}
	}

	// 机器人输赢
	lAndroidScore = lSystemScore;
}

int32_t bmw_space::logic_table::GetMaxArea()
{
	int32_t nArea = 0;
	int64_t lAll[PLACE_AREA] = {
		m_lAllSmallVW,m_lAllSmallBenz ,m_lAllSmallBMW , m_lAllSmallPorsche ,
		m_lAllBigVW  ,m_lAllBigBenz   ,m_lAllBigBMW   , m_lAllBigPorsche};
	int64_t lMax = lAll[0];
	for (int32_t i=1;i<PLACE_AREA;i++)
	{
		if(lMax <= lAll[i])
		{
			lMax = lAll[i];
			nArea = i;
		}
	}
	// nArea++;
	return nArea;
}

int32_t bmw_space::logic_table::GetPlayerMaxArea()
{
	/*
	**/
	int64_t lAll[PLACE_AREA] = {
		m_lAllSmallVW2,m_lAllSmallBenz2 ,m_lAllSmallBMW2 , m_lAllSmallPorsche2 ,
		m_lAllBigVW2  ,m_lAllBigBenz2   ,m_lAllBigBMW2   , m_lAllBigPorsche2 };
	int32_t nArea = 0;
	int64_t lMax = lAll[0];
	int64_t lSum = lAll[0];
	// 
	std::vector<int32_t> VecArea;
	for (int32_t i = 1; i < PLACE_AREA; i++)
	{
		if (lMax <= lAll[i])
		{
			lMax = lAll[i];
		}
		lSum += lAll[i];
	}
	if (lSum == 0)	nArea = -1;
	for (int32_t i = 0; i < PLACE_AREA; i++)
	{
		if (lMax == lAll[i])
		{
			VecArea.push_back(i);
		}
	}
	int nCount = VecArea.size();
	if (nCount > 0)
	{
		std::mt19937_64 g(std::time(0));
		std::uniform_int_distribution<> dis(1, 10000);
		nArea = VecArea.at(dis(g) % nCount);
	}

	return nArea;
}

void bmw_space::logic_table::OnEnterBackground(uint32_t playerid)
{
	SLOG_CRITICAL << boost::format("%1% :%2%") % __FUNCTION__%playerid;
	/*
	*  记录用户状态，
	*  1、庄 、游戏结束后 踢出房间
	*  2、庄队列  游戏结束离开队列踢出房间
	*  3、闲 游戏结束踢出房间
	*  4、在本轮游戏结束前回到游戏不再清除
	*/
	auto player_it = m_MapTablePlayer.find(playerid);
	if (player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR << boost::format("%1%: Can't find this user:%2%") % __FUNCTION__%playerid;
	}
	else
	{
		auto &user = player_it->second;
		user.p_background = true;
		auto player = user.p_playerptr;
	}
	
}

void bmw_space::logic_table::OnDealEnterBackground()
{
	for (auto user : m_MapTablePlayer)
	{
		auto player = user.second.p_playerptr;
		if (player == nullptr) continue;
		if (player->is_robot()) continue;
		uint32_t playerid = user.second.p_id;
		bool bbackground = user.second.p_background;
		if (player && bbackground)
		{
			if (playerid == m_nBankerID)
			{
				// 下庄
				ChangeBanker(true);
			}
			//
			player->reqPlayer_leaveGame();
		}
	}
}

void bmw_space::logic_table::CleanKickedPlayer()
{
	// 游戏结束踢出玩家
	for (auto &user : m_MapTablePlayer)
	{
		auto& tuser = user.second;
		auto player = tuser.p_playerptr;
		if (player == nullptr) continue;
		if (player->is_robot()) continue;
		auto &state = tuser.p_leave;
		if (state == true) continue;
		uint32_t playerid = user.second.p_id;
		int reason = 0;
		if (get_room()->getservice() == 0)
		{
			// @ 踢出用户：系统维护
			reason = 8;
			state = true;
		}
		else if (tuser.p_kick == true)
		{
			/*
			* @ 踢出用户 离开World服务
			*/
			reason = 7;
			tuser.p_kick = false;
			state = true;
		}
		else if (player->get_serverflag() == 1)
		{
			state = true;
			player->reqPlayer_leaveGame(3);
		}
		if (reason == 7 || reason == 8)
		{
			auto sendmsg = PACKET_CREATE(bmw_protocols::packetl2c_clean_out, bmw_protocols::e_mst_l2c_clean_out);
			sendmsg->set_reason((bmw_protocols::e_msg_cleanout_def)reason);
			sendmsg->set_sync_gold(player->get_gold());
			player->send_msg_to_client(sendmsg);

			player->reqPlayer_leaveGame();
		}
	}
}

