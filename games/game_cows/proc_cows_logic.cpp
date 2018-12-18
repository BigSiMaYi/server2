#include "stdafx.h"
#include "proc_cows_logic.h"
#include "proc_cows_protocol.h"
#include <i_game_player.h>
#include "logic_player.h"
#include "logic_room.h"
#include "logic_main.h"
#include "logic_banker.h"
#include "logic_other.h"
#include "logic_cards.h"
#include "game_engine.h"

COWS_SPACE_USING
using namespace boost;

void init_proc_cows_logic()
{
	packetc2l_get_scene_info_factory::regedit_factory();
	packetl2c_get_scene_info_result_factory::regedit_factory();

	packetc2l_ask_bet_info_factory::regedit_factory();
	packetl2c_bet_info_result_factory::regedit_factory();

	packetc2l_ask_apply_banker_factory::regedit_factory();
	packetl2c_apply_banker_result_factory::regedit_factory();

	packetc2l_ask_cancel_apply_banker_factory::regedit_factory();
	packetl2c_cancel_apply_banker_result_factory::regedit_factory();

	packetc2l_ask_leave_banker_factory::regedit_factory();
	packetl2c_leave_banker_result_factory::regedit_factory();

	packetc2l_ask_bankerlist_factory::regedit_factory();
	packetl2c_bankerlist_result_factory::regedit_factory();

	packetc2l_ask_playerlist_factory::regedit_factory();
	packetl2c_playerlist_result_factory::regedit_factory();

	packetc2l_ask_snatch_banker_factory::regedit_factory();
	packetl2c_snatch_banker_result_factory::regedit_factory();

	packetc2l_ask_history_info_factory::regedit_factory();
	packetl2c_history_info_factory::regedit_factory();

	packetl2c_banker_info_factory::regedit_factory();
	packetl2c_banker_success_factory::regedit_factory();

	packetc2l_ask_continue_bet_factory::regedit_factory();
	packetl2c_continue_bet_result_factory::regedit_factory();
	packetc2l_ask_clear_bet_factory::regedit_factory();
	packetl2c_clear_bet_result_factory::regedit_factory();

	packetl2c_bc_scene_prepare_into_factory::regedit_factory();
	packetl2c_bc_scene_bet_into_factory::regedit_factory();
	packetl2c_bc_sync_scene_bet_into_factory::regedit_factory();
	packetl2c_bc_scene_result_into_factory::regedit_factory();
	packetl2c_bc_scene_deal_into_factory::regedit_factory();
	packetl2c_bc_snatch_banker_factory::regedit_factory();
	packetl2c_kick_player_factory::regedit_factory();
}

