#pragma once
#include "logic_def.h"


namespace cows_protocols
{
	class msg_player_info;
}

COWS_SPACE_BEGIN
	//ׯ��
class logic_banker
{
public:
	logic_banker(void);
	~logic_banker(void);

	void check_player_state();
	//��ʼ��һ����Ϸ
	void start_game();

	//�ı��Ǯ
	void win_gold(GOLD_TYPE gold, GOLD_TYPE tax);

	logic_cards* get_cards();
	void set_cards(logic_cards* cards);

	uint32_t get_player_id();
	LPlayerPtr& get_player();
	//����
	std::string get_banker_name();
	//��ע���
	GOLD_TYPE get_max_bet_gold();
	//ׯ�ҽ��
	GOLD_TYPE get_banker_gold();

	GOLD_TYPE get_cur_bet_gold();
	//�ж�ׯ�ҽ���Ƿ��㹻
	bool check_bet_gold(GOLD_TYPE gold);
	void bet_gold(GOLD_TYPE gold);
	void clear_bet_gold(GOLD_TYPE gold);

	void resetSystemBanker();

	//�������ׯ��
	void set_player_banker(LPlayerPtr& player);
	bool is_banker(uint32_t player_id);
	//�Ƿ���ׯ��
	bool is_player_banker();

	bool is_banker_protect();
	//ϵͳ���������ׯ
	bool is_system_banker();
	//��ׯ����
	int get_banker_count();
	int get_min_banker_count();
	GOLD_TYPE get_total_win_gold();
	GOLD_TYPE get_last_win_gold();

	void fill_player_info(cows_protocols::msg_player_info* player_info);
	//������ׯ
	int force_leave();
	void apply_leave();
	bool is_apply_leave();

	//������Ϣ
	const std::string getBankerRegion();
private:
	//ׯ�ҵ���
	logic_cards* m_cards;

	//�����ע���
	GOLD_TYPE m_max_bet_gold;
	//��ǰ��ע���
	GOLD_TYPE m_cur_bet_gold;

	std::string m_system_name;
	GOLD_TYPE m_system_gold;

	int m_min_banker_count;
	int m_max_banker_count;
	float m_leave_banker_cost;

	int m_offline_count;
	bool m_apply_leave;
	LPlayerPtr m_player;
	int m_banker_count;			//��ׯ����
	time_t m_start_time;		//��ʼׯ��ʱ��
	GOLD_TYPE m_start_gold;			//��ʼ��ׯ��Ǯ
	int64_t m_total_win_gold;
	GOLD_TYPE m_last_win_gold;
	GOLD_TYPE m_sys_lose_gold;		//��ׯϵͳ֧����Ǯ
};

COWS_SPACE_END