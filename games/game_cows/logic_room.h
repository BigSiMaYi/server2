#pragma once
#include "logic_def.h"

struct Cows_RoomCFGData;

namespace logic2logsvr
{
	class CowsProfitInfo;
}

COWS_SPACE_BEGIN

class logic_room :public game_object
{
public:
	logic_room(const Cows_RoomCFGData* cfg, logic_lobby* _lobby, int child_id);
	~logic_room(void);

	void heartbeat( double elapsed );
	void robot_heartbeat(double elapsed);
	void request_robot(int level = 0);
	//��ʼ����������ע���	
	void init_robot_bet();

	const Cows_RoomCFGData* get_roomcfg();
	virtual uint32_t get_id();
	uint16_t get_cur_cout();
	LPlayerPtr& get_player(uint32_t pid);
	LPLAYER_MAP& get_players();
	LPLAYER_MAP get_otherplayers_without_banker(LPlayerPtr& lcplayer);

	bool has_seat(uint16_t& tableid);

	int enter_room(LPlayerPtr player);
	void leave_room(uint32_t pid);

	uint16_t inline get_room_id();

	const Cows_RoomCFGData* get_data() const;

	logic_lobby*  get_lobby(){return m_lobby;}
	logic_main* get_game_main() {return m_main;}

	//ͬ�������˽��
	void sync_player_gold();
	double    get_rate();
	void broadcast_balance_msg();

	int64_t get_today_win_gold();
	int64_t get_today_lose_gold();
	int64_t get_today_bet_gold();
	void clear_today_gold();

	void reflush_history();
	void kill_points(int32_t cutRound, bool status);
	void kick_player(uint32_t playerid, int bforce);
	void service_ctrl(int32_t optype);
	//�ӷ���id
	int  get_child_id()
	{
		return m_child_id;
	}
    std::string get_name();

	int32_t get_server_status()
	{
		return m_server_stauts;
	}
	int  get_cut_round()
	{
		return m_cut_round;
	}
public:
	//�㲥Э�飬���̷���
	template<class T>
	int broadcast_msg_to_client(T msg)
	{
		return broadcast_msg_to_client(m_pids, msg->packet_id(), msg);
	};
	int broadcast_msg_to_client(std::vector<uint32_t>& pids, uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg);
protected:
private:
	logic_lobby* m_lobby;
	const Cows_RoomCFGData* m_cfg;

	LPLAYER_MAP m_players;
	std::vector<uint32_t> m_pids;

	logic_main* m_main;

	double m_robot_elapsed;
	LPLAYER_MAP m_robot_players;

	//////////////////////////////////////////////////////////////////////////
	void create_room();
	bool load_room();

	void reflush_rate();
public:
	virtual void init_game_object();//ע������
	virtual bool store_game_object(bool to_all = false);//������������ʵ�ִ˽ӿ�

	//1���ã�0��ͨ��-1������-2�����
	int get_earn_type(GOLD_TYPE bet_gold, logic2logsvr::CowsProfitInfo& profit);

	void log_game_info(GOLD_TYPE total_win_gold, GOLD_TYPE bet_gold);
	void log_robot_info(GOLD_TYPE robot_gold);
	void log_game_single_info(int i, GOLD_TYPE win_gold, bool win);
	void log_banker_win_gold(GOLD_TYPE gold);
	void log_banker_lost_gold(GOLD_TYPE gold);

	void print_bet_win();
private:
	double m_checksave;
	Tfield<int16_t>::TFieldPtr RoomID;			//����id
	Tfield<int64_t>::TFieldPtr BankerAddGold;	//��ׯ������
	Tfield<int64_t>::TFieldPtr BankerSubGold;	//��ׯϵͳ֧��

	Tfield<int64_t>::TFieldPtr TotalWinGold;	//�����ܻ���
	Tfield<int64_t>::TFieldPtr TotalLoseGold;	//������֧��

	Tfield<int64_t>::TFieldPtr TotalBetGold;	//��������ע

	Tfield<int64_t>::TFieldPtr TotalRobotWinGold;	//������Ӯ���
	Tfield<int64_t>::TFieldPtr TotalRobotLoseGold;	//�����������

	Tfield<int64_t>::TFieldPtr WinGold1;		//���ӻ���1
	Tfield<int64_t>::TFieldPtr LoseGold1;		//����֧��1
	Tfield<int64_t>::TFieldPtr WinCount1;		//Ӯ����1

	Tfield<int64_t>::TFieldPtr WinGold2;		//���ӻ���2
	Tfield<int64_t>::TFieldPtr LoseGold2;		//����֧��2
	Tfield<int64_t>::TFieldPtr WinCount2;		//Ӯ����1

	Tfield<int64_t>::TFieldPtr WinGold3;		//���ӻ���3
	Tfield<int64_t>::TFieldPtr LoseGold3;		//����֧��3
	Tfield<int64_t>::TFieldPtr WinCount3;		//Ӯ����1

	Tfield<int64_t>::TFieldPtr WinGold4;		//���ӻ���4
	Tfield<int64_t>::TFieldPtr LoseGold4;		//����֧��4
	Tfield<int64_t>::TFieldPtr WinCount4;		//Ӯ����1

	Tfield<double>::TFieldPtr MaxEarnRate;		//���ӯ����
	Tfield<double>::TFieldPtr ExpectEarnRate;	//Ԥ��ӯ����
	Tfield<double>::TFieldPtr MinEarnRate;		//��Сӯ����
	int32_t m_cut_round;		    //ÿ100��ɱ�ִ���;

	Tfield<int64_t>::TFieldPtr HistoryLogTime;		//��¼ʱ��
	GIntListFieldPtr History;

	int64_t m_today_win_gold;
	int64_t m_today_lose_gold;
	int64_t m_today_bet_gold;

	int64_t m_avg_bet_gold;

	uint16_t m_nMaxPlayerCnt;
	double m_banker_robot_interval;
	bool m_kill_points_switch;
	int32_t m_server_stauts;
	int m_child_id;
};


COWS_SPACE_END