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

	//��ȡ�����������
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
	virtual void init_game_object();//ע������
	virtual bool store_game_object(bool to_all = false);//������������ʵ�ִ˽ӿ�
	Tfield<int16_t>::TFieldPtr		RoomID;			//����id
	Tfield<double>::TFieldPtr		EarningsRate;	//ӯ����
	Tfield<int64_t>::TFieldPtr		TotalIncome;		//��ǰ����
	Tfield<int64_t>::TFieldPtr		TotalOutlay;		//��ǰ����
	Tfield<int64_t>::TFieldPtr		EnterCount;		//�������
};


EXAMPLE_SPACE_END
