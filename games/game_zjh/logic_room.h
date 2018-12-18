#pragma once
#include "logic_def.h"

#include "GoldFlower_RobotRoomCFG.h"

struct GoldFlower_RoomCFGData;


ZJH_SPACE_BEGIN

class logic_room :public game_object
{
public:
	logic_room(const GoldFlower_RoomCFGData* cfg, logic_lobby* _lobby);
	~logic_room(void);
	void release();
	void heartbeat( double elapsed );
	virtual uint32_t get_id();
	uint16_t get_cur_cout();

	bool has_seat(uint16_t& tableid);

	bool has_seat_robot(uint16_t & tableid);

	bool change_table(uint16_t& tableid);
	
	int enter_table(LPlayerPtr player,uint16_t tid);
	void leave_table(uint32_t pid);
	void getout_room(uint32_t pid);
	const GoldFlower_RoomCFGData* get_roomcfg();

	void setservice(int service = 0);
	int getservice() const;

	void setkiller(int killer = 0, int cutRound = 0);
	int getkiller() const;
	int getCutRound() const;

	void setRobotSwitch(int rswitch);
	int getRobotSwitch() const;

	LPlayerPtr& get_player(uint32_t pid);
	LPLAYER_MAP& get_players();

	bool is_full();

	uint16_t inline get_room_id();

	const GoldFlower_RoomCFGData* get_data() const;

	logic_lobby*  get_lobby(){return m_lobby;}

	// 
	void read_robotcfg();
	void robot_heartbeat(double elapsed);
	void request_robot();
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
	const GoldFlower_RoomCFGData* m_cfg;
	GoldFlower_RobotRoomCFGData	m_robotcfg;
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
	int		m_service;	// 服务状态 1:开服 0：停服
	int			m_killer;	// 1 : 0
	int		m_rSwitch;	// 1:开 2:关
	int32_t		m_cutRound;	// 万分比例
private:
	double m_robot_elapsed;
	LPLAYER_MAP m_robot_players;
	double m_synplayers_elapsed;
};


ZJH_SPACE_END
