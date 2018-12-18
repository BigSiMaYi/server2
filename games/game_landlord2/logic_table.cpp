#include "stdafx.h"
#include "logic_table.h"
#include "logic_player.h"

#include "logic_room.h"
#include "game_engine.h"
#include "i_game_ehandler.h"
#include "game_db.h"
#include "logic_lobby.h"
#include <net/packet_manager.h>
#include "time_helper.h"

#include "landlord3_def.pb.h"
#include "landlord3_logic.pb.h"
#include "Landlord3_RobotGameCFG.h"
#include "Landlord3_BaseInfo.h"

#include "enable_random.h"

#include "robot_mgr.h"
#include "logic_common.h"
#include "logic2logsvr_msg_type.pb.h"

#include <cfloat> // DBL_MAX
#include <cmath> // std::nextafter

using namespace boost;

LANDLORD_SPACE_USING

static const int MAX_TALBE_PLAYER = 5;//桌子人数
static const int MAX_OP_PLAYER = 4;//观战人数

static const std::string k_logflag = "---------- ";

#define MAX_TIME	180

void get_seat(int seat, int& preId, int& nextid)
{
    for (int i = 0; i < 3; ++i)
    {
        if (seat == i)
        {
            preId = i - 1;
            nextid = i + 1;

            if (preId < 0)
            {
                preId += 3;
            }
            if (nextid > 2)
            {
                nextid -= 3;
            }
        }
    }
}

//-----------------------------------------------------------------------
logic_table::logic_table(void)
    : m_room(nullptr)
    , m_player_count(0)
    , m_elapse(0.0)
    , m_checksave(0)
    , m_questioned(0)
    , m_robot_cfg(0)
    , m_service_status(0)
    , m_server_stop(false)
    , m_player_stock_factor(0, 0)
	, m_log_flag(false)
	, m_robot_switch(false)
	, m_tb_palyer_size(1)
	, m_round(0)
	, m_cutround_tag(0)
{
	initGame();
	for (int32_t i = 0; i < GAME_PLAYER; i++)
	{
		m_chair_ids.push_back(i);
	}
}

logic_table::~logic_table(void)
{
}

void logic_table::init_table(uint16_t tid, logic_room* room)
{
	m_room = room;
    m_tid = tid;
	set_status(eGameState_Free);

	m_ReadyTime = Landlord3_BaseInfo::GetSingleton()->GetData("PreTime")->mValue;
	m_BankerTime = Landlord3_BaseInfo::GetSingleton()->GetData("BankerTime")->mValue;
	m_OperaTime = Landlord3_BaseInfo::GetSingleton()->GetData("OperaTime")->mValue;
	m_ResultTime = Landlord3_BaseInfo::GetSingleton()->GetData("ResultTime")->mValue;

    m_robot_cfg = std::make_shared<robot_cfg>(m_room, this, m_MapTablePlayer, tid);
	if (m_robot_switch)
	{
		m_tb_palyer_size = m_room->get_roomcfg()->mPlayerMaxCounter;
	}
	else
	{
		m_tb_palyer_size = 3;
	}
}

uint32_t logic_table::get_id()
{
	return m_tid;
}

int8_t logic_table::get_status()
{
	return m_tb_status;
}

void logic_table::set_status(int8_t state)
{
    //char st = m_analyser.GetStatus();
    m_tb_status = state;
	if (state == eGameState_Free)
	{
		repositGame();
	}
}

void logic_table::CheckPlayerStatus()
{
    std::vector<LPlayerPtr> pids;
    int64_t now = GetCurrentStamp64();
	int i = 0;
    for (auto& item : m_MapTablePlayer)
    {
        auto& user = item.second;
		auto dur = now - user.p_selftime;
        auto uid = user.p_id;
		auto interval = global_random::instance().rand_int(8, m_ReadyTime) + i * 2;
		++i;
        auto player = user.p_playerptr;
        int64_t gold = user.p_asset;
        int32_t noaction = user.p_noaction;
        int32_t nReason = 0;
		if (player->is_robot() && m_robot_switch == false && now - user.p_selftime >= m_ReadyTime-2)
		{
			nReason = 5;
		}
        if (player->get_wait() >= 3)
        {
            nReason = 3;
        }
        else if (user.p_noaction >= 2)
        {
            nReason = 4;
        }
        else if (gold < m_room->get_data()->mKickGoldCondition && now - user.p_selftime >= m_ReadyTime - 2)//不立即踢出用户;
        {
            nReason = 5;
        }
        else if (player->get_kick_status() != 0)
        {
            nReason = landlord3_protocols::e_msg_cleanout_def::e_cleanout_gm_kick;
        }
		else if (player->get_leave_status() != 0 && dur >= interval)
		{
			nReason = 5;
		}
        else if (user.p_active == false)// 未准备玩家踢出桌子;
        {
            if (now - user.p_selftime >= m_ReadyTime + TIME_LESS/*+50*/)
            {
                nReason = landlord3_protocols::e_msg_cleanout_def::e_cleanout_not_ready_three;
                
                user.p_selftime = now;
            }
        }

        if (nReason != 0)
        {
            onGameCleanOut(uid, nReason);
			pids.push_back(user.p_playerptr);
        }
    }

    for (auto& id : pids)
    {
		if (id->is_robot())
		{
			auto pid = id->get_pid();
			bc_leave_seat(pid);
			m_robot_cfg->release_robot(pid);
		}
		else
		{
			id->leave_table();
		}
    }
}

void logic_table::heartbeat( double elapsed )
{
    m_robot_cfg->robot_heartbeat(elapsed);
    if (m_player_count == 0)
    {
        return;
    }
    
    if (m_duration >= 0)
    {
        m_duration -= elapsed;
    }

	m_elapse += elapsed;
    
    for (auto& user : m_MapTablePlayer)
    {
        LPlayerPtr& player = user.second.p_playerptr;
        if (player != nullptr)
        {
            player->heartbeat(elapsed);
        }
    }

    int8_t nStatus = this->get_status();
	switch (nStatus)
	{
	case eGameState_Free:
		{
            if (m_service_status == 100)
            {
                if (!m_server_stop  && m_duration <= 0)
                {
                    server_stop();
                    m_server_stop = true;
                }
                return;
            }
			// 清理超时未准备玩家;
            // CleanOutPlayer();
            CheckPlayerStatus();
			int nReadyCount = GetActiveUserCount();
			if( nReadyCount==GAME_PLAYER)
			{
                m_analyser.Start();
				onGameStart();
			}
		}
		break;
	case eGameState_FaPai:
        {
            if (m_duration <= 0)
            {
                m_duration = 0.0;
                set_status(eGameState_Banker);
                SetPlayerTime(m_curpid);
                onNoticeJiao(m_curpid);
                if (IsRobot(m_curpid))
                {
                    m_duration = GetRobotTimes(eGameState_Banker);
                    //m_duration = global_random::instance().rand_int(2, 3) + TIME_LESS;
                }
                else
                {
                    m_duration = m_BankerTime + TIME_LESS;
                }
            }
        }
        break;
	case eGameState_Banker:
		{
			if(m_duration<=0)
			{            
                defaultJiao(m_curpid);
			}
		}
		break;
	case eGameState_Play:
		{
			if (m_duration<=0)
			{
				defaultOutCard();
			}
			if (nStatus == eGameState_End)
			{
				m_duration = 0;
				//SLOG_CRITICAL << k_logflag << "defaultOutCard: time:" << boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());
			}
		}
		break;
	case eGameState_End:
		{
			if (!m_log_flag)
			{
				//SLOG_CRITICAL << k_logflag << m_duration;
				m_log_flag = true;
				m_duration = 0;
			}
			
			if (m_duration<=0)
			{
				m_duration = 0.0;

				// 桌子复位
				repositGame();
                m_analyser.GameOver();

                //复盘日志;
                m_landlordlog->set_endtime(time_helper::instance().get_cur_time());
                //m_landlordlog->set_discard(m_discardinfo);
                std::string debugstr = m_landlordlog->SerializeAsString();
                //SLOG_CRITICAL << m_landlordlog->DebugString();
				game_engine::instance().get_handler()->sendLogInfo(debugstr, game_engine::instance().get_gameid());
                set_status(eGameState_Free);
			}
			if (m_service_status == 100)
			{
				m_duration = 15.0;//停服等待15s在踢人;
			}
		}
		break;

	default:
		break;
	}
}

void logic_table::inc_dec_count(bool binc/* = true*/)
{
    if (binc)
    {
        m_player_count++;
    }
    else
    {
        m_player_count--;

        //if (m_player_count == 0)
        //store_game_object();
    }
}

