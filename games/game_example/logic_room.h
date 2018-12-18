#pragma once
#include "logic_def.h"

//struct Fish_RoomCFGData;

EXAMPLE_SPACE_BEGIN

class logic_room :public game_object
{
public:
	logic_room(/*const Fish_RoomCFGData* cfg,*/ logic_lobby* _lobby);
	~logic_room(void);

	void heartbeat( double elapsed );

	virtual uint32_t get_id();
	uint16_t get_cur_cout();

	bool has_seat(uint16_t& tableid);

	int enter_table(LPlayerPtr player, uint16_t tid);
	void on_leave_table();

	void check_rate(uint32_t& rate);
	int get_max_rate();
	int get_min_rate();

	//const Fish_RoomCFGData* get_data() const;

	double get_earnings_rate();

	logic_lobby*  get_lobby(){return m_lobby;};

	//获取房间道具消耗
	int get_item_consume(int itemid);
private:
	//const Fish_RoomCFGData* m_cfg;
	LTABLE_MAP m_tables;
	uint16_t m_player_count;
	logic_lobby* m_lobby;

	double m_check_rate;
	void reflush_rate();
	std::string m_key;

	//////////////////////////////////////////////////////////////////////////
	void create_room();
	bool load_room();
public:
	virtual void init_game_object();//注册属性
	virtual bool store_game_object(bool to_all = false);//非数组对象必须实现此接口
	Tfield<int16_t>::TFieldPtr		RoomID;			//房间id
	Tfield<double>::TFieldPtr		EarningsRate;	//盈利率
	Tfield<int64_t>::TFieldPtr		TotalIncome;		//当前收入
	Tfield<int64_t>::TFieldPtr		TotalOutlay;		//当前消耗
	Tfield<int64_t>::TFieldPtr		EnterCount;		//进入次数
};


EXAMPLE_SPACE_END
