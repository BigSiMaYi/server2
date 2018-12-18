#pragma once
#include "logic_def.h"

struct Landlord3_RoomCFGData;

LANDLORD_SPACE_BEGIN

class logic_room :public game_object
{
public:
	logic_room(const Landlord3_RoomCFGData* cfg, logic_lobby* _lobby, int child_id);
	~logic_room(void);
	void release();
	void heartbeat( double elapsed );
	virtual uint32_t get_id();
	uint16_t get_cur_cout();

	bool has_seat(uint16_t& tableid);
	
	int enter_table(LPlayerPtr player,uint16_t tid);
	void leave_table(uint32_t pid);
	const Landlord3_RoomCFGData* get_roomcfg();

	LPlayerPtr& get_player(uint32_t pid);
	LPLAYER_MAP& get_players();

	bool is_full();

	uint16_t inline logic_room::get_room_id();

	const Landlord3_RoomCFGData* get_data() const;

	logic_lobby*  get_lobby(){return m_lobby;}

	// 
	void robot_heartbeat(double elapsed);
	void request_robot(int32_t tid);
	//初始化机器人下注金额	

public:
	//广播协议，立刻发送
	template<class T>
	int broadcast_msg_to_client(T msg)
	{
		return broadcast_msg_to_client(m_pids, msg->packet_id(), msg);
	};
	int broadcast_msg_to_client(std::vector<uint32_t>& pids, uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg);
protected:

private:
	logic_lobby* m_lobby;
	const Landlord3_RoomCFGData* m_cfg;
	LTABLE_MAP m_tables;

	LPLAYER_MAP m_players;
	std::vector<uint32_t> m_pids;
	//////////////////////////////////////////////////////////////////////////
	void create_room();
	bool load_room();

public:
	virtual void init_game_object();//注册属性
	virtual bool store_game_object(bool to_all = false);//非数组对象必须实现此接口

	//1天堂，0普通，-1地狱，-2大地狱
	int get_earn_type(GOLD_TYPE bet_gold);

	void log_game_info(GOLD_TYPE total_win_gold, GOLD_TYPE bet_gold);
	void log_robot_info(GOLD_TYPE robot_gold);
	void log_game_single_info(int i, GOLD_TYPE win_gold, bool win);
	void log_banker_win_gold(GOLD_TYPE gold);
	void log_banker_lost_gold(GOLD_TYPE gold);

	void print_bet_win();


private:
	double m_checksave;
	Tfield<int16_t>::TFieldPtr RoomID;			//桌子id
	Tfield<int16_t>::TFieldPtr PlayerCount;		//当前玩家数

private:
	double m_robot_elapsed;
	LPLAYER_MAP m_robot_players;
};


LANDLORD_SPACE_END