int logic_table::enter_table(LPlayerPtr player)
{
	if(player->get_table()!=nullptr) 
	{
		SLOG_CRITICAL <<"logic_table enter_table is in table";
		return 2;
	}
    if (player->is_robot())
    {
        int32_t robotsize = get_robot_size();
		int32_t playersize = get_player_size(); //如果房间有超过一个玩家，则机器人不进入;
        if (robotsize >= m_robot_cfg->robot_maxcounter() || playersize > 1)
        {
			SLOG_CRITICAL << "robot size :" << robotsize << ", player size: "<< playersize;
            return 12;
        }
    }
    else
    {
		if (get_player_size() == 0) //第一个玩家进入，取它的库存系数，看它是否能匹配机器人;
		{
			m_player_stock_factor = player->get_stock_factor();
		}
		
		if (m_robot_switch == true && m_player_stock_factor.second == 1)
		{
			if (!can_enter_table(player))
			{
				SLOG_CRITICAL << ">>> current player size: "<< get_player_size() <<", room cfg player size: "<< m_room->get_roomcfg()->mPlayerMaxCounter;
				return 12;
			}
		}
		else
		{
			if (get_player_size() >= 3)
			{
				SLOG_CRITICAL << ">>> current player size: " << get_player_size() << ", room cfg player size: " << m_room->get_roomcfg()->mPlayerMaxCounter;
				return 12;
			}
		}
    }

	// 随机分配椅子;
	if(m_chair_ids.size()>0)
	{
		std::random_shuffle(std::begin(m_chair_ids),std::end(m_chair_ids));
		int32_t seat = m_chair_ids.back();
		m_chair_ids.pop_back();

        player->join_table(this, seat);

		bc_enter_seat(seat, player);

		inc_dec_count();
		//
		TablePlayer table_player;
		memset(&table_player,0,sizeof(table_player));
		table_player.p_playerptr = player;
		table_player.p_idx = seat;
		uint32_t uid = player->get_pid();
		table_player.p_id  = uid;
		table_player.p_asset = player->get_gold();
		table_player.p_active = false;

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

		SLOG_CRITICAL <<boost::format("logic_table::enter_table:%1%,seatid:%2%")%get_id()%seat;
		return 1;
	}
    else
    {
		SLOG_CRITICAL << "player enter table, id:"<< player->get_pid() << ", table seat is full";
        //player->join_table(nullptr, -1);
    }
	
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
		m_chair_ids.push_back(seat);
		LPlayerPtr& player = user.p_playerptr;
		// 允许弃牌离开;
		if(player)
		{
            if (player->get_status() == eUserState_dead)
            {
                int64_t lUserExpend = user.p_expend;
                int64_t lGold = 0;
                if (lUserExpend > 0)
                {
                    lGold -= lUserExpend;
                    player->write_property(lGold);
                    SLOG_CRITICAL << boost::format("User[%1%]-Leave-Table:expend:%2%") % pid%lGold;
                }
            }
            // 清理用户;
            player->set_status(eUserState_null);
            player->clear_round();
            player->set_wait(0);
		}
		if (player->is_robot())
		{
			m_room->record_robot(false);
		}
		m_room->leave_table(pid);
		// 
		inc_dec_count(false);

		bc_leave_seat(pid);	

		m_MapTablePlayer.erase(it);
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
			//auto status = player->get_status();
			auto status = this->get_status();
			if(status != eGameState_Free)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
	}
    return true;
}

bool logic_table::change_sit(uint32_t pid, uint32_t seat_index)
{
	return true;
}

