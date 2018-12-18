#pragma once
#include "logic_def.h"
#include <i_game_phandler.h>
#include "i_game_player.h"
#include "robot_mgr.h"

LANDLORD_SPACE_BEGIN

enum quest_type
{
	game_count = 1301,
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
	virtual void quest_change_from_world(int quest_type,int count,int param) ;
	//��ҵ���
	virtual void on_change_state();

	//////////////////////////////////////////////////////////////////////////
	//�뿪��Ϸʱ����
	void enter_game(logic_lobby* lobby);
	bool join_table(logic_table* table,int32_t seat=0);
	logic_table* get_table();
	int32_t get_seat() const;
	void leave_table();
	bool can_leave_table();

	void set_status(int state);
	int32_t get_status();
	
	void clear_round(int round=0);
	void add_round(int round=1);
	int32_t get_round();

	void set_wait(int wait);
	void add_wait(int wait=1);
	int32_t get_wait();

	bool onEventUserReady();
	//////////////////////////////////////////////////////////////////////////
	void release();//�˳�������Ϸ

	// ��ȡ���ID
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
	// д��
	void write_property(int64_t value,int64_t tax = 0,const std::string& param="");
	//�㲥
	void bc_game_msg(int money, const std::string& sinfo, int mtype = 1);

	void add_star_lottery_info(int32_t award,int32_t star = 0);
	void quest_change(int questid, int count=1, int param=0);

	template<class T>
	int send_msg_to_client(T msg)
	{
#if 1
		return m_player->send_msg_to_client(msg);
#else
		if(!is_robot())
		{
			return m_player->send_msg_to_client(msg);
		}
		else
		{
			robot_mgr::instance().recv_packet(get_pid(), msg->packet_id(), msg);
		}
		return 1;
#endif
	};

	int send_packet(uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg);
    void reqPlayer_leaveGame();

private:
	logic_lobby* m_lobby;
	logic_room* m_room;
	logic_table* m_table;

	LRobotPtr m_robot;

	GOLD_TYPE m_logic_gold;	
	GOLD_TYPE m_change_gold;

	Tfield<int32_t>::TFieldPtr m_win_count;    //���ǳ齱�ۼ�ӮǮ���� 

	Tfield<int32_t>::TFieldPtr m_nChairID;    

	Tfield<int32_t>::TFieldPtr m_nPlayerStatus;  

	Tfield<int32_t>::TFieldPtr m_nPlayRound;  

	Tfield<int32_t>::TFieldPtr m_nWaitRound;
	//////////////////////////////////////////////////////////////////////////
	void create_player();
	bool load_player();	
public:
	virtual void init_game_object();//ע������
	virtual bool store_game_object(bool to_all = false);//������������ʵ�ִ˽ӿ�

};

LANDLORD_SPACE_END
