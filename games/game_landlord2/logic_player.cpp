#include "stdafx.h"
#include "logic_player.h"
#include "logic_table.h"

#include <i_game_player.h>
#include "game_db.h"
#include "game_db_log.h"
#include "logic_room.h"

#include "robot_player.h"

#include "proc_landlord_logic.h"
#include "proc_landlord_protocol.h"

#include "Landlord3_RoomCFG.h"
#include "Landlord3_BaseInfo.h"
#include "game_engine.h"

#include "msg_type_def.pb.h"
#include "pump_type.pb.h"

#include "Landlord3_PlayerStock.h"

std::pair<int, int> get_fishcfg_factor(int64_t loss_gold, std::vector<int>& mParam, int top_up, bool& back_bander_flag, int& times)
{
	std::pair<int, int> probability;
    //int probability = 0;
    int sectionSize = mParam.size() / 4; //区间段;
    if (mParam.size() % 4 == 0)
    {
        int index = 0;
        for (int i = 0; i < sectionSize; ++i)
        {
            int64_t down_border = mParam[index] < mParam[index + 1] ? mParam[index] : mParam[index + 1];
            int64_t up_border = mParam[index] < mParam[index + 1] ? mParam[index + 1] : mParam[index];

            bool is_start_border = (mParam[index] == mParam[index + 1] && i == 0);
            bool is_end_border = (mParam[index] == mParam[index + 1] && i == sectionSize - 1);

            if (is_start_border)
            {
                if (loss_gold <= down_border)
                {
					probability = std::make_pair(mParam[index + 2], mParam[index +3]);
					return probability;
                }
            }
            else if (is_end_border)
            {
                if (top_up == 0 && i == sectionSize - 1 && back_bander_flag == true)
                {
                    ++times;
                }
                back_bander_flag = false;
                if (loss_gold > up_border)
                {
					probability = std::make_pair(mParam[index + 2], mParam[index + 3]);
					return probability;
                }
            }
            else
            {
                if (loss_gold > down_border && loss_gold <= up_border)
                {
                    back_bander_flag = true;
					probability = std::make_pair(mParam[index + 2], mParam[index + 3]);
					return probability;
                }
            }
            index += 4;
        }
    }
    return probability;
}

int no_top_up_range(std::map<int, std::pair<int64_t, int64_t>>& topUpRangMap, int times)
{
    int index = 0;
    std::vector<int> ranges;
    for (auto& item : topUpRangMap)
    {
        if (item.second.first == item.second.second && item.second.first == 0)
        {
            ranges.push_back(item.first);
        }
    }
    int size = ranges.size();
    if (times < 0 && size > 0)
    {
        index = ranges[0];
    }
    else if (times < size)
    {
        index = ranges[times];
    }
    else if (times >= size)
    {
        index = ranges[size - 1];
    }

    return index;
}

int top_up_range(std::map<int, std::pair<int64_t, int64_t>>& topUpRangMap, int64_t top_up_gold)
{
    int index = 0;
    int i = 0;
    for (auto& item : topUpRangMap)
    {
        if (i == 0)
        {
            if (top_up_gold <= item.second.first)
            {
                index = item.first;
                break;
            }
        }
        else if (i == topUpRangMap.size() - 1)
        {
            if (top_up_gold > item.second.second)
            {
                index = item.first;
                break;
            }
        }
        else
        {
            if (top_up_gold > item.second.first && top_up_gold <= item.second.second)
            {
                index = item.first;
                break;
            }
        }
        ++i;
    }
    return index;
}


LANDLORD_SPACE_USING