int32_t logic_table::GetActiveUserCount()
{
	int32_t nActiveCount = 0;
	for(auto& user : m_MapTablePlayer)
	{
		if(user.second.p_active == true)
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
		SLOG_CRITICAL<<"player Can't Ready, not found id: " << playerid;
		nResult = eReadyResult_Faild_unknow;
	}
	else
	{
		auto& tuser = it->second;
		tuser.p_active = false;
		tuser.p_playing = false;
		auto player = tuser.p_playerptr;
		if(player->is_robot())
		{
			if (!m_robot_switch || m_player_stock_factor.second == 0)
			{
				return false;
			}
			SLOG_CRITICAL<<boost::format("onEventUserReady:Robot-id:%1%")%playerid;
		}
		else
		{
			SLOG_CRITICAL<<boost::format("onEventUserReady:User-id:%1%")%playerid;
		}
		if (get_status() == eGameState_Free)
		{
			player->set_status(eUserState_free);
		}
		if(player->get_status()>eUserState_free)
		{
			SLOG_CRITICAL<<"*********** logic_table::onEventUserReady Can't Ready:usestate, status :"<< player->get_status() <<", m_durition: "<<m_duration;
			nResult = eReadyResult_Faild_userstate;
		}
		else if(get_status()!=eGameState_Free )
		{
			SLOG_CRITICAL<<"*********** logic_table::onEventUserReady Can't Ready:gamestate, status :" << get_status() << ", m_durition: " << m_duration;
			nResult = eReadyResult_Faild_gamestate;
		}
		else if(player->get_gold()< m_room->get_data()->mKickGoldCondition)
		{
			SLOG_CRITICAL<<"logic_table::onEventUserReady Can't Ready:gold";
			nResult = eReadyResult_Faild_gold;
		}
		else
		{
			// succeed
			tuser.p_active = true;
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

	m_VecActive.clear();

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
	//SLOG_CRITICAL<<"------------RepositGame------------";
	// 游戏数据初始化
	m_landlordpid = 0;
	m_lastpid = 0;
	m_curpid = 0;
    m_questioned = 0;
	m_nFirstID = 0;
	m_nextpid = 0;
	m_firstid = 0;
	
	m_VecActive.clear();

	m_nGameRate = 0;

	// reset
	m_vecCardData.clear();

	// 重置数据
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		if(player==nullptr) continue;
		user.p_active = false;
		user.p_playing = false;
		user.p_asset = player->get_gold();
		user.p_expend = 0;
		user.p_selftime = GetCurrentStamp64();
		user.p_bankrate = -1;

		user.p_outs = 0;
		user.p_isauto = false;
		user.p_islast1 = false;

		player->set_status(eUserState_free);
	}

	check_player_stock();
	// 清理不足条件用户
	//CleanOutPlayer();
}

void rewash_card(VECPOKER& cards)
{
    cards.clear();
    for (int32_t i = 0; i < GPOKERS; i++)
    {
        cards.push_back(g_pokers[i]);
    }
    std::random_device rd;
    std::mt19937_64 g(rd());
    for (int i = 0; i < 5; ++i) //混淆5次;
    {
        std::shuffle(cards.begin(), cards.end(), g);
    }
}

void logic_table::Wash_Card(bool reset)
{
	if (m_round++ >= 0)
	{
		rewash_card(m_vecCardData);
		m_round = 0;
		m_bomCounter = 0;
		return;
	}

    if (reset)
    {
        m_vecOutCard.clear();
    }
    if (m_vecOutCard.empty())
    {
        rewash_card(m_vecOutCard);
    }
	// 洗牌
    m_vecCardData.clear();
	std::srand(unsigned(std::time(0)));
	if(m_vecOutCard.size()==GPOKERS)
	{
		POKER *lpHead = &(*m_vecOutCard.begin());
		int32_t nCut = global_random::instance().rand_int(1,GAME_PLAYER);
		if(m_bomCounter >= 7)
		{
			nCut = 4;
		}
		else if(m_bomCounter > 3)
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
		for (auto& card : m_vecOutCard)
		{
			m_vecCardData.push_back(card);
		}

        if (!m_analyser.CheckPokerValue(m_vecCardData))
        {
            rewash_card(m_vecCardData);
        }
	}
	else
	{
        rewash_card(m_vecCardData);
	}
    m_bomCounter = 0;
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
    m_nBasePoker.clear();
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

bool logic_table::onGameStart()
{
	SLOG_CRITICAL<<"------------GameStart------------";

	// check play and active
	for(auto it=m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto &user = it->second;
		auto player = user.p_playerptr;
		auto playerid = user.p_id;
		if(player==nullptr) continue;
		if(user.p_active==true)
		{
			m_VecActive.push_back(playerid);
            user.p_isauto = false;
			user.p_autotime = 0;
            if (player->is_robot())
            {
                user.p_isauto = true; //机器人自动出牌;
            }
            else
            {
				m_player_recharge = player->get_recharge_gold();
				m_player_cut_roundflag = player->get_cut_round_flag();
                m_player_stock_factor = player->get_stock_factor();
				m_tb_pid = player->get_id();
            }
			user.p_expend = 0;
			user.p_result = 0;
			user.p_bankrate = -1; //没有叫地主;
            user.p_bankertime = 0;
            user.p_outcardtime = 0;
            user.p_discardtimeout = false;
            user.p_begingold = player->get_gold();

			player->set_status(eUserState_play);
			player->add_round();
			player->set_wait(0);
		}
	}
    if (m_VecActive.size() < 3)
    {
        return false;
    }
  
    if (get_robot_size() == 0)
    {
        srand((unsigned)time(nullptr));
        std::random_shuffle(m_VecActive.begin(), m_VecActive.end());

        m_curpid = m_VecActive.back();
        m_firstid = m_curpid;
    }
    else
    {
        int cfg_rate = Landlord3_BaseInfo::GetSingleton()->GetData("PlayerBankerRate")->mValue;
        double rate = global_random::instance().rand_01();
        if (rate < (cfg_rate / 100.0))
        {
            for (auto& id : m_VecActive)
            {
                if (!IsRobot(id))
                {
                    m_curpid = id;
                    m_firstid = m_curpid;
                    break;
                }
            }
        }
        else
        {
            for (auto& id : m_VecActive)
            {
                if (IsRobot(id))
                {
                    m_curpid = id;
                    m_firstid = m_curpid;
                    break;
                }
            }
        }
    }
	if (m_curpid == 0)
	{
		SLOG_ERROR << "current playerid = " << m_curpid;
		return false;
	}
	// 
	//Wash_Card();
	rewash_card(m_vecCardData);
	m_bomCounter = 0;
	m_log_flag = false;
	m_player_pokers.clear();
	m_player_out_pokers.clear();

    //复盘日志;
    m_landlordlog.reset();
    m_landlordlog = std::make_shared<logic2logsvr::LandlordGameLog>();
    m_landlordlog->set_begintime(time_helper::instance().get_cur_time());
    auto room_id = get_room()->get_room_id();
    auto child_id = get_room()->get_child_id();
    std::string now = boost::posix_time::to_iso_extended_string(boost::posix_time::second_clock::local_time());
    std::string id = std::to_string(room_id) + "_" + std::to_string(child_id) + "_" + std::to_string(get_id()) +"_"+ now;
    m_landlordlog->set_gameroundindex(id);
    auto gameinfo = m_landlordlog->mutable_ginfo();
    gameinfo->set_gameid(game_engine::instance().get_gameid());
    gameinfo->set_roomid(get_room()->get_room_id());
    gameinfo->set_tableid(get_room()->get_child_id());
    //复盘日志end;

    int32_t  bomb_total = 0;
	int rewashsize = 0;
    auto it = m_VecActive.begin();
    while (it != m_VecActive.end())
    {
        uint32_t playerid = *it;

        VECPOKER poker;
        Allot_Card(playerid, poker);
		if (m_analyser.CheckPokerValue(poker))
		{
			m_player_pokers.insert(std::make_pair(playerid, poker));
			it++;
		}
		else
		{
			it = m_VecActive.begin();
			m_player_pokers.clear();
			rewash_card(m_vecCardData);
			if (++rewashsize >= 3)
			{
				set_status(eGameState_Free);
				return false;
			}
		}
#if 0
        std::sort(poker.begin(), poker.end(), POKER_GREATER());
        bool reset = false;
        bool reset_card = false;
        if (!m_analyser.CheckPokerValue(poker))
        {
            reset_card = true;
            reset = true;
        }

        int32_t bombsize = 0;
        int32_t groupsize = 0;
        try
        {
            m_analyser.GetCardGroupBomb(poker, bombsize, groupsize);
        }
        catch (std::exception& e)
        {
            SLOG_CRITICAL << "------------ GetCardGroupBomb exception: "<<e.what();
            reset_card = true;
            reset = true;
        }
        catch (...)
        {
            SLOG_CRITICAL << "------------ GetCardGroupBomb exception: unkown";
            reset_card = true;
            reset = true;
        }
        
        //int wight = m_analyser.CalcCardsWight(poker);
        bomb_total += bombsize;

        if (!IsRobot(playerid))
        {
            if (groupsize > 9)
            {
                reset = true;
            }
        }
        if (bombsize > 3 || bomb_total > 7)
        {
            reset = true;
        }
        if (reset)
        {
            Wash_Card(reset_card);
            it = m_VecActive.begin();
            m_player_pokers.clear();
        }
        else
        {
            if (m_analyser.CheckPokerValue(poker))
            {
                m_player_pokers.insert(std::make_pair(playerid, poker));
                it++;
            }
            else
            {
                it = m_VecActive.begin();
                m_player_pokers.clear();
            }
        }
#endif
    }

	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{ // 参与游戏用户
		auto &user = it->second;
		auto player = user.p_playerptr;
        if (player == nullptr)
        {
            set_status(eGameState_Free);
            return false;
        }
		uint32_t playerid = player->get_pid();

        auto itr =m_player_pokers.find(playerid);
        if (itr != m_player_pokers.end())
        {
            m_analyser.SendCard(user.p_idx, itr->second, player->is_robot());
            //发送消息;
            SendCardMessage(player, itr->second);
        }
        else
        {
            set_status(eGameState_Free);
            return false;
        }
      
        //复盘日志;
        //auto cardsinfo = m_landlordlog->add_cardsinfo();
	}

	set_status(eGameState_FaPai);

	// 计算发牌时间;
	float fCardTime = Landlord3_BaseInfo::GetSingleton()->GetData("CardTime")->mValue;
	float fPauseTime = Landlord3_BaseInfo::GetSingleton()->GetData("PauseTime")->mValue;

	m_duration = fCardTime + fPauseTime;

	return true;
}

uint32_t logic_table::GetNextPid(uint32_t pid)
{
    int seat = get_seat_byid(pid);
    int pre_sid;
    int next_sid;
    get_seat(seat, pre_sid, next_sid);
    uint32_t nextid = get_id_byseat(next_sid);
    SetPlayerTime(nextid);

    return nextid;
#if 0
    if (m_pro_landloard_id == 0) //判断前一个玩家是否叫过;
    {
        m_pro_landloard_id = m_get_landloar_id;
        m_curpid = m_get_landloar_id;
        return m_get_landloar_id;
    }
    else
    {
        m_pro_landloard_id = m_get_landloar_id;
        int cur_getlandlord_id = m_pro_landloard_id;

        auto player_it = m_MapTablePlayer.find(cur_getlandlord_id);
        if (player_it != m_MapTablePlayer.end())
        {
            m_curpid = cur_getlandlord_id;
            bool turn_next = true;

            auto& user = player_it->second;
            auto player = user.p_playerptr;

            if (user.p_bankrate == -1) //还没有叫;
            {
                int64_t now = GetCurrentStamp64();
                if (now - user.p_bankertime >= 10)//叫牌超时，不叫;
                {
                    int32_t robrate = 0;
                    if (player->is_robot())
                    {
                        int32_t robrate = m_analyser.CalcGetLandScore(user.p_idx);
                    }
                    onEventJiao(m_get_landloar_id, robrate);
                }
                else
                {
                    turn_next = false;
                }
            }
            if (turn_next)
            {
                int seat = get_seat_byid(m_get_landloar_id);
                int preid;
                int nextid;
                get_seat(seat, preid, nextid);
                uint32_t nextpid = get_id_byseat(nextid);
                m_pro_landloard_id = m_get_landloar_id;
                m_get_landloar_id = nextpid;
                return m_pro_landloard_id;
            }
        }
    }
    return 0;
#endif
}

void logic_table::CheckLandlordTimeout(uint32_t pid)
{
    auto player_it = m_MapTablePlayer.find(pid);
    if (player_it != m_MapTablePlayer.end())
    {
        m_curpid = pid;
        bool turn_next = true;

        auto& user = player_it->second;
        auto player = user.p_playerptr;

        if (user.p_bankrate == -1) //还没有叫;
        {
            int64_t now = GetCurrentStamp64();
            if (user.p_bankertime == 0)
            {
                user.p_bankertime = now;
            }
            if (now - user.p_bankertime >= m_BankerTime)//叫牌超时，不叫;
            {
                int32_t robrate = 0;
                if (player->is_robot())
                {
                    robrate = m_analyser.CalcGetLandScore(user.p_idx);
                }
                onEventJiao(pid, robrate);
            }
        }
    }
}

void logic_table::CheckDiscardTimeout(TablePlayer& user)
{
    auto player = user.p_playerptr;
    int64_t now = GetCurrentStamp64();
    if (now - user.p_outcardtime >= m_OperaTime)//出牌超时;
    {
        onEventAuto(player->get_pid(), true);
		user.p_isauto = true;
        //user.p_discardtimeout = true;
        m_duration = TIME_LESS + 0;
    }
}

void landlord_space::logic_table::SetPlayerTime(uint32_t pid)
{
    auto tb_itr = m_MapTablePlayer.find(pid);
    if (tb_itr != m_MapTablePlayer.end())
    {
        int64_t now = GetCurrentStamp64();
        tb_itr->second.p_bankertime = now;
        tb_itr->second.p_outcardtime = now;
    }
}

bool landlord_space::logic_table::IsRobot(uint32_t pid)
{
    auto tb_itr = m_MapTablePlayer.find(pid);
    if (tb_itr != m_MapTablePlayer.end())
    {
        if (tb_itr->second.p_playerptr)
        {
            return tb_itr->second.p_playerptr->is_robot();
        }
    }
    return false;
}

bool logic_table::onEventJiao(uint32_t playerid, int32_t robrate)
{
    SLOG_CRITICAL << __FUNCTION__ << " : " << __LINE__ << " playerid : " << playerid << ", robrate : " << robrate;

	int32_t nGameState= get_status();
	if( nGameState != eGameState_Banker)
	{
		SLOG_CRITICAL << "game current state : "<< nGameState <<" error,  state: "<< eGameState_Banker <<" can`t not apply banker";
		return false;
	}

    if (m_curpid != playerid)
    {
		SLOG_CRITICAL << "current player: " << m_curpid << ", apply banker: " << playerid << ", ";
        return false;
    }

    if (!IsValidRate(robrate))
    {
		SLOG_CRITICAL << "onGameRobBanker invalid rate";
        return false;
    }

    auto player_it = m_MapTablePlayer.find(playerid);
    if (player_it == m_MapTablePlayer.end())
    {
        SLOG_CRITICAL << " player not found";
        return false;
    }
	auto& user =  player_it->second;
    user.p_bankrate = robrate;

    m_analyser.GetLandlord(user.p_idx, robrate);

	// send msg
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_robbank_result,landlord3_protocols::e_mst_l2c_ask_robbank_result);
	sendmsg->set_playerid(playerid);
	sendmsg->set_robrate(robrate);
	broadcast_msg_to_client2(sendmsg);

	// check
	maybe_JiaoEnd(playerid, robrate);

	return true;
}

void logic_table::defaultJiao(uint32_t playerid)
{
    //SLOG_CRITICAL << boost::format("%1%:%2%,playerid:%3%") % __FUNCTION__%__LINE__%playerid;

    auto player_it = m_MapTablePlayer.find(playerid);
    if (player_it != m_MapTablePlayer.end())
    {
        auto& user = player_it->second;
        auto player = user.p_playerptr;
        if (player && player->is_robot())
        {
            //机器人叫地主;
            int32_t robrate = m_analyser.CalcGetLandScore(user.p_idx);
            onEventJiao(playerid, robrate);
        }
        else
        {
            CheckLandlordTimeout(playerid);
        }
    }
	else
	{
		SLOG_CRITICAL << __FUNCTION__ <<":"<< __LINE__ <<", playerid: " << playerid<<",  not found";
		set_status(eGameState_Free);
	}
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
	if (m_questioned == 1)
	{
		m_firstid = m_curpid;
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
            m_curpid = m_firstid;
            m_landlordpid = m_firstid;
            m_lastpid = 0;

            // 都不抢庄默认抢庄;
            auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_robbank_result, landlord3_protocols::e_mst_l2c_ask_robbank_result);
            sendmsg->set_playerid(m_curpid);
            sendmsg->set_robrate(eCallBanker_OnePoint);
            broadcast_msg_to_client2(sendmsg);
        }
    }

    if (m_landlordpid != 0)
    {
        MixRobotCards(m_landlordpid);

        m_bankerRate = m_nGameRate;
        m_curpid = m_landlordpid;
        m_lastpid = m_curpid;
        Allot_BaseCard(m_nBasePoker);
        std::stringstream sm;
        auto lorditr = m_player_pokers.find(m_landlordpid);
        for (auto& pk : m_nBasePoker)
        {
            if (lorditr != m_player_pokers.end())
            {
                auto& lordcard = lorditr->second;
                lordcard.push_back(pk);

                poker_to_stringstream(pk, sm);
            }
        }

        //复盘日志：每个玩家的牌;
        std::string card_str = sm.str();
        ReplayCardLog(m_landlordpid, card_str);

		auto chairid = -1;
		auto landlord_itr = m_MapTablePlayer.find(m_landlordpid);
		if (landlord_itr != m_MapTablePlayer.end())
		{
			chairid = landlord_itr->second.p_idx;
		}
		if (chairid <0 || chairid >= 3)
		{
			set_status(eGameState_Free);
			return;
		}
        //发送地主牌消息;
        m_analyser.SendLandlordCard(chairid, m_nBasePoker);
        onEventBaseCard(m_nBasePoker);

        //发完地主牌，状态转到出牌，等待 m_BankerTime;
        set_status(eGameState_Play);
        m_duration = TIME_LESS + m_BankerTime;
        if (IsRobot(m_landlordpid))
        {
            m_duration = GetRobotTimes(eGameState_Play);
            //m_duration = global_random::instance().rand_int(2, 3) + TIME_LESS;;
        }
    }
    else
    {
        m_curpid = GetNextPid(playerid);

        //轮到下一个用户叫地主，状态切换：不超时;
        set_status(eGameState_FaPai);
        m_duration = TIME_LESS + 0;
    }
}

