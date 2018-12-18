
#pragma once
#include "logic_def.h"
#include <i_game_phandler.h>
#include <i_game_player.h>

EXAMPLE_SPACE_BEGIN


class logic_player: 
	public enable_obj_pool<logic_player>
	,public i_game_phandler
	,public game_object
{
public:
	logic_player(void);
	virtual ~logic_player(void);

	void heartbeat( double elapsed );

	//////////////////////////////////////////////////////////////////////////
	//�ӷ�����֪ͨ�߼��Ľӿ�
	//����
	virtual void on_attribute_change(int atype, int v);

	virtual void on_attribute64_change(int atype, GOLD_TYPE v = 0) {};
	//��ҵ���
	virtual void on_offline();

	virtual void on_change_state(void){};

	//////////////////////////////////////////////////////////////////////////
	//�뿪��Ϸʱ����
	void enter_game(logic_lobby* lobby);
	bool join_table(logic_table* table);
	void leave_table();
	void release();//�˳�������Ϸ

	uint32_t get_pid();
	//��ȡ��ҵ�ǰ���(��Ϸ����)
	int get_gold();
	//��ȡ���VIP�ȼ�
	int16_t get_viplvl();

	//�Ƿ�����VIP
	bool is_ExperienceVIP();

	//��ȡ��ȯ
	int get_ticket();

	//�ı���(��Ϸ����)
	bool change_gold(int v, bool needbc = false);

	//�ı���ȯ(��Ϸ����)
	bool change_ticket(int v, bool needbc = false);

	//��ȡ�ǳ�
	const std::string& get_nickname();


	int get_sex();
	int get_photo_frame();
	const std::string& get_icon_custom();

	logic_table* get_table();

	void addexp(int exp);

	void bc_game_msg(int money, const std::string& sinfo, int mtype = 1);

	void init_tickettime();
	int check_ticket();//���ÿ������ȯ


	template<class T>
	int send_msg_to_client(T msg)
	{
		return m_player->send_msg_to_client(msg->packet_id(), msg);
	};
private:
	logic_table* m_table;
	logic_lobby* m_lobby;

	int32_t m_logic_gold;	
	int32_t m_change_gold;
	int32_t m_log_gold;
	double m_check_sync;
	double m_check_ticket;
	double m_cur_tickettime;

	bool m_isRobot;

	//////////////////////////////////////////////////////////////////////////
	void create_player();
	bool load_player();	
public:
	virtual void init_game_object();//ע������
	virtual bool store_game_object(bool to_all = false);//������������ʵ�ִ˽ӿ�
		
	Tfield<int32_t>::TFieldPtr		KillFishCount;		//ɱ����
	Tfield<int32_t>::TFieldPtr		KillFishScore;		//ɱ�����
	Tfield<int16_t>::TFieldPtr		Level;				//�ȼ�
	Tfield<int32_t>::TFieldPtr		Exp;				//����

	Tfield<int32_t>::TFieldPtr		TodayTicket;			//���ջ�ȡ��ȯ
	Tfield<time_t>::TFieldPtr		LastGetTicket;			//����ȡ��ȯ����
	Tfield<bool>::TFieldPtr			ReceiveTicket;			//�״���ȯ
	//////////////////////////////////////////////////////////////////////////
};
EXAMPLE_SPACE_END
