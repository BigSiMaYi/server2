#pragma once
#include "cards_def.h"

namespace cows_protocols
{
	class msg_result_info;
}
namespace logic2logsvr
{
	class CowsGameLog;
}
COWS_SPACE_BEGIN

class logic_main
{
public:
	logic_main(logic_room* room);
	~logic_main(void);

	void heartbeat(double elapsed);

	void test_game();
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
	//������ׯ
	int apply_banker(LPlayerPtr& player);
	//ȡ��������ׯ
	int cancel_apply_banker(LPlayerPtr& player);
	//�����ׯ����
	void clear_apply_banker(LPlayerPtr& player);

	std::list<LPlayerPtr>& get_apply_bankers();

	int get_robot_banker_size();
	//���
	int get_inc_id();

	//������ׯ
	bool can_snatch();
	int get_snatch_gold();
	uint32_t get_snatch_player();
	//��ׯ
	int snatch_banker(LPlayerPtr& player, int gold);
	//������ׯ
	int ask_leave_banker(LPlayerPtr& player, bool force, int32_t& cost_ticket);
	//��·
	void log_history_cards();
	struct total_history_info
	{
		int m_total_win[4];
		int m_total_lose[4];
	};
	struct history_info
	{
		bool m_is_win[4];
	};

	int64_t get_history_log_time();
	int get_history_total_count();
	const total_history_info& get_total_history_info();
	std::list<history_info>& get_history_infos();

	bool can_leave_room(uint32_t player_id);
	//����뿪����
	void leave_room(LPlayerPtr& player);

	void fill_result_info(LPlayerPtr& player, cows_protocols::msg_result_info* result_info);

	void update_star_gold();
	double  get_room_commission_rate();
public:
	//�ж�ׯ��
	bool check_banker();
	void clear_history();
	void init_history(GIntListFieldPtr& history, time_t log_time);
public:
	logic_room* get_room();
	logic_banker* get_banker();
	std::vector<logic_other*>& get_others();
	enum class game_state
	{
		game_state_unknown,	//δ��ʼ
		game_state_prepare,	//׼��,������ׯ
		game_state_bet,		//��ע
		game_state_deal,	//����
		game_state_result,	//���
		game_state_pause,	//��ͣ;
	};
	game_state get_game_state();
	float get_duration();
	int get_cd_time();

	void kill_points(int32_t cutRound, bool status);
	void service_ctrl(int32_t optype);

protected:
	void set_game_state(game_state state);
	void add_snatch_player(LPlayerPtr& player, int ticket);
	void remove_snatch_player(LPlayerPtr& player);
	void clear_snatch_player();
	//�����������˰��;
	void  calc_player_rax();
	LPlayerPtr getBesterPlayerPtr();
	void server_stop();
	void check_player_status(bool server_stop);
	//����;
	void swap_cards(std::vector<std::pair<int, logic_cards*> >& cards, int swap_type);
	//��һ�ֵ�ʤ�ʣ�ͨɱ-1��ͨ��1������0;
	int pre_game_win_rate();
	int cur_game_wind_rate();
	//�������Ƿ���ע;
	bool check_bet(int32_t pid);
private:
	int m_inc_id;
	logic_room* m_room;

	game_state m_game_state;
	float m_duration;
	float m_sync_eleased;
	float m_sync_interval;

	//ׯ��
	logic_banker* m_banker;
	bool m_banker_killall;
	bool m_change_bet;
	//�м�
	std::vector<logic_other*> m_others;
	//��¼�������������ע;
	std::map<int, GOLD_TYPE> m_rebots_bet;
	//������ע��¼;
	std::map<int, GOLD_TYPE> m_player_bet;

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
	total_history_info m_total_history;
	std::list<history_info> m_history_infos;

	//���ƹ���
	logic_poker_mgr* m_poker_mgr;

	//��Һͻ�������ׯ���ʷ��Ƹ���;
	std::vector<float> m_player_banker_probs;
	std::vector<float> m_robot_banker_probs;
	bool m_kill_points_switch;
	int32_t m_service_status;
	std::shared_ptr<logic2logsvr::CowsGameLog> m_cows_log;
};

COWS_SPACE_END