void landlord_space::logic_table::MixRobotCards(uint32_t landlordpid)
{
	m_cutround_tag = 0;
	if (!m_player_cut_roundflag)
	{
		if (!cut_round_check(m_cfgCutRound))
		{
			return;
		}
	}
	else
	{
		SLOG_CRITICAL << "--------------[cut round check] tableid: " << get_id() <<", playerid: "<< m_tb_pid << ", player factor: "<< m_player_stock_factor.first;
		//杀分;
		if (!cut_round_check(m_player_stock_factor.first))
		{
			return;
		}
// 		int cut_round = /*m_room->get_cut_round() + */m_player_stock_factor.first;
// 		int rd = global_random::instance().rand_int(1, 10);
// 		if (cut_round < rd)
// 		{
// 			return;
// 		}
	}

	int robot_count = 0;
	for (auto user : m_MapTablePlayer)
	{
		LPlayerPtr& player = user.second.p_playerptr;
		if (player != nullptr)
		{
			if (player->is_robot())
				robot_count++;
		}
	}
	if (robot_count != 2)
	{
		return;
	}

    VECPOKER poker1;
    VECPOKER poker2;
    for (auto& item : m_player_pokers)
    {
        if (IsRobot(item.first))
        {
            if (poker1.empty())
            {
                poker1 = item.second;
            }
            else if (poker2.empty())
            {
                poker2 = item.second;
            }
        }
    }

    m_analyser.MixRobotCards(poker1, poker2);
    if (poker1.size() != 17 || poker2.size() != 17)
    {
        return;
    }
    if (!m_analyser.CheckPokerValue(poker1)
        || !m_analyser.CheckPokerValue(poker2))
    {
        return;
    }

	m_cutround_tag = 1;
	SLOG_CRITICAL << "--------------[cut round check] tableid: " << get_id() << ", playerid: " << m_tb_pid << ", player factor:" << m_player_stock_factor.first <<", cut round tag: " << m_cutround_tag;
    int index = 0;
    for (auto& item : m_player_pokers)
    {
        if (IsRobot(item.first)) //机器人;
        {
            if (item.first == landlordpid)
            {
                item.second = poker2;
            }
            else
            {
                ++index;
                if (index == 1)
                {
                    item.second = poker1;
                }
                if (index == 2)
                {
                    item.second = poker2;
                }
            }
        }
    }
    for (auto& item : m_MapTablePlayer)
    {
        auto itr = m_player_pokers.find(item.first);
        if (itr != m_player_pokers.end())
        {
            m_analyser.SendCard2(item.second.p_idx, itr->second, IsRobot(item.first));
        }
    }
}

bool logic_table::onEventAuto(uint32_t playerid,bool bAuto)
{
	// 
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_ERROR << ">> onEventAuto, playerid: " << playerid << ", not found ";
		return false;
	}
	auto &user =  player_it->second;
	bool bauto = CheckTrusteeStatus(user, bAuto);
	// 
	
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_trustee_result,landlord3_protocols::e_mst_l2c_trustee_result);
	sendmsg->set_playerid(playerid);
	sendmsg->set_bauto(bauto);
    if (user.p_playerptr)
    {
		if (get_status()== eUserState_play)
		{
			if (bauto)
			{
				user.p_playerptr->set_status(eUserState_Trustee);
				if (m_curpid == playerid)
				{
					m_duration = 0;
				}
			}
			else
			{
				user.p_playerptr->set_status(eUserState_play);
			}
		}
		
        user.p_playerptr->send_msg_to_client(sendmsg);
    }

    //设置当前玩家是否自动出牌;
    //m_analyser.SetRobot(user.p_idx, bAuto);

	return true;
}

bool logic_table::CheckTrusteeStatus(TablePlayer& user, bool bAuto)
{
	if (get_status() != eGameState_Play)
	{
		return false;
	}

	if (user.p_isauto == bAuto)
	{
		//SLOG_ERROR << ">> onEventAuto, playerid: " << user.p_id << ", bauto: " << bAuto;
		return user.p_isauto;
	}
	user.p_isauto = bAuto;
	return user.p_isauto;
}

void logic_table::defaultOutCard()
{
	auto itr = m_MapTablePlayer.find(m_curpid);
	if(itr != m_MapTablePlayer.end())
	{
	    auto& cur_user = itr->second;
        auto player = cur_user.p_playerptr;
        auto it = m_player_pokers.find(m_curpid);
        if (it != m_player_pokers.end() && player)
        {
			auto& mypoker = it->second;

			bool outcard = false;
			if (player->is_robot())
			{
				outcard = true;
			}
			else
			{
				int64_t now = GetCurrentStamp64();
				if (cur_user.p_isauto)
				{
					outcard = true;
				}
				else
				{
					if (now - cur_user.p_outcardtime >= m_OperaTime)
					{
						outcard = true;
						cur_user.p_outcardtime = now;
						onEventAuto(player->get_pid(), true);
						cur_user.p_isauto = true;
						m_duration = TIME_LESS + 0;
					}
				}
			}
			
            if (outcard)
            {
				int16_t cur_cid = get_seat_byid(m_curpid);
				int16_t last_cid = get_seat_byid(m_lastpid);
				m_analyser.SetCurrentPlayer(cur_cid);
				m_analyser.SetLastPlayer(last_cid);

                int cid = -1;
                int next_cid = -1;
                VECPOKER pokers;
                m_analyser.Discard(cid, next_cid, (VECPOKER&)pokers);

                if (cid == -1 || next_cid == -1)
                {
                    //玩家出牌错误;
                    SLOG_CRITICAL << k_logflag << "robot player discard error, pid: "<< m_curpid;
					set_status(eGameState_Free);
                    return;
                }

                m_nextpid = GetNextPid(m_curpid);

                bool ispass = m_analyser.IsPass(cid);
                if (ispass)
                {
					//pass 需要清空前面出牌记录,否则断线重连回来是上一次的牌;
					auto it_out = m_player_out_pokers.find(m_curpid);
					if (it_out != m_player_out_pokers.end())
					{
						auto& poker = it_out->second;
						poker.clear();
					}
                    defaultPass(m_curpid, m_nextpid, true);
                }
                else
                {
                    onEventOutCard(m_curpid, pokers, true);
                    //cur_user.p_outs += 1;
                }
                if (get_status() == eGameState_Free)
                {
                    m_duration = TIME_LESS + 1;
					SLOG_CRITICAL <<"get_status() == eGameState_Free: ";
                    return;
                }
                if (!IsRobot(m_nextpid))
                {
                    auto itr = m_MapTablePlayer.find(m_nextpid);
                    if (itr->second.p_isauto)
                    {
                        static int trustee_time = Landlord3_BaseInfo::GetSingleton()->GetData("TrusteeOperaTime")->mValue;
                        m_duration = trustee_time;
                    }
                    else
                    {
                        m_duration = TIME_LESS + m_OperaTime;
                    }
                }
            }
        }
		else
		{
			SLOG_CRITICAL << k_logflag << "m_player_pokers: palyerid: " << m_curpid << ", not found";
			set_status(eGameState_Free);
		}
    }
	else
	{
		SLOG_CRITICAL << k_logflag << "m_MapTablePlayer: palyerid: " << m_curpid << ", not found";
		set_status(eGameState_Free);
	}
}

bool landlord_space::logic_table::PlayerDiscard(uint32_t playerid, const VECPOKER&pokers)
{
    auto player_it = m_MapTablePlayer.find(playerid);
    if (player_it != m_MapTablePlayer.end())
    {
        auto& user = player_it->second;

        //检查出牌，如果不合格则不出，如果合格则轮到下一个玩家;
        return m_analyser.PlayerDiscard(user.p_idx, pokers);
    }
    else
    {
		SLOG_CRITICAL << k_logflag << "PlayerDiscard, playerid: " << playerid << ", not found ";
    }
    return false;
}