landlord_space::logic_player::logic_player(void)
:m_lobby(nullptr)
,m_table(nullptr)
, m_room(nullptr)
,m_logic_gold(0)
,m_change_gold(0)
, m_bforce(0)
, m_recharge_gold(0)
, m_cut_round_flag(false)
, m_leave_status(0)
{
	init_game_object();

	m_nChairID->set_value(INVALID_CHAIR);
	m_nPlayerStatus->set_value(eUserState_null);
	m_nPlayRound->set_value(0);
	m_nWaitRound->set_value(0);
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

void logic_player::quest_change_from_world(int quest_type,int count,int param)
{

}

void logic_player::on_change_state()
{
	// TODO
}

int logic_player::cltReq_leaveGame()
{
	if (can_leave_table())
	{
		SLOG_CRITICAL << "cltReq_leaveGame reqPlayer_leaveGame player id: " << get_pid();
		getIGamePlayer()->reqPlayer_leaveGame(0);
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
	// TODO 超时踢人
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

	tagStaticOp* opt = getIGamePlayer()->GetOpItems();

	if (opt)
	{
		m_recharge_gold = opt->RecvGiftCoinTotal + opt->RechargeCoinTotal;
		if (m_recharge_gold <= 0)
		{
			m_nTotalInScore->set_value(get_gold() + opt->safeBag);
		}
	}
	load_player_stock();
}

bool logic_player::join_table(logic_table* table,int32_t seat/*=0*/)
{
	// 入桌
	m_table = table;
	// 设置用户椅子、状态
	m_nChairID->set_value(seat);

	m_nPlayRound->set_value(0);
	m_nWaitRound->set_value(0);

    if (!is_robot())
    {
        tagStaticOp* opt = getIGamePlayer()->GetOpItems();

        if (opt)
        {
            m_recharge_gold = opt->RecvGiftCoinTotal + opt->RechargeCoinTotal;
			if (m_recharge_gold <= 0)
			{
				m_nTotalInScore->set_value(get_gold() + opt->safeBag);
			}
        }
    }
	return true;
}

int32_t logic_player::get_seat() const
{
	return m_nChairID->get_value();
}

void logic_player::set_status(int state)
{
	m_nPlayerStatus->set_value(state);
}

int32_t logic_player::get_status()
{
	return m_nPlayerStatus->get_value();
}


void landlord_space::logic_player::set_wait(int wait)
{
	m_nWaitRound->set_value(wait);
}

void landlord_space::logic_player::add_wait(int wait/*=1*/)
{
	m_nWaitRound->add_value(wait);
}

int32_t landlord_space::logic_player::get_wait()
{
	return m_nWaitRound->get_value();
}

void landlord_space::logic_player::add_round(int round/*=1*/)
{
	m_nPlayRound->add_value(round);
}

void landlord_space::logic_player::clear_round(int round/*=0*/)
{
	m_nPlayRound->set_value(round);
}

int32_t landlord_space::logic_player::get_round()
{
	return m_nPlayRound->get_value();
}

logic_table* logic_player::get_table()
{
	return m_table;
}


void logic_player::leave_table()
{
	// TODO 用户离开桌子
	//SLOG_DEBUG<<"logic_player::leave_table >>>>";
	//
	if(m_table != nullptr)
    {
		m_table->leave_table(get_pid());
		m_table = nullptr;
		m_nChairID->set_value(INVALID_CHAIR);
		set_status(eUserState_null);
		clear_round();
		set_wait(0);
	}
}

bool logic_player::can_leave_table()
{
	if(m_table==nullptr) return true;

	return m_table->can_leave_table(get_pid());
}

void logic_player::release()
{
	leave_table();
	m_player.reset();
}


uint32_t logic_player::get_pid()
{
	return m_player->get_playerid();
}

GOLD_TYPE logic_player::get_gold()
{
	return m_player->get_attribute64(msg_type_def::e_itd_gold);
//	return m_logic_gold;
}

GOLD_TYPE landlord_space::logic_player::get_pre_gold()
{
    return m_pre_logic_gold;
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
	if(m_logic_gold + v >= 0)
	{
		if (m_logic_gold + v > MAX_MONEY)
			v = MAX_MONEY - m_logic_gold;
		if(v==0) return false;

		m_change_gold += v;
		m_logic_gold += v;
		if(needbc && m_table!=nullptr)
		{
			// TODO
		}
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
		if (m_table)
		{
			cut_tag = m_table->get_cut_round_tag();
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
		if (m_table)
		{
			cut_tag = m_table->get_cut_round_tag();
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

void logic_player::sync_change_gold()
{
	if(m_change_gold != 0)
	{
		if (!is_robot())
		{
			if (m_change_gold > 0)
			{
				add_star_lottery_info(0, 1);

				quest_change(win_gold, m_change_gold);

				//广播
				static int broadcast_gold = Landlord3_BaseInfo::GetSingleton()->GetData("BroadcastGold")->mValue;
				if (m_change_gold >= broadcast_gold)
				{
					const std::string *pStr = m_player->getLan("CowsBroadcast");
					if(pStr)
					{
						boost::format fmt = boost::format(pStr->c_str()) 
							% get_viplvl()
							% get_nickname() 
							% m_change_gold;
						m_player->gameBroadcast(fmt.str());
					}
				}
			}
		}
		if(m_player->change_gold(m_change_gold))
		{
			if (!is_robot())
			{
				int cut_tag = 0;
				if (m_table)
				{
					cut_tag = m_table->get_cut_round_tag();
				}
				//记录
				db_log::instance().property_log(this, game_engine::instance().get_gameid(), 
					1,//e_item_type_def::e_itd_gold
					m_change_gold,0, cut_tag,
					11//PropertyReasonType::type_reason_single_round_balance
					);
			}
			m_change_gold = 0;		
		}
	}	
	store_game_object();
}

void landlord_space::logic_player::write_property(int64_t value,int64_t tax /*= 0*/,const std::string& param/*=""*/)
{
    m_nTotalInScore->add_value(value);
	if(value==0) return;
	if(m_player->change_gold(value,tax))
	{
		int cut_tag = 0;
		if (m_table)
		{
			cut_tag = m_table->get_cut_round_tag();
		}

		db_log::instance().property_log(this, game_engine::instance().get_gameid(), 
				msg_type_def::e_item_type_def::e_itd_gold,
				value, tax, cut_tag,
				PropertyReasonType::type_reason_single_round_balance,
				param);
			
	}
	load_player_stock();
	store_game_object();

	if (is_robot())
	{
		static int robot_gold = Landlord3_BaseInfo::GetSingleton()->GetData("RobotBroadcastGold")->mValue;
		if (value < robot_gold) return;
		static int robot_rate = Landlord3_BaseInfo::GetSingleton()->GetData("RobotBroadcastRate")->mValue;
		if (global_random::instance().rand_1w() > robot_rate) return;

	}
	else
	{
		static int broadcast_gold = Landlord3_BaseInfo::GetSingleton()->GetData("BroadcastGold")->mValue;
		if (value < broadcast_gold) return;
		static int broadcast_rate = Landlord3_BaseInfo::GetSingleton()->GetData("BroadcastRate")->mValue;
		if (global_random::instance().rand_1w() > broadcast_rate) return;

	}
	//
	const std::string *pStr = m_player->getLan("N_WinPrize4Others");
	if (pStr)
	{
		std::string strRoomNameInfo;
		if (m_table)
		{
			auto room = m_table->get_room();
			if (room) {
				strRoomNameInfo = room->get_roomcfg()->mRoomName;
			}
		}
		boost::format fmt;
		auto remon = value / 100.0;
		fmt = boost::format(pStr->c_str()) % get_nickname() % strRoomNameInfo.c_str() % remon;
		m_player->gameBroadcast(fmt.str());
	}
	//重新检查当前玩家输赢区间;
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

const std::string& logic_player::GetUserRegion()
{
	std::string *pStr = m_player->GetUserRegion();
	return *pStr;
}

// logic_room* logic_player::get_room()
// {
// 	return m_room;
// }

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
	if(is_robot() && m_robot==nullptr)
	{
		m_robot = robot_player::malloc();
	//	m_robot->init(this);
	}
}

//////////////////////////////////////////////////////////////////////////
void logic_player::create_player()
{
	m_win_count->set_value(0);

	store_game_object(true);
}

bool logic_player::load_player()
{
	mongo::BSONObj b = db_game::instance().findone(DB_LANDLORD_PLAYER, BSON("player_id" << get_pid()));
	if(b.isEmpty())
		return false;

	return from_bson(b);
}

void logic_player::init_game_object()
{	
    m_nTotalInScore = CONVERT_POINT(Tfield<int32_t>, regedit_tfield(e_got_int32, "total_score"));

	m_win_count = CONVERT_POINT(Tfield<int32_t>, regedit_tfield(e_got_int32, "win_count"));

	m_nChairID = CONVERT_POINT(Tfield<int32_t>,regedit_tfield(e_got_int32,"chair_id"));

	m_nPlayerStatus = CONVERT_POINT(Tfield<int32_t>,regedit_tfield(e_got_int32,"player_state"));

	m_nPlayRound = CONVERT_POINT(Tfield<int32_t>,regedit_tfield(e_got_int32,"player_round"));

	m_nWaitRound = CONVERT_POINT(Tfield<int32_t>,regedit_tfield(e_got_int32,"wait_round"));
}

bool logic_player::store_game_object(bool to_all)
{
	if(!has_update())
		return true;

	auto err = db_game::instance().update(DB_LANDLORD_PLAYER, BSON("player_id" << get_pid()), BSON("$set"<<to_bson(to_all)));
	if(!err.empty())
	{
		SLOG_ERROR << "logic_player::store_game_object :" <<err;
		return false;
	}
	return true;
}

void logic_player::bc_game_msg(int money, const std::string& sinfo, int mtype)
{
	if(m_table)
	{
		auto rcfg = m_table->get_room()->get_data();
		m_player->game_broadcast(rcfg->mRoomName, 1, sinfo, money, mtype);
	}
}


void logic_player::add_star_lottery_info(int32_t award,int32_t star /*= 0*/)
{
	return;
	m_player->add_starinfo(award);

	m_win_count->add_value(star);
	static int WinStarCount = Landlord3_BaseInfo::GetSingleton()->GetData("WinStarCount")->mValue;
	if (m_win_count->get_value() > WinStarCount)
	{
		m_win_count->set_value(0);
		m_player->add_starinfo(0,1);
	}
}

void logic_player::quest_change(int questid, int count, int param)
{
	if (!is_robot())
	{
		m_player->quest_change(questid, count, param);
	}
}

void logic_player::reqPlayer_leaveGame()
{
	SLOG_CRITICAL << "logic_player reqPlayer_leaveGame player id: " << get_pid() << m_leave_status;
	if (m_leave_status != 0)
	{
		m_player->reqPlayer_leaveGame(m_leave_status);
	}
	else
	{
		m_player->reqPlayer_leaveGame(0);
	}
}

std::pair<int, int> logic_player::get_stock_factor()
{
	return m_stock_factor;
}


bool logic_player::get_cut_round_flag()
{
	return m_cut_round_flag;
}

void logic_player::load_player_stock()
{
	int64_t loss_gold = get_gold() + getIGamePlayer()->GetOpItems()->safeBag;
	int over_time = 0;
	bool back_bonder_flag = 0;

	auto& fileStockMap = Landlord3_PlayerStock::GetSingleton()->GetMapData();
	std::map<int, std::pair<int64_t, int64_t> > topUpRangSet;
	std::map<int, int> cut_rounds;
	for (auto item : fileStockMap)
	{
		auto& topUpRang = item.second.mTopUpRang;
		if (topUpRang.size() == 2)
		{
			int64_t topUpMin = topUpRang[0];
			int64_t topUpMax = topUpRang[1];
			topUpRangSet[item.first] = std::make_pair(topUpMin, topUpMax);
			cut_rounds[item.first] = item.second.mCutRoundFlag;
		}
	}
	//判断充值记录所在范围;
	int index = 0;
	if (m_recharge_gold > 0)
	{
		index = top_up_range(topUpRangSet, m_recharge_gold);
	}
	else
	{
		index = no_top_up_range(topUpRangSet, over_time);
	}
	m_cut_round_flag = cut_rounds[index];
	//获取系数;
	auto itemIndex = fileStockMap.find(index);
	if (itemIndex != fileStockMap.end())
	{
		auto& paramVec = itemIndex->second.mParam;
		m_stock_factor = get_fishcfg_factor(loss_gold, paramVec, m_recharge_gold, back_bonder_flag, over_time);
		//m_over_stock_times->set_value(over_time);
		//m_back_bander_flag->set_value(back_bonder_flag);
	}

	//     SLOG_CRITICAL << "playerID: " << this->get_pid() << ", top up gold: " << top_up_gold << " ,total income: " << KillFishScore->get_value()
	//         << ", total outlay:" << TotalPlayerOutlay->get_value() << ", stock gold: " << loss_gold << ", stock factor: " << factor << ", over stock times: " << over_time;
}

int logic_player::get_roomid()
{
	if (m_table)
	{
		if (m_table->get_room())
		{
			return m_table->get_room()->get_id();
		}
	}
	return 0;
}

int landlord_space::logic_player::send_packet(uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg)
{
	__ENTER_FUNCTION
	auto factory = packet_manager::instance().get_factroy(packet_id);
	if(factory != nullptr)
	{			
		factory->packet_process(nullptr, m_player, msg);
	}			
	return 0;
	__LEAVE_FUNCTION
	return -1;
}


bool logic_player::onEventUserReady()
{
// 	if(get_status()>eUserState_free)
// 	{
// 		SLOG_CRITICAL<<"logic_player::onEventUserReady Can't Ready:usestate";
// 		return false;
// 	}
// 	if(m_table && m_table->get_status()!=eGameState_Free)
// 	{
// 		SLOG_CRITICAL<<"logic_player::onEventUserReady Can't Ready:gamestate";
// 		return false;
// 
// 	}

	return m_table->onEventUserReady(get_pid());
}