//获取游戏信息
bool packetc2l_get_scene_info_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
													 shared_ptr<packetc2l_get_scene_info> msg)
{	
	__ENTER_FUNCTION_CHECK;
	
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if (lcplayer->get_room() != nullptr)
	{
		//检查系统状态
		auto server_status = lcplayer->get_room()->get_server_status();
		if (server_status==100)
		{
			auto sendmsg = PACKET_CREATE(packetl2c_kick_player, e_mst_l2c_kick_player);
			if (sendmsg)
			{
				sendmsg->set_reason(e_msg_kick_player::e_cleanout_servicestop);
				player->send_msg_to_client(sendmsg);
			}
			return !EX_CHECK;
		}
	}
	if (lcplayer != nullptr && lcplayer->get_room() == nullptr)
	{
		game_engine::instance().get_lobby().enter_room_nocheck(lcplayer->get_pid(), 1);
	}

	if(lcplayer && lcplayer->get_room())
	{
		lcplayer->set_kick_status(0);
		auto sendmsg = PACKET_CREATE(packetl2c_get_scene_info_result, e_mst_l2c_get_scene_info_result);
		auto scene_info = sendmsg->mutable_scene_info();
		scene_info->set_roomid(lcplayer->get_room()->get_id());
		auto game_main = lcplayer->get_room()->get_game_main();
		scene_info->set_scene_state((int)(game_main->get_game_state()));
		scene_info->set_count_down(game_main->get_cd_time());
		scene_info->set_main_id(game_main->get_inc_id());

		int state = (int)(game_main->get_game_state());
		if (state >= (int)logic_main::game_state::game_state_prepare)
		{
			auto banker_info = scene_info->mutable_banker_info();
			auto player_info = banker_info->mutable_player_info();
			game_main->get_banker()->fill_player_info(player_info);
			banker_info->set_max_bet_gold(game_main->get_banker()->get_max_bet_gold());
			banker_info->set_can_snatch(game_main->can_snatch());
			banker_info->set_snatch_gold(game_main->get_snatch_gold());
			banker_info->set_snatch_player_id(game_main->get_snatch_player());
			player_info->set_player_region(game_main->get_banker()->getBankerRegion());
		}
		if (state >= (int)logic_main::game_state::game_state_bet)
		{
			auto bet_info = scene_info->mutable_bet_info();
			auto& other = game_main->get_others();
			for (unsigned int i = 0; i < other.size(); i++)
			{
				bet_info->add_self_bet_golds(other[i]->get_bet_gold(lcplayer->get_pid()));
				bet_info->add_total_bet_golds(other[i]->get_total_bet_gold());
			}
		}
		if (state >= (int)logic_main::game_state::game_state_deal)
		{
			auto cards_info = scene_info->add_cards_infos();
			game_main->get_banker()->get_cards()->fill_cards_info(cards_info);

			auto& other = game_main->get_others();
			for (unsigned int i = 0; i < other.size(); i++)
			{
				auto cards_info = scene_info->add_cards_infos();
				other[i]->get_cards()->fill_cards_info(cards_info);
			}
		}
		if (state >= (int)logic_main::game_state::game_state_result)
		{
			auto result_info = scene_info->mutable_result_info();
			game_main->fill_result_info(lcplayer, result_info);
		}

		player->send_msg_to_client(sendmsg);
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//请求下注
bool packetc2l_ask_bet_info_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
													  shared_ptr<packetc2l_ask_bet_info> msg)
{	
	__ENTER_FUNCTION_CHECK;
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(lcplayer && lcplayer->get_room())
	{
		int ret = lcplayer->get_room()->get_game_main()->bet_gold(lcplayer, msg->bet_index()-1, msg->bet_gold());
		auto sendmsg = PACKET_CREATE(packetl2c_bet_info_result, e_mst_l2c_bet_info_result);
		sendmsg->set_result((msg_type_def::e_msg_result_def)ret);
		sendmsg->set_bet_index(msg->bet_index());
		sendmsg->set_bet_gold(msg->bet_gold());
		sendmsg->set_player_id(lcplayer->get_pid());
		
		//玩家下注不广播;
		lcplayer->send_msg_to_client(sendmsg);
		//lcplayer->get_room()->broadcast_msg_to_client(sendmsg);
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//请求申请上庄
bool packetc2l_ask_apply_banker_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
													shared_ptr<packetc2l_ask_apply_banker> msg)
{	
	__ENTER_FUNCTION_CHECK;
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(lcplayer && lcplayer->get_room())
	{
		int ret = lcplayer->get_room()->get_game_main()->apply_banker(lcplayer);
		auto sendmsg = PACKET_CREATE(packetl2c_apply_banker_result, e_mst_l2c_apply_banker_result);
		sendmsg->set_result((msg_type_def::e_msg_result_def)ret);

		player->send_msg_to_client(sendmsg);
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//请求申请抢庄
bool packetc2l_ask_snatch_banker_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
														shared_ptr<packetc2l_ask_snatch_banker> msg)
{	
	__ENTER_FUNCTION_CHECK;
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(lcplayer && lcplayer->get_room())
	{
		auto game_main = lcplayer->get_room()->get_game_main();
		int ret = game_main->snatch_banker(lcplayer, msg->snatch_gold());
		auto sendmsg = PACKET_CREATE(packetl2c_snatch_banker_result, e_mst_l2c_snatch_banker_result);
		sendmsg->set_result((msg_type_def::e_msg_result_def)ret);

		player->send_msg_to_client(sendmsg);
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//请求取消申请上庄
bool packetc2l_ask_cancel_apply_banker_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
														shared_ptr<packetc2l_ask_cancel_apply_banker> msg)
{	
	__ENTER_FUNCTION_CHECK;
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(lcplayer && lcplayer->get_room())
	{
		int ret = lcplayer->get_room()->get_game_main()->cancel_apply_banker(lcplayer);
		auto sendmsg = PACKET_CREATE(packetl2c_cancel_apply_banker_result, e_mst_l2c_cancel_apply_banker_result);
		sendmsg->set_result((msg_type_def::e_msg_result_def)ret);

		player->send_msg_to_client(sendmsg);
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//请求申请下庄
bool packetc2l_ask_leave_banker_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
														shared_ptr<packetc2l_ask_leave_banker> msg)
{	
	__ENTER_FUNCTION_CHECK;
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(lcplayer && lcplayer->get_room())
	{
		int32_t cost_ticket = 0;
		int ret = lcplayer->get_room()->get_game_main()->ask_leave_banker(lcplayer, msg->force(), cost_ticket);
		auto sendmsg = PACKET_CREATE(packetl2c_leave_banker_result, e_mst_l2c_leave_banker_result);
		sendmsg->set_result((msg_type_def::e_msg_result_def)ret);
		sendmsg->set_cost_ticket(cost_ticket);

		player->send_msg_to_client(sendmsg);
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//请求续压
bool packetc2l_ask_continue_bet_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
														shared_ptr<packetc2l_ask_continue_bet> msg)
{	
	__ENTER_FUNCTION_CHECK;
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(lcplayer && lcplayer->get_room())
	{
		int ret = msg_type_def::e_msg_result_def::e_rmt_success;
		for (int i = 0; i < msg->bet_golds_size(); i++)
		{
			 ret = lcplayer->get_room()->get_game_main()->bet_gold(lcplayer, i, msg->bet_golds(i));
			 if (ret != msg_type_def::e_msg_result_def::e_rmt_success)
			 {
				 lcplayer->get_room()->get_game_main()->clear_bet_gold(lcplayer);
				 break;
			 }
		}
		auto sendmsg = PACKET_CREATE(packetl2c_continue_bet_result, e_mst_l2c_continue_bet_result);
		sendmsg->set_result((msg_type_def::e_msg_result_def)ret);
		if (ret == msg_type_def::e_msg_result_def::e_rmt_success)
		{
			for (int i = 0; i < msg->bet_golds_size(); i++)
			{
				sendmsg->mutable_bet_golds()->Add(msg->bet_golds(i));
			}
		}

		player->send_msg_to_client(sendmsg);
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//请求清空下注
bool packetc2l_ask_clear_bet_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
														shared_ptr<packetc2l_ask_clear_bet> msg)
{	
	__ENTER_FUNCTION_CHECK;
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(lcplayer && lcplayer->get_room())
	{
		lcplayer->get_room()->get_game_main()->clear_bet_gold(lcplayer);

		auto sendmsg = PACKET_CREATE(packetl2c_clear_bet_result, e_mst_l2c_clear_bet_result);
		sendmsg->set_result(msg_type_def::e_msg_result_def::e_rmt_success);

		player->send_msg_to_client(sendmsg);
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//请求庄家列表
bool packetc2l_ask_bankerlist_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
														shared_ptr<packetc2l_ask_bankerlist> msg)
{	
	__ENTER_FUNCTION_CHECK;
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(lcplayer && lcplayer->get_room())
	{
		auto& players = lcplayer->get_room()->get_game_main()->get_apply_bankers();

		auto sendmsg = PACKET_CREATE(packetl2c_bankerlist_result, e_mst_l2c_bankerlist_result);
		sendmsg->mutable_player_infos()->Reserve(players.size());
		for (auto it = players.begin(); it != players.end(); ++it)
		{
			auto player_info = sendmsg->add_player_infos();
			player_info->set_player_id((*it)->get_pid());
			//player_info->set_player_name((*it)->get_nickname());//去掉昵称;
			player_info->set_head_frame((*it)->get_photo_frame());
			player_info->set_head_custom((*it)->get_icon_custom());
			player_info->set_player_gold((*it)->get_gold());
			player_info->set_player_sex((*it)->get_sex());
			player_info->set_vip_level((*it)->get_viplvl());
			player_info->set_player_region((*it)->GetUserRegion());
		}

		player->send_msg_to_client(sendmsg);
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

bool my_greater(LPlayerPtr& a, LPlayerPtr& b)
{
	return a->get_gold() > b->get_gold();
}

//请求玩家列表
bool packetc2l_ask_playerlist_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
													  shared_ptr<packetc2l_ask_playerlist> msg)
{	
	__ENTER_FUNCTION_CHECK;
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(lcplayer && lcplayer->get_room())
	{
		int maxCount = 6;
		auto players = lcplayer->get_room()->get_otherplayers_without_banker(lcplayer);

		std::vector<LPlayerPtr> vec_players;
		for (auto it = players.begin(); it != players.end(); ++it)
		{
			vec_players.push_back(it->second);
		}
		std::sort(vec_players.begin(), vec_players.end(), my_greater);

		auto sendmsg = PACKET_CREATE(packetl2c_playerlist_result, e_mst_l2c_playerlist_result);

		if (players.size() < maxCount)
		{
			maxCount = players.size();
		}
		std::vector<LPlayerPtr> player_list;
		sendmsg->mutable_player_infos()->Reserve(maxCount);
		for (auto it = vec_players.begin(); it != vec_players.end(); ++it)
		{
			if (maxCount <= 0)
				break;

			//if (lcplayer==it->second)
			//	continue;

			player_list.push_back(*it);
			maxCount--;
		}
		std::sort(player_list.begin(), player_list.end(), my_greater);

		//存放客户端显示的玩家列表，用于结算时知道他们的输赢
		lcplayer->set_otherplayers(player_list);

		for (auto it = player_list.begin(); it != player_list.end(); it++)
		{
			auto player_info = sendmsg->add_player_infos();
			player_info->set_player_id((*it)->get_pid());
			//player_info->set_player_name((*it)->get_nickname());//去掉昵称;
			player_info->set_head_frame((*it)->get_photo_frame());
			player_info->set_head_custom((*it)->get_icon_custom());
			player_info->set_player_gold((*it)->get_gold());
			player_info->set_player_sex((*it)->get_sex());
			player_info->set_vip_level((*it)->get_viplvl());
			player_info->set_player_region((*it)->GetUserRegion());
		}

		player->send_msg_to_client(sendmsg);
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//请求牌路
bool packetc2l_ask_history_info_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
														shared_ptr<packetc2l_ask_history_info> msg)
{	
	__ENTER_FUNCTION_CHECK;
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(lcplayer && lcplayer->get_room())
	{
		auto sendmsg = PACKET_CREATE(packetl2c_history_info, e_mst_l2c_history_info);
		auto game_main = lcplayer->get_room()->get_game_main();
		auto& history_infos = game_main->get_history_infos();
		sendmsg->mutable_history_infos()->Reserve(history_infos.size());
		for (auto it = history_infos.begin(); it != history_infos.end(); ++it)
		{
			auto history_info = sendmsg->add_history_infos();
			history_info->mutable_is_win()->Reserve(4);
			for (int i = 0; i < 4; i++)
			{
				history_info->add_is_win(it->m_is_win[i]);
			}
		}

		auto& total_history_info = game_main->get_total_history_info();
		sendmsg->mutable_win_counts()->Reserve(4);
		sendmsg->mutable_lose_counts()->Reserve(4);
		for (int i = 0; i < 4; i++)
		{
			sendmsg->add_win_counts(total_history_info.m_total_win[i]);
			sendmsg->add_lose_counts(total_history_info.m_total_lose[i]);
		}
		sendmsg->set_total_count(game_main->get_history_total_count());

		player->send_msg_to_client(sendmsg);
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}