bool logic_table::onEventOutCard(uint32_t playerid, const VECPOKER&pokers, bool isauto)
{
	// check game state
	int32_t nGameState= get_status();
	if( nGameState != eGameState_Play)
	{
		SLOG_CRITICAL << "onEventOutCard error, playerid:"<< playerid <<", out cards: "<< poker_to_string(pokers) << ", isauto: " << isauto <<", lastpid:"<<m_lastpid;
		return false;
	}
	// check player
	if(m_curpid != playerid)
	{
		SLOG_CRITICAL <<"onGameOutCard OperatorID err, curpid: "<< m_curpid<<", discard playerid: "<< playerid << ", isauto: " << isauto << ", lastpid:" << m_lastpid;
		return false;
	}
	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_CRITICAL <<"onGameOutCard player err";
		return false;
	}

	// check card invalid
	if(CheckPoker(playerid,pokers)==false)
	{
        std::string discard = poker_to_string(pokers);
		SLOG_CRITICAL <<boost::format("%1%:%2% CheckPoker ")%__FUNCTION__%__LINE__<<": discard playerid : "<< playerid<<", discard: "<< discard<<", isauto: " << isauto << ", lastpid:" << m_lastpid;
		return false;
	}
	// Compare
	char style = m_logiccard.GetCardType(pokers);
	if(style == STYLE_NO)
	{
		SLOG_CRITICAL <<boost::format("%1%:%2% STYLE_NO ")%__FUNCTION__%__LINE__;
		//return false;
	}

    char val = m_logiccard.GetAValueFromAConstTypePoke(style, pokers);
    char len = pokers.size();

	if(style > STYLE_NO)
	{
        if (m_lastpid != 0 && m_curpid != m_lastpid)
        {
            //判断当前是否是重新出牌：即自己的牌没人在继续要;
            char v = m_logiccard.CompareCard(m_style, m_val, m_len, style, val, len);
            if (v != 1)
            {
                std::string discard = poker_to_string(pokers);
				SLOG_CRITICAL << boost::format("%1%:%2% Compare Err ") % __FUNCTION__%__LINE__ << ": lastpid: "<<m_lastpid <<"curpid:"<<m_curpid <<", discard: "<< discard << ", isauto: " << isauto;
                //return false;
            }
        }
	}

    if (isauto == false)//玩家出牌(不是机器人(系统)自己出牌);
    {
        if (!PlayerDiscard(playerid, pokers))
        {
            std::string discard = poker_to_string(pokers);
			SLOG_CRITICAL << k_logflag << "playerid: "<< playerid << ", check discard poker error: "<< discard << ", lastpid:" << m_lastpid;;
            return false;
        }
    }
    m_style = style;
    m_val = val;
    m_len = len;

    //复盘日志;
    std::stringstream sm;
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_outcard_result,landlord3_protocols::e_mst_l2c_outcard_result);
	sendmsg->set_playerid(playerid);
	auto cardinfo = sendmsg->mutable_cardinfo();
	//cardinfo->mutable_pokers()->Reserve(MAXPOKER1);
	for (auto& p : pokers)
	{
        poker_to_stringstream(p, sm);
        
		auto pokerinfo = cardinfo->add_pokers();
		pokerinfo->set_value(p.value);
		pokerinfo->set_style(p.style);
		// save out card
		m_vecOutCard.push_back(p);
	}

	std::string tm_str = boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time().time_of_day());
    std::string discard = sm.str();
    std::string discard_str = std::to_string(playerid) + " " +  tm_str + " " + std::to_string(isauto) + ": "  + discard;
    m_landlordlog->add_discard(discard_str);
    //SLOG_CRITICAL << k_logflag << discard_str;

	cardinfo->set_pokertype(style);
	cardinfo->clear_pokerstatus();
	// record current out card
	auto outit = m_player_out_pokers.find(playerid);
	if(outit != m_player_out_pokers.end())
	{
		auto& outpokers = outit->second;
		outpokers.clear();

		outpokers = pokers;

		//SLOG_CRITICAL<<format("%d,out card :%d")%playerid%len;
	}
    else
    {
        m_player_out_pokers[playerid] = pokers;
    }
    auto& cur_user = player_it->second;
    cur_user.p_outs += 1;
    m_nextpid = GetNextPid(m_curpid);
    m_nFirstID = m_lastpid;
    m_lastpid = m_curpid;
    m_curpid = m_nextpid;

    if (IsRobot(m_nextpid))
    {
        m_duration = GetRobotTimes(eGameState_Play);
        //m_duration = global_random::instance().rand_int(2, 3);
    }
    else
    {
		auto nextplayer_it = m_MapTablePlayer.find(m_nextpid);
		auto& next_user = nextplayer_it->second;
		if (next_user.p_isauto)
		{
			m_duration = GetRobotTimes(eGameState_Play);
		}
		else
		{
			m_duration = TIME_LESS + m_OperaTime;
		}
    }
	// remove hand card
	int32_t nCount = RemovePoker(playerid, pokers);
	// check maybe over
	bool bGameOver = false;
	if(nCount == 0)
	{
		sendmsg->clear_nextid();
		bGameOver = true;
	}
	else
	{
        sendmsg->set_nextid(m_nextpid);
	}

    broadcast_msg_to_client2(sendmsg);


	// chang rate
	if(style >= STYLE_BOMB)
	{
		m_nGameRate *= 2;
		onNoticeChangeRate();
		m_bomCounter++;
	}
	
	// 游戏结束
	if(bGameOver)
	{
		// TODO
		// 2017-9-7 11:17:07
        auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_open_card, landlord3_protocols::e_mst_l2c_open_card);
		// Save other hand card
		for(auto& item: m_player_pokers)
		{
            auto opencard = sendmsg->add_opencard();
            opencard->set_playerid(item.first);
            auto cardinfo = opencard->mutable_cardinfo();
			for (auto& p : item.second)
			{
                auto pokerinfo = cardinfo->add_pokers();
                pokerinfo->set_value(p.value);
                pokerinfo->set_style(p.style);
				m_vecOutCard.push_back(p);
			}

			//复盘日志：结束剩余手牌;
			int size = m_landlordlog->cardsinfo_size();
			for (int i = 0; i < size; ++i)
			{
				auto ptr = m_landlordlog->mutable_cardsinfo(i);
				if (ptr->pid() == item.first)
				{
					ptr->set_rem_cards(poker_to_string(item.second));
					break;
				}
			}
		}
        broadcast_msg_to_client2(sendmsg);
        onGameOver(playerid);
        //m_duration = 1;
	}

	return true;
}

