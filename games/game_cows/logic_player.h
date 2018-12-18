#pragma once
#include "logic_def.h"
#include <i_game_phandler.h>
#include "i_game_player.h"

COWS_SPACE_BEGIN

enum quest_type
{
	game_count = 401,
	banker_win_count,
	win_gold,
	cards_win,
};

class logic_player: 
	public enable_obj_pool<logic_player>
	,public i_game_phandler
	,public game_object
{
public:
	logic_player(void);
	virtual ~logic_player(void);

	void heartbeat( double elapsed );

	void init(iGPlayerPtr player);

	//////////////////////////////////////////////////////////////////////////
	//�ӷ�����֪ͨ�߼��Ľӿ�
	//����
	virtual void on_attribute_change(int atype, int v);
	virtual void on_attribute64_change(int atype, GOLD_TYPE v = 0);
	virtual void quest_change_from_world(int quest_type,int count,int param);

	//��ҵ���
	virtual void on_change_state();

	//////////////////////////////////////////////////////////////////////////
	//�뿪��Ϸʱ����
	void enter_game(logic_lobby* lobby);

	logic_room* get_room();
	bool enter_room(logic_room* room);
	void leave_room();
	int can_leave_room();

	void release();//�˳�������Ϸ

	bool is_offline();
	uint32_t get_pid();
	//��ȡ��ҵ�ǰ���(��Ϸ����)
	GOLD_TYPE get_gold();
	//��ȡ���VIP�ȼ�
	int16_t get_viplvl();
	//��ȡ��ȯ
	int get_ticket();
	//��ȡ�ǳ�
	const std::string& get_nickname();
	//�Ա�
	int get_sex();
	//ͷ��
	int get_photo_frame();
	//ͷ��
	const std::string& get_icon_custom();
	//������Ϣ
	const std::string& GetUserRegion();
	//�Ƿ������
	bool is_robot();
	//�������߼�
	LRobotPtr& get_robot();
	//�����������߼�
	void create_robot();

	//�ı���(��Ϸ����)
	bool change_gold(GOLD_TYPE v, bool needbc = false);
	//�ı���ȯ(��Ϸ����)
	bool change_ticket(int v, int season);
	bool change_gold2(int v, int season);
	//ͬ�����
	void sync_change_gold();
	//�㲥
	void bc_game_msg(int money, const std::string& sinfo, int mtype = 1);

	bool check_bet_gold(GOLD_TYPE bet_gold, GOLD_TYPE total_gold);
	void add_star_lottery_info(int32_t award,int32_t star = 0);
	void quest_change(int questid, int count=1, int param=0);

	void set_self_is_bet(bool is_bet){m_self_is_bet=is_bet;}
	bool get_self_is_bet(){return m_self_is_bet;}

	void set_self_win_gold(GOLD_TYPE win_gold){m_self_win_gold=win_gold;}
	GOLD_TYPE get_self_win_gold(){return m_self_win_gold;}

	//����˰�գ��м�������: ÿ����ע������Ӯ�ܺͺ��˰;
	void  set_tax(GOLD_TYPE tax);
	//���ּ�¼��ǰ������Ͻ��;
	GOLD_TYPE get_pre_gold();
	void record_gold();
	GOLD_TYPE get_tax();
	void  set_kick_status(int bforce);
	int get_kick_status();
		
	void set_otherplayers(std::vector<LPlayerPtr> other_players)
	{
		m_other_players.clear();
		m_other_players=other_players;
	}
	std::vector<LPlayerPtr>& get_otherplayers(){return m_other_players;}

	template<class T>
	int send_msg_to_client(T msg)
	{
		return m_player->send_msg_to_client(msg);
	};
private:
	logic_lobby* m_lobby;
	logic_room* m_room;

	LRobotPtr m_robot;
	GOLD_TYPE m_pre_logic_gold; //����ʱ���ϵ�Ǯ;
	GOLD_TYPE m_logic_gold;	
	GOLD_TYPE m_change_gold;
	GOLD_TYPE m_tax_gold;

	bool m_self_is_bet;
	GOLD_TYPE m_self_win_gold;

	std::vector<LPlayerPtr> m_other_players;

	Tfield<int32_t>::TFieldPtr m_win_count;    //���ǳ齱�ۼ�ӮǮ����   
	int m_bforce;

	//////////////////////////////////////////////////////////////////////////
	void create_player();
	bool load_player();	
public:
	virtual void init_game_object();//ע������
	virtual bool store_game_object(bool to_all = false);//������������ʵ�ִ˽ӿ�

	void on_turret_change();
};

COWS_SPACE_END