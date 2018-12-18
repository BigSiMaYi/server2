#pragma once
#include "cards_def.h"
#include "dragon_tiger_logic.pb.h"
#include "logic2logsvr_msg_type.pb.h"

namespace dragon_tiger_protocols
{
	class msg_result_info;
}

DRAGON_TIGER_SPACE_BEGIN

class logic_main
{
public:
	logic_main(logic_room* room);
	~logic_main(void);

	void heartbeat(double elapsed);

	//��Ϸû�˵ȣ���ͣ��Ϸ
	void stop_game();
	//׼����ʼ��Ϸ
	void pre_start_game();
	//��ʼ��һ��,�������
	void start_game();
	void clear_last_data();
	//��ע
	int bet_gold(LPlayerPtr& player, uint32_t index, GOLD_TYPE gold);
	//�����ע
	void clear_bet_gold(LPlayerPtr& player);

	//��ȡ�����ע���
	GOLD_TYPE get_player_betgold(LPlayerPtr& player);
	//���������ע���
	void add_player_betgold(LPlayerPtr& player, GOLD_TYPE gold);
	//��������ע���
	void clear_player_betgold(LPlayerPtr& player);

	//ͬ����ע���
	void sync_bet_gold();
	//����
	void deal_cards();
	//������
	void computer_result();

	//���
	int get_inc_id();
	time_t get_history_log_time();
	int get_history_total_count();

	bool can_leave_room(uint32_t player_id);
	//����뿪����
	void leave_room(LPlayerPtr& player);

	void fill_result_info(LPlayerPtr& player, dragon_tiger_protocols::msg_result_info* result_info);

	void update_star_gold();

	logic_poker_mgr* get_poker_manager() { return m_poker_mgr; }
	
public:
	logic_room* get_room();

	std::vector<logic_other*>& get_others();
	enum class game_state
	{
		game_state_unknown,	//δ��ʼ
		game_state_prepare,	//׼��,������ׯ
		game_state_bet,		//��ע
		game_state_deal,	//����
		game_state_result,	//���
	};
	game_state get_game_state();
	float get_duration();
	int get_cd_time();

	void kill_points(int32_t cutRound, bool status);
	void service_ctrl(int32_t optype);
	bool cut_round_check();
	void server_stop();

	int get_cut_round_tag();

protected:
	void set_game_state(game_state state);
	void check_player_status(bool server_stop);
private:
	int m_inc_id;
	logic_room* m_room;

	std::shared_ptr<logic2logsvr::DragonTigerGameLog> m_dragon_tiger_log;
	game_state m_game_state;
	float m_duration;
	float m_sync_eleased;
	float m_sync_interval;

	bool m_change_bet;
	//�м�
	std::vector<logic_other*> m_others;

	//ׯ������
	std::list<LPlayerPtr> m_apply_players;
	std::set<uint32_t> m_apply_pids;

	LPlayerPtr m_snatch_player;
	int m_snatch_gold;
	std::map<uint32_t, int32_t> m_snatch_player_ids;

	//��ע���
	std::map<uint32_t, GOLD_TYPE> m_bet_players;

	//��·
	int m_total_count;
	time_t m_log_time;

	logic_poker_mgr* m_poker_mgr;

	int32_t m_service_status;
	int m_cutroudtag;
	bool m_kill_points_switch;
	int m_cfgCutRound;
	int m_gametimes;
	int m_cuttimes;

	int m_winner_index;

	int m_cutround_tag;
};

DRAGON_TIGER_SPACE_END