bool logic_table::CheckPoker(uint32_t playerid,const VECPOKER&poker)
{
	// 
	bool bfind = false;
	auto it = m_player_pokers.find(playerid);
	if(it != m_player_pokers.end())
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
	auto it = m_player_pokers.find(playerid);
	if(it != m_player_pokers.end())
	{
		auto& mypoker = it->second;
		
        for (auto& p : poker)
        {
            bool notfound = true;
            for (auto myit = mypoker.begin(); myit != mypoker.end(); myit++)
            {
				if( (*myit) == p)
				{
                    notfound = false;
                    mypoker.erase(myit);
					break;
				}
			}
            if (notfound)
            {
                int a = 0;
                ++a;
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
    uint32_t nextid = get_id_byseat(nextseatid);

    if (nextid == 0)
    {
        int a = playerid;
        int b = a;
    }
	return nextid;
}

bool logic_table::onEventPass(uint32_t playerid,bool bTimeOver/*=false*/)
{
	int32_t nGameState= get_status();
	if( nGameState !=eGameState_Play)
	{
		SLOG_CRITICAL <<boost::format("%1%:%2% game state err:%3%")%__FUNCTION__%__LINE__%nGameState;
		return false;
	}

	auto player_it = m_MapTablePlayer.find(playerid);
	if(player_it == m_MapTablePlayer.end())
	{
		SLOG_CRITICAL <<boost::format("[Danger]%1%:%2% player err:%3%")%__FUNCTION__%__LINE__%playerid;
		return false;
	}
	auto &user =  player_it->second;
	auto player = user.p_playerptr;
	if(player && player->is_robot())
	{
		SLOG_CRITICAL <<boost::format("%1%:%2% Robot-id:%3%")%__FUNCTION__%__LINE__%playerid;
	}

	if(playerid != m_curpid)
	{
		//SLOG_ERROR<<boost::format("%1%:%2% current player err:")%__FUNCTION__%__LINE__;
		SLOG_CRITICAL << k_logflag << "current playerid: " << m_curpid << ", pass playerid: " << playerid << ", can`t pass";
		return false;
	}
	if(playerid == m_lastpid)
	{
		//SLOG_ERROR<<boost::format("%1%:%2% first player can't pass")%__FUNCTION__%__LINE__;
        SLOG_CRITICAL << k_logflag << "last discard player is self, playerid : " << playerid << ", can`t pass";
		return false;
	}
	// 

	// clear
	auto it_out = m_player_out_pokers.find(playerid);
	if(it_out != m_player_out_pokers.end())
	{
		auto& poker = it_out->second;
		poker.clear();
	}
	if(m_lastpid == m_nFirstID)
	{
		//m_style = STYLE_NO;
		//m_val = 0;
		//m_len = 0;
	}
    //当前玩家过牌，轮到下一个玩家;
    int next_cid = m_analyser.Pass(user.p_idx);
    if (next_cid < 0)
    {
        SLOG_CRITICAL << k_logflag << "playerid: " << playerid << ", seatid: "<< user.p_idx << ", Pass error";
        return false;
    }

    m_nextpid = GetNextPid(m_curpid);

    defaultPass(m_curpid, m_nextpid);
    if (IsRobot(m_nextpid))
    {
        m_duration = GetRobotTimes(eGameState_Play);
        //m_duration = global_random::instance().rand_int(2, 3) + TIME_LESS;
    }
    else
    {
		auto itr = m_MapTablePlayer.find(m_nextpid);
		if (itr->second.p_isauto)
		{
			static int trustee_time = Landlord3_BaseInfo::GetSingleton()->GetData("TrusteeOperaTime")->mValue;
			m_duration = trustee_time;
		}
		else
		{
			m_duration = TIME_LESS + m_OperaTime;
		}
    }

	return true;
}

int64_t GetFarmerScore(std::map<uint32_t, TablePlayer>& playerMap, uint32_t landlordpid, int64_t close_score, bool flag = true)
{
    int64_t sum = 0;
    int64_t lord_score = 0;
    for (auto& item : playerMap)
    {
        auto& user = item.second;
        LPlayerPtr& player = user.p_playerptr;
        if (/*user.p_active == false || */player == nullptr)
        {
            continue;
        }

        int64_t u_gold = player->get_gold();
        auto uid = user.p_id;
        if (uid != landlordpid)
        {
            if (u_gold > close_score)
            {
                sum += close_score;
                
                if (flag)
                {
                    user.p_result = -close_score;
                }
                else
                {
                    user.p_result = close_score;
                }
            }
            else
            {
                sum += u_gold;
                if (flag)
                {
                    user.p_result = -u_gold;
                }
                else
                {
                    user.p_result = u_gold;
                }
            }
        }
    }

    return sum;
}

void SetFarmerWinScore(std::map<uint32_t, TablePlayer>& playerMap, uint32_t landlordpid, int64_t landlord_score, int64_t close_score)
{
    for (auto& item : playerMap)
    {
        auto& user = item.second;
        LPlayerPtr& player = user.p_playerptr;
        if (user.p_active == false || player == nullptr)
        {
            continue;
        }

        int64_t u_gold = player->get_gold();
        auto uid = user.p_id;
        if (uid != landlordpid)
        {
            int64_t u_score0 = u_gold;
            if (u_score0 > close_score)
            {
                u_score0 = close_score;
            }
   
            user.p_result = landlord_score * u_score0 /(u_score0 + close_score);
        }
    }
}

bool CheckPlayerScore2(std::map<uint32_t, TablePlayer>& playerMap, uint32_t lastpid, uint32_t landlordpid, int64_t close_score)
{
    //判断地主身上钱;
    auto banker_it = playerMap.find(landlordpid);
    if (banker_it == playerMap.end())
    {
        return false;
    }
    auto& lord_user = banker_it->second;
    LPlayerPtr lord_player = lord_user.p_playerptr;
    int64_t landlord_gold = lord_player->get_gold();
    if (lastpid == landlordpid)// 地主赢;
    {
        int scores = GetFarmerScore(playerMap, landlordpid, close_score);
        if (landlord_gold >= scores)
        {
            lord_user.p_result = scores;
        }
        else
        {
            int temp_close = landlord_gold / 2;
            int scores = GetFarmerScore(playerMap, landlordpid, temp_close);
            lord_user.p_result = scores;
        }
    }
    else// 地主输;
    {
        int scores = GetFarmerScore(playerMap, landlordpid, close_score, false);
        lord_user.p_result = -scores;
        if (scores > landlord_gold)
        {
            lord_user.p_result = -landlord_gold;
            SetFarmerWinScore(playerMap, landlordpid, landlord_gold, close_score);
        }
    }

    return true;
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
		m_bomCounter++;
	}
    else
    {
        char outs = 0, zouts = 0;
        bool isauto = false;
        for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
        {
            auto user = it->second;
            LPlayerPtr& player = user.p_playerptr;
            if (/*user.p_active == false || */player == nullptr) continue;
            auto uid = user.p_id;
            if (uid == m_landlordpid)
            {
                zouts += user.p_outs;
            }
            else
            {
                outs += user.p_outs;
            }
            if (user.p_isauto && user.p_islast1) isauto = true;
        }
        //  春天;
        m_chun = GAME_CHUN_NO;
        if (outs == 0)
        {
            m_nGameRate *= 2;
            m_chun = GAME_CHUN_YES;
        }
        else if (zouts == 1)
        {
            m_nGameRate *= 2;
            m_chun = GAME_CHUN_FAN;
        }

        if (m_chun != GAME_CHUN_NO)
            money *= 2;

        int64_t zmoney = 0;
        if (money == 0)
        {
            SLOG_CRITICAL << format("[!!!!!!!!!!!!!]");
            money = base * 3;
        }

        //判断地主身上钱;
        CheckPlayerScore2(m_MapTablePlayer, playerid, m_landlordpid, money);
    }
	//复盘日志;
	m_landlordlog->set_roomscore(base);
	m_landlordlog->set_bankerrate(m_bankerRate);
	m_landlordlog->set_gamerate(m_nGameRate);
	m_landlordlog->set_bombcount(m_bomCounter);

	// write
	auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_game_result,landlord3_protocols::e_mst_l2c_game_result);
	sendmsg->mutable_resultinfo()->Reserve(GAME_PLAYER);
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{
		auto user = it->second;
		LPlayerPtr& player = user.p_playerptr;
		if(/*user.p_active==false || */player==nullptr) continue;
        
        sendmsg->set_winid(playerid);
        sendmsg->set_robrate(m_bankerRate);
        sendmsg->set_gamerate(m_nGameRate);
        sendmsg->set_bombcount(m_bomCounter);
        sendmsg->set_bombrare(2);
        sendmsg->set_ischun(m_chun);

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

        //复盘日志;
        auto pinfo = m_landlordlog->add_pinfo();
        pinfo->set_pid(uid);
        pinfo->set_goldbegin(user.p_begingold);
        pinfo->set_goldend(player->get_gold());
        pinfo->set_vargold(resultgold);
        //pinfo->set_luckvalue(pl->get_lucky());
        pinfo->set_isrobot(player->is_robot());
		pinfo->set_seatid(user.p_idx);
        pinfo->set_commission(nTax);
        pinfo->set_isbanker(playerid == uid ? true : false);
	}
	broadcast_msg_to_client2(sendmsg);
	return true;
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

bool logic_table::is_full()
{
    return m_player_count >= GAME_PLAYER;
}

bool landlord_space::logic_table::is_opentable()
{
    return m_player_count >= 1;
}

LPlayerPtr& logic_table::get_player_byid(uint32_t pid)
{
    auto it = m_MapTablePlayer.find(pid);
    if (it != m_MapTablePlayer.end())
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
    if (it != m_MapTablePlayer.end())
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
        if (user.second.p_idx == seat)
        {
            uid = user.second.p_id;
            break;
        }
    }
    return uid;
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

int32_t logic_table::get_player_size()
{
    int32_t nCount = 0;
    for (auto& item : m_MapTablePlayer)
    {
        LPlayerPtr& player = item.second.p_playerptr;
        if (player && !player->is_robot())
            nCount++;
    }
    return nCount;
}

bool logic_table::can_enter_table(LPlayerPtr& player)
{
	int size = get_player_size();
	auto stock = player->get_stock_factor();
	if (m_robot_switch && stock.second == 1)
	{
		auto maxcounter = m_room->get_roomcfg()->mPlayerMaxCounter;
		if (size < maxcounter)
		{
			return true;
		}
		return false;
	}
	else
	{
		if (size < 3)
		{
			return true;
		}
		return false;
	}
}

void logic_table::CleanOutPlayer()
{
    std::map<uint32_t, TablePlayer>  tMapTablePlayer = m_MapTablePlayer;
    for (auto user : tMapTablePlayer)
    {
        auto player = user.second.p_playerptr;
        uint32_t playerid = user.second.p_id;
        int32_t noaction = user.second.p_noaction;
        int64_t lGold = user.second.p_asset;
        if (player && !player->is_robot())
        {// e_msg_cleanout_def
         // wait
            int32_t nReason = 0;
            if (player->get_wait() >= 3)
            {
                nReason = 3;
            }
            else if (noaction >= 2)
            {
                nReason = 4;
            }
            else if (lGold < m_room->get_data()->mGoldMinCondition)
            {
                nReason = 5;
            }

            if (nReason > 0)
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

void logic_table::SendCardMessage(LPlayerPtr player, VECPOKER& poker)
{
    int64_t base = m_room->get_data()->mBaseCondition; // 底注;
    auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_game_start, landlord3_protocols::e_mst_l2c_game_start);
    sendmsg->set_basegold(base);
    // state
    for (auto it2 = m_MapTablePlayer.begin(); it2 != m_MapTablePlayer.end(); it2++)
    {
        auto& user = it2->second;
        sendmsg->add_state(user.p_active);
        sendmsg->add_stateid(user.p_id);
    }

    // card info
    auto cardinfo = sendmsg->mutable_cardinfo();
    if (player->is_robot())
    {
        robot_mgr::instance().recv_packet(player->get_pid(), sendmsg->packet_id(), sendmsg);
    }
    else
    {
        //给用户发牌;
        for (auto& item : poker)
        {
            auto pokerinfo = cardinfo->add_pokers();
            pokerinfo->set_value(item.value);
            pokerinfo->set_style(item.style);
        }
        auto pid = player->get_pid();
        cardinfo->set_pokertype(0);
        cardinfo->clear_pokerstatus();
        player->send_msg_to_client(sendmsg);
    }
}

int logic_table::broadcast_msg_to_client(std::vector<uint32_t>& pids, uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg)
{
    //	printf("broadcast_msg_to_client: %d:%d,,%d\n",pids.size(),pids[pids.size()-1],packet_id);
    return game_engine::instance().get_handler()->broadcast_msg_to_client(pids, packet_id, msg);
}

void logic_table::bc_enter_seat(int seat_index, LPlayerPtr& player)
{
    //  notice other
    auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_user_enter_seat, landlord3_protocols::e_mst_l2c_user_enter_seat);
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
	playerinfo->set_player_region(*player->getIGamePlayer()->GetUserRegion());

    broadcast_msg_to_client2(sendmsg);
}

void logic_table::bc_leave_seat(uint32_t player_id)
{
    auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_user_leave_seat, landlord3_protocols::e_mst_l2c_user_leave_seat);
    sendmsg->set_playerid(player_id);
    broadcast_msg_to_client2(sendmsg);
}

void logic_table::service_ctrl(int32_t optype)
{
    m_service_status = optype;
}

void logic_table::robot_ctrl(bool on_off)
{
	m_robot_switch = on_off;
	m_robot_cfg->robot_ctrl(on_off);
	if (m_robot_switch)
	{
		m_tb_palyer_size = m_room->get_roomcfg()->mPlayerMaxCounter;
	}
	else
	{
		m_tb_palyer_size = 3;
	}
}

bool logic_table::check_player_stock()
{
	LPlayerPtr p_ptr = nullptr;
	int p_count = 0;
	int r_count = 0;
	for (auto& it : m_MapTablePlayer)
	{
		auto ptr = it.second.p_playerptr;
		if (ptr)
		{
			if (ptr->is_robot())
			{
				++r_count;
			}
			else
			{
				++p_count;
				p_ptr = ptr;
			}
		}
	}
	if (p_count == 1)
	{
		p_ptr->load_player_stock();
		auto stock = p_ptr->get_stock_factor();
		m_player_stock_factor = stock;
		if (stock.second == 1)
		{
			return true;
		}
		else
		{
			for (auto& it : m_MapTablePlayer)
			{
				auto ptr = it.second.p_playerptr;
				if (ptr)
				{
					if (ptr->is_robot())
					{
						ptr->set_leave_status(1);
					}
				}
			}
		}
	}
	return false;
}

void logic_table::kill_points(int32_t cutRound, bool status)
{
	m_kill_points_switch = status;
	if (m_kill_points_switch)
	{
		m_gametimes = abs(cutRound);
		m_cfgCutRound = cutRound;
	}
	else
	{
		m_gametimes = 0;
		m_cfgCutRound = 3;
	}
}

int logic_table::get_cut_round_tag()
{
	return m_cutround_tag;
}

bool logic_table::cut_round_check(int cut_round)
{
	if (cut_round != 0)
	{
		auto round = abs(cut_round);
		if (m_gametimes >= round)
		{
			m_gametimes = 0;
			m_cuttimes = 0;
		}
		if (m_cuttimes == 0)
		{
			m_cuttimes = global_random::instance().rand_int(1, round);;
		}
		if (++m_gametimes == m_cuttimes)
		{
			return true;
		}
	}
	return false;
}

void landlord_space::logic_table::server_stop()
{
    auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_clean_out, landlord3_protocols::e_mst_l2c_clean_out);
    if (sendmsg)
    {
        sendmsg->set_reason(landlord3_protocols::e_cleanout_servicestop);
		
        //m_room->broadcast_msg_to_client(sendmsg);
    }
	for (auto& item : m_MapTablePlayer)
	{
		auto ptr = item.second.p_playerptr;
		if (ptr)
		{
			auto pl = ptr->getIGamePlayer();
			if (pl)
			{
				sendmsg->set_sync_gold(ptr->get_gold());
				pl->send_msg_to_client(sendmsg);
				SLOG_CRITICAL << "server_stop reqPlayer_leaveGame player id: " << pl->get_playerid();
				pl->reqPlayer_leaveGame();
			}
		}
	}
}

boost::shared_ptr<landlord3_protocols::packetl2c_scene_info_free> logic_table::get_scene_info_msg_free(uint32_t uid)
{
    auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_scene_info_free, landlord3_protocols::e_mst_l2c_scene_info_free);
    auto freeinfo = sendmsg->mutable_freeinfo();
    int8_t nGameState = get_status();
    int32_t nSpareTime = 0;

    freeinfo->set_status(nGameState);
    freeinfo->set_roomid(m_room->get_id());
    freeinfo->set_tableid(get_id());
	int64_t now = GetCurrentStamp64();
	
    for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
    {
        auto &user = it->second;
        auto player = user.p_playerptr;
		if (uid == it->first)
		{
			auto duration = m_ReadyTime -(now - user.p_selftime);
			freeinfo->set_sparetime(duration);
		}

        auto seat = user.p_idx;
        if (player != nullptr)
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

boost::shared_ptr<landlord3_protocols::packetl2c_scene_info_banker> landlord_space::logic_table::get_scene_info_msg_banker(uint32_t uid)
{
    auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_scene_info_banker, landlord3_protocols::e_mst_l2c_scene_info_banker);
    auto playinfo = sendmsg->mutable_playinfo();
    fill_playerinfo(uid, playinfo);

    return sendmsg;
}

boost::shared_ptr<landlord3_protocols::packetl2c_scene_info_fapai> landlord_space::logic_table::get_scene_info_msg_fapai(uint32_t uid)
{
    auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_scene_info_fapai, landlord3_protocols::e_mst_l2c_scene_info_fapai);
    auto playinfo = sendmsg->mutable_playinfo();
    fill_playerinfo(uid, playinfo);
    return sendmsg;
}

