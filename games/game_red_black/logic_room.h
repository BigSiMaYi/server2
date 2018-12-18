#pragma once
#include "logic_def.h"

struct RedBlack_RoomCFGData;
struct RedBlack_RoomStockCFGData;
struct cards_trend;

DRAGON_RED_BLACK_BEGIN

class logic_room : public game_object
{
public:
	logic_room(const RedBlack_RoomCFGData* cfg, logic_lobby* _lobby, uint16_t room_id, uint16_t child_id);
	~logic_room(void);

	void heartbeat( double elapsed );
	void robot_heartbeat(double elapsed);
	void request_robot();

	virtual uint32_t get_id();
	uint16_t get_cur_cout();
	LPlayerPtr& get_player(uint32_t pid);
	LPLAYER_MAP& get_players();
	void update_playerlist();
	std::list<std::pair<uint32_t, LPlayerPtr>>& get_show_players(uint32_t pid);

	bool has_seat(uint16_t& tableid);

	int enter_room(LPlayerPtr player);
	void leave_room(uint32_t pid);

	void init_robot_bet();
	double    get_rate();
	uint16_t inline get_room_id();
	int  get_child_id();

	const RedBlack_RoomCFGData* get_data() const;

	logic_lobby*  get_lobby(){return m_lobby;}
	logic_main* get_game_main() {return m_main;}

	//同步所有人金额
	void sync_player_gold();

	//清除局信息
	void clear_player_balane_info();

	//记录房间游戏输赢信息
	void record_room_win_lose_info(cards_trend& cardstrend);

	int  get_red_win_counts();

	int  get_black_win_counts();

	std::list<std::pair<int, int> > &  get_history_pokes_info();

	int64_t get_today_win_gold();
	int64_t get_today_lose_gold();
	int64_t get_today_bet_gold();
	void     clear_today_gold();

	void kill_points(int32_t cutRound, bool status);
	void kick_player(uint32_t playerid, int bforce);
	void service_ctrl(int32_t optype);

public:
	//广播协议，立刻发送
	template<class T>
	int broadcast_msg_to_client(T msg)
	{
		return broadcast_msg_to_client(m_pids, msg->packet_id(), msg);
	};
	int broadcast_msg_to_client(std::vector<uint32_t>& pids, uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg);

protected:
	std::vector<std::pair<uint32_t, LPlayerPtr>> get_player_list();
	std::pair<uint32_t, LPlayerPtr> get_rich_player();
	std::pair<uint32_t, LPlayerPtr> get_best_player(uint32_t pid);

public:
	const RedBlack_RoomStockCFGData *m_room_cfg;

private:
	logic_lobby* m_lobby;
	const RedBlack_RoomCFGData* m_cfg;

	LPLAYER_MAP m_players;
	std::vector<uint32_t> m_pids;

	logic_main* m_main;

	double m_robot_elapsed;
	LPLAYER_MAP m_robot_players;

	//////////////////////////////////////////////////////////////////////////
	void create_room();
	bool load_room();

public:
	virtual void init_game_object();//注册属性
	virtual bool store_game_object(bool to_all = false);//非数组对象必须实现此接口

	void release();
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

	int64_t m_today_win_gold;
	int64_t m_today_lose_gold;
	int64_t m_today_bet_gold;

	int64_t m_avg_bet_gold;

	uint16_t m_player_count;
	

	PROPERTY_DEFINE( bool,  stock_change,		private );


	std::list<std::pair<int, int>>			m_list_balance_win_lose_info;
	
	std::list<std::pair<uint32_t, LPlayerPtr>> m_player_list;//左右两边的排列的6个玩家: 第一个大富豪，第二个神算子，后面4个每局随机变化两个;

public:
	Tfield<double>::TFieldPtr		WinnerEarningsRate;	//赢钱抽水
	Tfield<double>::TFieldPtr		LostEarningsRate;		//输钱抽水
	Tfield<int64_t>::TFieldPtr		EnterCount;		//进入次数
	Tfield<int16_t>::TFieldPtr		PlayerCount;	    //当前玩家数
	Tfield<double>::TFieldPtr		TotalStock;		//总库存
	Tfield<double>::TFieldPtr		TotalTaxWinner;	//赢的税收
	Tfield<double>::TFieldPtr		TotalTaxLost;		//输的税收

	int  get_current_warter();//获取当前房间的水位
	void set_total_stock_value(int value) { TotalStock->set_value(value); }
	int  get_total_stock_value()		   { return TotalStock->get_value(); }
	void add_total_stock_value(int value) { TotalStock->add_value(value);}

private:
	uint16_t m_room_id;
	uint16_t m_child_id;
	int32_t m_cut_round;		    //每100局杀分次数;
	bool m_kill_points_switch;
	int32_t m_server_stauts;
	double m_sync_palyer_times;
};


DRAGON_RED_BLACK_END
