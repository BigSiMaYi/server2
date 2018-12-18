#include "stdafx.h"
#include "proc_dragon_tiger_logic.h"
#include <i_game_player.h>
#include "logic_player.h"
#include "logic_room.h"
#include "logic_main.h"
#include "logic_other.h"
#include "logic_cards.h"
#include "game_engine.h"
#include "DragonTiger_RoomCFG.h"

DRAGON_TIGER_SPACE_USING
using namespace boost;

void init_proc_dragon_tiger_logic()
{
	packetc2l_get_scene_info_factory::regedit_factory();
	packetl2c_get_scene_info_result_factory::regedit_factory();

	packetc2l_ask_bet_info_factory::regedit_factory();
	packetl2c_bet_info_result_factory::regedit_factory();

	packetc2l_ask_playerlist_factory::regedit_factory();
	packetl2c_playerlist_result_factory::regedit_factory();

	packetc2l_ask_history_info_factory::regedit_factory();
	packetl2c_history_info_factory::regedit_factory();

	packetc2l_ask_continue_bet_factory::regedit_factory();
	packetl2c_continue_bet_result_factory::regedit_factory();
	packetc2l_ask_clear_bet_factory::regedit_factory();
	packetl2c_clear_bet_result_factory::regedit_factory();

	packetl2c_bc_scene_prepare_into_factory::regedit_factory();
	packetl2c_bc_scene_bet_into_factory::regedit_factory();
	packetl2c_bc_sync_scene_bet_into_factory::regedit_factory();
	packetl2c_bc_scene_result_into_factory::regedit_factory();
	packetl2c_bc_scene_deal_into_factory::regedit_factory();
}

//��ȡ��Ϸ��Ϣ
bool packetc2l_get_scene_info_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
													 shared_ptr<packetc2l_get_scene_info> msg)
{	
	__ENTER_FUNCTION_CHECK;

	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if (lcplayer != nullptr && lcplayer->get_room() == nullptr)
	{
		game_engine::instance().get_lobby().enter_room_nocheck(lcplayer->get_pid(), 1);
	}

	if(lcplayer && lcplayer->get_room())
	{
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
// 			auto cards_info = scene_info->add_cards_infos();
// 			game_main->get_banker()->get_cards()->fill_cards_info(cards_info);
// 
// 			auto& other = game_main->get_others();
// 			for (unsigned int i = 0; i < other.size(); i++)
// 			{
// 				auto cards_info = scene_info->add_cards_infos();
// 				other[i]->get_cards()->fill_cards_info(cards_info);
// 			}
		}
		if (state >= (int)logic_main::game_state::game_state_result)
		{
			auto result_info = scene_info->mutable_result_info();
			//game_main->fill_result_info(lcplayer, result_info);
		}

		player->send_msg_to_client(sendmsg);
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//������ע
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

		player->send_msg_to_client(sendmsg);
	}

	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//������ѹ
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

//���������ע
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


bool my_greater(LPlayerPtr& a, LPlayerPtr& b)
{
	return a->get_gold() > b->get_gold();
}

//��������б�
bool packetc2l_ask_playerlist_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
													  shared_ptr<packetc2l_ask_playerlist> msg)
{	
	__ENTER_FUNCTION_CHECK;
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(lcplayer && lcplayer->get_room())
	{
		auto& players = lcplayer->get_room()->get_players();

		auto sendmsg = PACKET_CREATE(packetl2c_playerlist_result, e_mst_l2c_playerlist_result);
		int maxCount = 120;
		if (players.size() < maxCount)
		{
			maxCount = players.size();
		}
		std::vector<LPlayerPtr> player_list;
		sendmsg->mutable_player_infos()->Reserve(maxCount);
		for (auto it = players.begin(); it != players.end(); ++it)
		{
			if (maxCount < 0)
				break;

			player_list.push_back(it->second);
			maxCount--;
		}
		std::sort(player_list.begin(), player_list.end(), my_greater);

		for (auto it = player_list.begin(); it != player_list.end(); it++)
		{
			auto player_info = sendmsg->add_player_infos();
			player_info->set_player_id((*it)->get_pid());
			player_info->set_player_name((*it)->get_nickname());
			player_info->set_head_frame((*it)->get_photo_frame());
			player_info->set_head_custom((*it)->get_icon_custom());
			player_info->set_player_gold((*it)->get_gold());
			player_info->set_player_sex((*it)->get_sex());
			player_info->set_vip_level((*it)->get_viplvl());
			player_info->set_cost_gold((*it)->get_bet_gold());
			player_info->set_winner_count((*it)->get_winner_count());
		}

		player->send_msg_to_client(sendmsg);
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}

//������·
bool packetc2l_ask_history_info_factory::packet_process(shared_ptr<peer_tcp> peer, shared_ptr<i_game_player> player, 
														shared_ptr<packetc2l_ask_history_info> msg)
{	
	__ENTER_FUNCTION_CHECK;
	auto lcplayer =  CONVERT_POINT(logic_player, player->get_handler());
	if(lcplayer && lcplayer->get_room())
	{
		auto sendmsg = PACKET_CREATE(packetl2c_history_info, e_mst_l2c_history_info);
		sendmsg->set_dragon_counts(lcplayer->get_room()->get_dragon_counts());
		sendmsg->set_tiger_counts(lcplayer->get_room()->get_tiger_counts());

		auto& history_infos = lcplayer->get_room()->get_history_pokes_info();
		sendmsg->mutable_history_infos()->Reserve(history_infos.size());

		std::list<int>::iterator iter = history_infos.begin();
		for (; iter != history_infos.end(); iter++)
		{
			auto history_info = sendmsg->add_history_infos();
			history_info->set_win_type(*iter);
		}
	
		player->send_msg_to_client(sendmsg);
	}
	__LEAVE_FUNCTION_CHECK
		return !EX_CHECK;
}