boost::shared_ptr<landlord3_protocols::packetl2c_scene_info_play> landlord_space::logic_table::get_scene_info_msg_play(uint32_t uid)
{
    auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_scene_info_play, landlord3_protocols::e_mst_l2c_scene_info_play);
    // 
    auto playinfo = sendmsg->mutable_playinfo();
    fill_playerinfo(uid, playinfo);
    return sendmsg;
}

void landlord_space::logic_table::fill_playerinfo(uint32_t uid, landlord3_protocols::msg_scene_info_play* playinfo)
{
    int8_t nGameState = get_status();
    playinfo->set_status(nGameState);
    int32_t rid = m_room->get_id();
    playinfo->set_roomid(rid);
    playinfo->set_tableid(get_id());
    playinfo->set_bankerid(m_landlordpid);
    playinfo->set_curid(m_curpid);
    playinfo->set_leftcd(m_duration);
    playinfo->set_gamerate(m_nGameRate);
    auto basecard = playinfo->mutable_basecardinfo();
    for (auto& item : m_nBasePoker)
    {
        auto pk = basecard->add_pokers();
        pk->set_style(item.style);
        pk->set_value(item.value);
    }

    for (auto it = m_MapTablePlayer.begin(); it != m_MapTablePlayer.end(); it++)
    {
        auto &user = it->second;
        auto player = user.p_playerptr;
        auto seat = user.p_idx;
        auto usergold = user.p_asset;
        auto userid = user.p_id;
        auto cflag = user.p_cardflag;
        if (player != nullptr)
        {
            landlord3_protocols::msg_player_info* playerinfo = playinfo->add_playerinfo();
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
			//抢庄分数;
            playerinfo->set_robrate(user.p_bankrate);
			playerinfo->set_player_region(*player->getIGamePlayer()->GetUserRegion());
            //最后出的牌;
            auto itr = m_player_out_pokers.find(it->first);
            if (itr != m_player_out_pokers.end())
            {
                auto discard = playerinfo->mutable_discardinfo();
                for (auto& pk : itr->second)
                {
                    auto poker = discard->add_pokers();
                    poker->set_value(pk.value);
                    poker->set_style(pk.style);
                }
            }

            //手牌;
            auto cardinfo = playerinfo->mutable_cardinfo();

            auto itr2 = m_player_pokers.find(it->first);
            if (itr2 != m_player_pokers.end())
            {
                for (auto& pk : itr2->second)
                {
                    auto poker = cardinfo->add_pokers();
                    if (it->first == uid)
                    {
                        poker->set_value(pk.value);
                        poker->set_style(pk.style);
                    }
                    else
                    {
                        poker->set_value(0);
                        poker->set_style(0);
                    }
                }
            }
        }
    }
}

void logic_table::onGameCleanOut(uint32_t playerid, int32_t nReason)
{
    auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_clean_out, landlord3_protocols::e_mst_l2c_clean_out);
    sendmsg->set_reason((landlord3_protocols::e_msg_cleanout_def)nReason);
	
#if 1
    auto player = get_player_byid(playerid);
	sendmsg->set_sync_gold(player->get_gold());
    if (player != nullptr)
    {
        player->send_msg_to_client(sendmsg);
    }
    // 清理;
    auto pl = player->getIGamePlayer();
    if (pl && !pl->is_robot())
    {
		player->reqPlayer_leaveGame();
		SLOG_CRITICAL << "onGameCleanOut reqPlayer_leaveGame player id: " << pl->get_playerid();
    }
#else
    // broadcast
    broadcast_msg_to_client2(sendmsg);
#endif
}

bool logic_table::defaultPass(uint32_t playerid, uint32_t next_pid, bool bTimeOver/*=false*/)
{
    //复盘日志：每个玩家的出牌;
	std::string tm_str = boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time().time_of_day());
    //复盘日志：每个玩家的出牌;
    std::string discard_str = std::to_string(playerid) + " " + tm_str + " " + std::to_string(bTimeOver) + ": - ";
    m_landlordlog->add_discard(discard_str);
    //SLOG_CRITICAL << k_logflag << discard_str;

    auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_pass_result, landlord3_protocols::e_mst_l2c_pass_result);
    sendmsg->set_playerid(playerid);
    //uint32_t nNext = GetNextPlayerByID(playerid, 0);
    sendmsg->set_nextid(next_pid);
    //m_lastpid = nNext;
    broadcast_msg_to_client2(sendmsg);

    m_nFirstID = m_lastpid;
    m_curpid = m_nextpid;
    //m_duration = m_OperaTime + TIME_LESS;
	return true;
}

template<class T>
void landlord_space::logic_table::broadcast_msg_to_client2(T msg)
{
    for (auto user : m_MapTablePlayer)
    {
        LPlayerPtr& player = user.second.p_playerptr;
        if (player != nullptr)
        {
            if (!player->is_robot())
            {
                player->send_msg_to_client(msg);
            }
            else
            {
                // Robot
                robot_mgr::instance().recv_packet(player->get_pid(), msg->packet_id(), msg);
            }
        }
    }
}

bool logic_table::onNoticeJiao(uint32_t playerid)
{
    if (playerid == 0)
    {
        SLOG_ERROR << boost::format("%1%:%2%") % __FUNCTION__%__LINE__;
		set_status(eGameState_Free);
        return false;
    }
    SLOG_CRITICAL << boost::format("%1%:%2%,Rob-id:%3%") % __FUNCTION__%__LINE__%playerid;
    auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_notice_robbanker, landlord3_protocols::e_mst_l2c_notice_robbanker);
    sendmsg->set_playerid(playerid);
    broadcast_msg_to_client2(sendmsg);
    
    return true;
}

