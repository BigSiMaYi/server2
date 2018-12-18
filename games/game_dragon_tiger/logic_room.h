#pragma once
#include "logic_def.h"

struct DragonTiger_RoomCFGData;
struct DragonTiger_RoomStockCFGData;
DRAGON_TIGER_SPACE_BEGIN
#define PROPERTY_DEFINE(type, name, access_permission)\
access_permission:\
	type m_##name;\
	public:\
	inline void set_##name(type v) {\
	m_##name = v;\
}\
	inline type get_##name() {\
	return m_##name;\
}\

struct cards_trend //牌路走势;
{
	int m_win_area;
	int m_is_win[3];
};


class logic_room :public game_object
{
public:
	logic_room(const DragonTiger_RoomCFGData* cfg, logic_lobby* _lobby, int child_id);
	~logic_room(void);

	void heartbeat( double elapsed );
	void robot_heartbeat(double elapsed);
	void request_robot();
	//初始化机器人下注金额	
	void init_robot_bet();

	const DragonTiger_RoomCFGData* get_roomcfg();
	virtual uint32_t get_id();
	uint16_t get_cur_cout();
	LPlayerPtr& get_player(uint32_t pid);
	LPLAYER_MAP& get_players();

	bool has_seat(uint16_t& tableid);

	int enter_room(LPlayerPtr player);
	void leave_room(uint32_t pid);

	uint16_t inline get_room_id();

	const DragonTiger_RoomCFGData* get_data() const;

	logic_lobby*  get_lobby(){return m_lobby;}
	logic_main* get_game_main() {return m_main;}

	//根据玩家输赢计算税收
	void calc_player_tax(float tax_rate);

	//同步所有人金额
	void sync_player_gold();

	//清除局信息
	void clear_player_balane_info();

	int get_child_id();

	PROPERTY_DEFINE(int, equal_panel_gold, public);

	//记录房间游戏输赢信息
	void record_room_win_lose_info(int info);

	void record_cards_trend(cards_trend cardstrend);
	void broadcast_balance_msg();

	int  get_dragon_counts();

	int  get_tiger_counts();

	std::list<int> &  get_history_pokes_info();

	int64_t get_today_win_gold();
	int64_t get_today_lose_gold();
	int64_t get_today_bet_gold();
	void clear_today_gold();

	void kill_points(int32_t cutRound, bool status);
	void kick_player(uint32_t playerid, int bforce);
	void service_ctrl(int32_t optype);

	std::list<cards_trend>& get_cards_trend()
	{
		return m_history_infos;
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
	const DragonTiger_RoomCFGData* m_cfg;

	LPLAYER_MAP m_players;
	std::vector<uint32_t> m_pids;

	logic_main* m_main;

	double m_robot_elapsed;
	LPLAYER_MAP m_robot_players;

	//////////////////////////////////////////////////////////////////////////
	void create_room();
	bool load_room();

	void reflush_rate();

	std::list<cards_trend> m_history_infos;   //牌路，用于计算机器人下注区域;
public:
	virtual void init_game_object();//注册属性
	virtual bool store_game_object(bool to_all = false);//非数组对象必须实现此接口

	void log_game_info(GOLD_TYPE total_win_gold, GOLD_TYPE bet_gold);
	void log_robot_info(GOLD_TYPE robot_gold);
	void log_game_single_info(int i, GOLD_TYPE win_gold, bool win);
	void update_playerlist();
	std::list<std::pair<uint32_t, LPlayerPtr>>& get_show_players(uint32_t pid);
	void print_bet_win();
	void release();

	std::vector<std::pair<uint32_t, LPlayerPtr>> get_player_list();
	std::pair<uint32_t, LPlayerPtr> get_rich_player();
	std::pair<uint32_t, LPlayerPtr> get_best_player(uint32_t pid);

	double m_checksave;
	Tfield<int16_t>::TFieldPtr RoomID;			//桌子id

private:

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

	Tfield<time_t>::TFieldPtr HistoryLogTime;		//记录时间
	GIntListFieldPtr History;

	int64_t m_today_win_gold;
	int64_t m_today_lose_gold;
	int64_t m_today_bet_gold;

	int64_t m_avg_bet_gold;

	uint16_t m_player_count;
	

	PROPERTY_DEFINE( bool,  stock_change,		private );

	double m_tempstock;

	double m_tempwinnertax;
	double m_templosttax;

	std::list<int>			m_list_balance_win_lose_info;
	std::list<std::pair<uint32_t, LPlayerPtr>> m_player_list;//左右两边的排列的6个玩家: 第一个大富豪，第二个神算子，后面4个每局随机变化两个;


public:

	const DragonTiger_RoomStockCFGData *m_slmdata;

	Tfield<double>::TFieldPtr		WinnerEarningsRate;	//赢钱抽水
	Tfield<double>::TFieldPtr		LostEarningsRate;		//输钱抽水
	Tfield<int64_t>::TFieldPtr		EnterCount;		//进入次数
	Tfield<int16_t>::TFieldPtr		PlayerCount;	    //当前玩家数
	Tfield<double>::TFieldPtr		TotalStock;		//总库存
	Tfield<double>::TFieldPtr		TotalTaxWinner;	//赢的税收
	Tfield<double>::TFieldPtr		TotalTaxLost;		//输的税收

	float get_winner_tax(int room_id);

	void reflush_roomstock();

private:
	int m_child_id;
	int32_t m_cut_round;		    //每100局杀分次数;
	bool m_kill_points_switch;
	int32_t m_server_stauts;
	double m_sync_palyer_times;
};


DRAGON_TIGER_SPACE_END
