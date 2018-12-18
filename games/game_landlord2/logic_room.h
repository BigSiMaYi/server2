#pragma once
#include "logic_def.h"

struct Landlord3_RoomCFGData;

LANDLORD_SPACE_BEGIN

class robot_cfg;

class logic_room :public game_object
{
public:
	logic_room(const Landlord3_RoomCFGData* cfg, logic_lobby* _lobby, int child_id);
	~logic_room(void);
	void release();
	void heartbeat( double elapsed );
	virtual uint32_t get_id();
	uint16_t get_cur_cout();

	bool has_seat(LPlayerPtr& player, uint16_t& tableid, bool isrobot);
	
	int enter_table(LPlayerPtr player,uint16_t tid);
	void leave_table(uint32_t pid);
	const Landlord3_RoomCFGData* get_roomcfg();

	LPlayerPtr& get_player(uint32_t pid);
	LPLAYER_MAP& get_players();

	bool is_full();

	uint16_t inline get_room_id();
    uint16_t get_child_id();

	const Landlord3_RoomCFGData* get_data() const;

	logic_lobby*  get_lobby(){return m_lobby;}

	// 
	void robot_heartbeat(double elapsed);
	void request_robot(int32_t tid);
	//��ʼ����������ע���	
private:
	logic_lobby* m_lobby;
	const Landlord3_RoomCFGData* m_cfg;
	LTABLE_MAP m_tables;
    std::map<int, robot_cfg*> m_table_robots;

	LPLAYER_MAP m_players;
	std::vector<uint32_t> m_pids;
	//////////////////////////////////////////////////////////////////////////
	void create_room();
	bool load_room();

public:
	//�㲥Э�飬���̷���
	template<class T>
	int broadcast_msg_to_client(T msg)
	{
		return broadcast_msg_to_client(m_pids, msg->packet_id(), msg);
	};
	int broadcast_msg_to_client(std::vector<uint32_t>& pids, uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg);

public:
	virtual void init_game_object();//ע������
	virtual bool store_game_object(bool to_all = false);//������������ʵ�ִ˽ӿ�

	//1���ã�0��ͨ��-1������-2�����
	int get_earn_type(GOLD_TYPE bet_gold);

	void log_game_info(GOLD_TYPE total_win_gold, GOLD_TYPE bet_gold);
	void log_robot_info(GOLD_TYPE robot_gold);
	void log_game_single_info(int i, GOLD_TYPE win_gold, bool win);
	void log_banker_win_gold(GOLD_TYPE gold);
	void log_banker_lost_gold(GOLD_TYPE gold);

	void print_bet_win();

	uint32_t get_robot_size();
	void record_robot(bool flag);

    void service_ctrl(int32_t optype);
    void kill_points(int32_t cutRound, bool status);
    void kick_player(uint32_t playerid, int bforce);
	void robot_ctrl(bool on_off);

    int32_t get_cut_round();

private:
	double m_checksave;
	Tfield<int16_t>::TFieldPtr RoomID;			//����id
	Tfield<int16_t>::TFieldPtr PlayerCount;		//��ǰ�����
    int16_t m_child_id;

private:
	double m_robot_elapsed;
	LPLAYER_MAP m_robot_players;
    int32_t m_killpoint;
    bool m_killpoint_status;
    int32_t m_server_stauts;
	int32_t m_req_robot;
	double m_sync_palyer_times = 0;
};


LANDLORD_SPACE_END