void logic_table::onNoticeChangeRate()
{
    auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_bombrate, landlord3_protocols::e_mst_l2c_bombrate);
    sendmsg->set_gamerate(m_nGameRate);

    broadcast_msg_to_client2(sendmsg);
}

bool logic_table::onEventBaseCard(std::vector<POKER>& basePoker)
{
    // get base card m_nBasePoker
    SLOG_CRITICAL << boost::format("onEventBaseCard:Banker:%d,GameRate:%d") % m_landlordpid%m_nGameRate;

    auto sendmsg = PACKET_CREATE(landlord3_protocols::packetl2c_base_card, landlord3_protocols::e_mst_l2c_send_base_card);
    sendmsg->set_playerid(m_landlordpid);
    sendmsg->set_gamerate(m_nGameRate);
    auto cardinfo = sendmsg->mutable_cardinfo();
    int32_t nCount = basePoker.size();
    //cardinfo->mutable_pokers()->Reserve(nCount);

    for (auto& item: basePoker)
    {
        auto pokerinfo = cardinfo->add_pokers();
        pokerinfo->set_value(item.value);
        pokerinfo->set_style(item.style);
    }

    cardinfo->set_pokertype(0);
    cardinfo->clear_pokerstatus();
    broadcast_msg_to_client2(sendmsg);
    return true;
}

void logic_table::ReplayCardLog(int32_t playerid, const std::string& logcard)
{
    //复盘日志：每个玩家的出牌;
    for (auto& item : m_player_pokers)
    {
        std::stringstream sm;
        for (auto& pk : item.second)
        {
            poker_to_stringstream(pk, sm);
        }

        int curid = get_seat_byid(item.first);
        int preid;
        int nextid;
        get_seat(curid, preid, nextid);
        int32_t prepid = get_id_byseat(preid);
        int32_t nextpid = get_id_byseat(nextid);

        //复盘日志;
        std::string cardstr = sm.str();
        auto landlord_cards = m_landlordlog->add_cardsinfo();
        landlord_cards->set_cards_info(cardstr);
        landlord_cards->set_pid(item.first);
        if (item.first == playerid)
        {
            landlord_cards->set_isbanker(1);
        }
        landlord_cards->set_propid(prepid);
        landlord_cards->set_nextpid(nextpid);
    }
}

float landlord_space::logic_table::GetRobotTimes(int8_t nStatus)
{
    auto robotcfg  = Landlord3_RobotGameCFG::GetSingleton()->GetData(m_room->get_id());
    if (robotcfg == nullptr)
    {
        return 0;
    }
    float duration = 0.0;
    switch (nStatus)
    {
    case eGameState_FaPai:
        break;
    case eGameState_Banker:
    {
        int min = robotcfg->mRobotReadyMinTime;
        int max = robotcfg->mRobotReadyMaxTime;
        int time = global_random::instance().rand_int(min, max);
        duration = time / 1000.0;
        break;
    }
    case eGameState_Play:
    {
        int min = robotcfg->mRobotOperaMinTime;
        int max = robotcfg->mRobotOperaMaxTime;
        int time = global_random::instance().rand_int(min, max);
        duration = time / 1000.0;
        break;
    }
        
    case eGameState_End:
        break;
    default:
        break;
    }

	return duration;
}

//////////////////////////////////////////////////////////////////////////

robot_cfg::robot_cfg(logic_room* room, logic_table* table, std::map<uint32_t, TablePlayer>&	players, int16_t tid)
    : m_room(room)
	, m_table(table)
    , m_MapTablePlayer(players)
    , m_tid(tid)
	, m_robot_switch(false)
{
    const boost::unordered_map<int, Landlord3_RobotCFGData>& list = Landlord3_RobotCFG::GetSingleton()->GetMapData();
    for (auto it = list.begin(); it != list.end(); ++it)
    {
        if (it->second.mRoomID == m_room->get_id())
        {
            m_robotcfg = it->second;
            break;
        }
    }
    m_robot_elapsed = global_random::instance().rand_int(m_robotcfg.mRobotMinEntry, m_robotcfg.mRobotMaxEntry);
}

void robot_cfg::robot_heartbeat(double elapsed)
{
	if (m_robotcfg.mIsOpen == 0)
	{
		return;
	}
	if (!m_robot_switch)
	{
		return;
	}

	int minCount = m_robotcfg.mRobotMinCount;
	int maxCount = m_robotcfg.mRobotMaxCount;
	/*static*/ int requireCount = global_random::instance().rand_int(minCount, maxCount);
    
	m_robot_elapsed -= elapsed;
	if (m_robot_elapsed <= 0)
	{
		m_robot_elapsed = global_random::instance().rand_int(m_robotcfg.mRobotMinEntry, m_robotcfg.mRobotMaxEntry);
		
		// 携带金币 超过局数 清理机器人;
		for (auto& user : m_MapTablePlayer)
		{
			auto player = user.second.p_playerptr;
			if (player && player->is_robot())
			{
				int  goldCond = m_room->get_data()->mGoldMinCondition;
				int64_t gold = player->get_gold();

				int32_t nRound = global_random::instance().rand_int(m_robotcfg.mRobotMinRound, m_robotcfg.mRobotMaxRound);
				int round = player->get_round();

				int32_t state = player->get_status();
				if (state != eUserState_play && state != eUserState_dead)
				{
					if (gold < goldCond || round >= nRound)
					{
						release_robot(player->get_pid());
					}
				}
			}
		}

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
		if (all_count >= GAME_PLAYER)
		{
			return;
		}

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
					//只剩机器人 : 暂且不清理机器人;
					release_robot(player->get_pid());
				}
			}
		}
		/*else*/ if (all_count<GAME_PLAYER && robot_count < requireCount)
		{
			if (player_count > 0)
			{
				auto robotsize = robot_mgr::instance().get_robot_count();
				if (robotsize >= m_robotcfg.mRoomRobotMaxCount)
				{
					SLOG_ERROR << "----RobotCounter:" << robotsize <<", config size: "<< m_robotcfg.mRoomRobotMaxCount;
					return;
				}
				if (m_table->check_player_stock())
				{
					request_robot();
				}
			}
		}
	}
}

void robot_cfg::request_robot()
{
	m_room->record_robot(true);
	// Robot
	static int minVIP = m_robotcfg.mRobotMinVip;
	static int maxVIP = m_robotcfg.mRobotMaxVip;
	int32_t vip_level = global_random::instance().rand_int(minVIP, maxVIP);	
	GOLD_TYPE gold_min = m_robotcfg.mRobotMinTake;
	GOLD_TYPE gold_max = m_robotcfg.mRobotMaxTake;

	GOLD_TYPE enter_gold = global_random::instance().rand_int(gold_min,gold_max);
	int32_t rid = m_room->get_id();
	int32_t tid = m_tid;
	//int tag = (rid<<4)|tid;
    int tag = rid + tid * 100;

	SLOG_CRITICAL<<boost::format("request_robot::rid:%1% tid:%2%,tag:%3%")%rid%tid%tag;
	game_engine::instance().request_robot(tag,enter_gold, vip_level);
}

void robot_cfg::release_robot(int32_t playerid)
{
	//m_room->record_robot(false); //在leave_table 在减少记录;
    //SLOG_CRITICAL << boost::format("release_robot:playerid:%1%") % playerid;
	//bc_leave_seat(playerid);
	m_room->get_lobby()->robot_leave(playerid);
	game_engine::instance().release_robot(playerid);
}

int32_t robot_cfg::release_robot_seat()
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

void robot_cfg::reverse_result(uint32_t reqid,uint32_t resid)
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

void robot_cfg::reverse_result()
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
			if(user.p_playing==false) continue;
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

void robot_cfg::robot_switch(uint32_t uid,int nRandom/*=100*/)
{
	
}

int32_t robot_cfg::robot_maxcounter()
{
    auto robot_cfg = Landlord3_RobotCFG::GetSingleton();
    auto roomid = m_room->get_id();

    static int maxCount = robot_cfg->GetData(roomid)->mRobotMaxCount;
    return maxCount;
}

void landlord_space::robot_cfg::robot_ctrl(bool on_off)
{
	m_robot_switch = on_off;
}

int32_t robot_cfg::robot_counter()
{
	int32_t nCount = 0;
	for (auto it = m_MapTablePlayer.begin();it!=m_MapTablePlayer.end();it++)
	{ // 活着的机器人
		auto user = it->second;
		auto player = user.p_playerptr;
		if(user.p_playing==false || player==nullptr) continue;
		if(player->is_robot())
		{
			nCount++;
		}
	}
	return nCount;
}

uint32_t robot_cfg::robot_id(uint32_t uid)
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

int32_t robot_cfg::robot_rate()
{
	int32_t nRate = 0;
	

	return nRate;
}

//////////////////////////////////////////////////////////////////////////
landlord_space::db_table::db_table()
{
    init_game_object();
}

landlord_space::db_table::~db_table()
{
}

void db_table::release()
{
    store_game_object();
}

void db_table::init_game_object()
{
    TableID = CONVERT_POINT(Tfield<int16_t>, regedit_tfield(e_got_int16, "table_id"));
    TableStatus = CONVERT_POINT(Tfield<int8_t>, regedit_tfield(e_got_int8, "table_status"));
}

bool db_table::load_table()
{
    mongo::BSONObj b = db_game::instance().findone(DB_LANDLORD3TABLE, BSON("room_id" << m_roomid << "table_id" << TableID->get_value()));
    if (b.isEmpty())
        return false;

    return from_bson(b);
}

bool db_table::store_game_object(bool to_all)
{
    if (!has_update())
        return true;

    auto err = db_game::instance().update(DB_LANDLORD3TABLE, BSON("room_id" << m_roomid << "table_id" << TableID->get_value()), BSON("$set" << to_bson(to_all)));
    if (!err.empty())
    {
        SLOG_ERROR << "logic_table::store_game_object :" << err;
        return false;
    }

    //m_checksave = 0;

    return true;
}
