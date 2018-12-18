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
	//初始化机器人下注金额	
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

	//同步所有人金额
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
	//子房间id
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
	virtual void init_game_object();//注册属性
	virtual bool store_game_object(bool to_all = false);//非数组对象必须实现此接口

	//1天堂，0普通，-1地狱，-2大地狱
	int get_earn_type(GOLD_TYPE bet_gold, logic2logsvr::CowsProfitInfo& profit);

	void log_game_info(GOLD_TYPE total_win_gold, GOLD_TYPE bet_gold);
	void log_robot_info(GOLD_TYPE robot_gold);
	void log_game_single_info(int i, GOLD_TYPE win_gold, bool win);
	void log_banker_win_gold(GOLD_TYPE gold);
	void log_banker_lost_gold(GOLD_TYPE gold);

	void print_bet_win();
private:
	double m_checksave;
	Tfield<int16_t>::TFieldPtr RoomID;			//桌子id
	Tfield<int64_t>::TFieldPtr BankerAddGold;	//上庄手续费
	Tfield<int64_t>::TFieldPtr BankerSubGold;	//爆庄系统支付

	Tfield<int64_t>::TFieldPtr TotalWinGold;	//桌子总获利
	Tfield<int64_t>::TFieldPtr TotalLoseGold;	//桌子总支出

	Tfield<int64_t>::TFieldPtr TotalBetGold;	//桌子总下注

	Tfield<int64_t>::TFieldPtr TotalRobotWinGold;	//机器人赢情况
	Tfield<int64_t>::TFieldPtr TotalRobotLoseGold;	//机器人输情况

	Tfield<int64_t>::TFieldPtr WinGold1;		//桌子获利1
	Tfield<int64_t>::TFieldPtr LoseGold1;		//桌子支出1
	Tfield<int64_t>::TFieldPtr WinCount1;		//赢次数1

	Tfield<int64_t>::TFieldPtr WinGold2;		//桌子获利2
	Tfield<int64_t>::TFieldPtr LoseGold2;		//桌子支出2
	Tfield<int64_t>::TFieldPtr WinCount2;		//赢次数1

	Tfield<int64_t>::TFieldPtr WinGold3;		//桌子获利3
	Tfield<int64_t>::TFieldPtr LoseGold3;		//桌子支出3
	Tfield<int64_t>::TFieldPtr WinCount3;		//赢次数1

	Tfield<int64_t>::TFieldPtr WinGold4;		//桌子获利4
	Tfield<int64_t>::TFieldPtr LoseGold4;		//桌子支出4
	Tfield<int64_t>::TFieldPtr WinCount4;		//赢次数1

	Tfield<double>::TFieldPtr MaxEarnRate;		//最大盈利率
	Tfield<double>::TFieldPtr ExpectEarnRate;	//预期盈利率
	Tfield<double>::TFieldPtr MinEarnRate;		//最小盈利率
	int32_t m_cut_round;		    //每100局杀分次数;

	Tfield<int64_t>::TFieldPtr HistoryLogTime;		//记录时间
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