#pragma once
#include "logic_def.h"
#include <i_game_phandler.h>
#include "i_game_player.h"

DRAGON_RED_BLACK_BEGIN

enum quest_type
{
	game_count = 401,
	banker_win_count,
	win_gold,
	cards_win,
};

class logic_player
	: public enable_obj_pool<logic_player>
	, public i_game_phandler
	, public game_object
{
public:
	logic_player(void);
	virtual ~logic_player(void);

public:
	void init(iGPlayerPtr player);
	void release();//�˳�������Ϸ;

	void heartbeat( double elapsed );

public:
	//�ӷ�����֪ͨ�߼��Ľӿ�;
	virtual void on_attribute_change(int atype, int v);
	virtual void on_attribute64_change(int atype, GOLD_TYPE v = 0);
	virtual void on_change_state();//��ҵ���;

	virtual int  cltReq_leaveGame();
public:
	virtual void init_game_object();//ע������;
	virtual bool store_game_object(bool to_all = false);//������������ʵ�ִ˽ӿ�;

public:
	//�뿪��Ϸʱ����;
	void enter_game(logic_lobby* lobby);

	bool enter_room(logic_room* room);
	void leave_room();
	bool can_leave_room();

	bool is_offline();
	bool is_robot();//�Ƿ������;
	void create_robot();//�����������߼�;

public:
	bool change_ticket(int v, int season);//�ı���ȯ(��Ϸ����);
	bool change_gold(GOLD_TYPE v, bool needbc = false);//�ı���(��Ϸ����);
	bool change_gold2(int v, int season);

	void sync_change_gold();//ͬ�����;

	void store_balance_record();
	void clear_balace_info();

	bool check_bet_gold(GOLD_TYPE bet_gold, GOLD_TYPE total_gold);
	void add_star_lottery_info(int32_t award, int32_t star = 0);
	void quest_change(int questid, int count = 1, int param = 0);
	bool check_room_gold(GOLD_TYPE& room_gold);

public:
	logic_room* get_room();
	int get_roomid();

	uint32_t get_pid();
	GOLD_TYPE get_gold();//��ȡ��ҵ�ǰ���(��Ϸ����);
	int16_t get_viplvl();//��ȡ���VIP�ȼ�;
	int get_ticket();//��ȡ��ȯ;
	const std::string& get_nickname();//��ȡ�ǳ�;
	const std::string& get_region();
	int get_sex();//�Ա�;
	int get_photo_frame();//ͷ��;
	const std::string& get_icon_custom();//ͷ��;
	LRobotPtr& get_robot();//�������߼�;

	GOLD_TYPE get_bet_gold();//���ؽ�20�ֵ���Ѻע���;
	int      get_winner_count();//���ؽ�20�ֵ���Ӯ�ľ���;

	int  get_banker_count() { return m_record_banker_count; }//��ׯ����ͳ��;
	void add_banker_count(int add_count) { m_record_banker_count += add_count; }
	void reset_banker_count() { m_record_banker_count = 1; }

	void  set_kick_status(int bforce) { m_bforce = bforce; }
	int     get_kick_status() { return m_bforce; }
	void  set_leave_status(int leave_status) { m_leave_status = leave_status; }
	int     get_leave_status() { return m_leave_status; }

	std::list<GOLD_TYPE> & get_bet_gold_list();
	std::list<bool>& get_winner_list();

	//���ּ�¼��ǰ������Ͻ��;
	GOLD_TYPE get_pre_gold() {return m_pre_logic_gold;}
	void record_gold() { m_tax_gold = 0; m_pre_logic_gold = m_logic_gold; }
	GOLD_TYPE get_tax() { return m_tax_gold; }
	void  set_tax(GOLD_TYPE tax) { m_tax_gold = tax; }
public:
	void bc_game_msg(int money, const std::string& sinfo, int mtype = 1);//�㲥;

	template<class T>
	int send_msg_to_client(T msg)
	{
		return m_player->send_msg_to_client(msg);
	};

protected:
	void create_player();
	bool load_player();

private:
	PROPERTY_DEFINE(GOLD_TYPE, balance_bet_gold, private);//�����ܹ�ѹ��Ǯ;
	PROPERTY_DEFINE(int, balance_win_or_lose, private);//������Ӯ������;
	PROPERTY_DEFINE(int, win_lose_final_gold, private);//��������Ӯ��Ǯ;
	PROPERTY_DEFINE(bool, balance_bet, private);//�����ǲ���ѹע;
	PROPERTY_DEFINE(bool, robot_bet, private);//ֻѹһ��

	Tfield<int32_t>::TFieldPtr m_win_count;    //���ǳ齱�ۼ�ӮǮ����;

private:
	logic_lobby* m_lobby;
	logic_room* m_room;
	LRobotPtr m_robot;

	GOLD_TYPE m_logic_gold;	
	GOLD_TYPE m_change_gold;
	GOLD_TYPE m_tax_gold;

	GOLD_TYPE m_pre_logic_gold; //����ʱ���ϵ�Ǯ;

	int      m_record_banker_count;

	std::list<GOLD_TYPE>		m_list_bet_gold;
	std::list<bool>			m_list_winner;
	int m_leave_status;
	int m_bforce;
};

DRAGON_RED_BLACK_END
