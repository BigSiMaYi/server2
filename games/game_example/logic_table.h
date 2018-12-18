#pragma once
#include "logic_def.h"
#include "i_game_def.h"
#include <vector>


EXAMPLE_SPACE_BEGIN


class logic_table: public enable_obj_pool<logic_table>
	,public game_object
{
public:
	logic_table(void);
	virtual ~logic_table(void);

	void init_talbe(uint16_t tid, logic_room* room);
	virtual uint32_t get_id();
	void heartbeat( double elapsed );

	int enter_table(LPlayerPtr player);//进入桌子
	void leave_table(uint32_t pid);//离开桌子
	//bool change_op(uint32_t pid);//坐下->观战
	bool change_sit(uint32_t pid, uint32_t seat_index);//改变座位
	bool is_full();

	unsigned int get_max_table_player();
	LPlayerPtr& get_player(int index);
	LPlayerPtr& get_player_byid(uint32_t pid);

	logic_room* get_room();
public:
	//广播协议，立刻发送
	template<class T>
	int broadcast_msg_to_client(T msg, uint32_t except_id = 0)
	{
		std::vector<uint32_t> pids;
		for (unsigned int i = 0; i < m_players.size(); i++)
		{
			if (m_players[i] != nullptr && m_players[i]->get_pid() != except_id)
			{
				pids.push_back(except_id);
			}
		}
		return broadcast_msg_to_client(pids, msg->packet_id(), msg);
	};
	int broadcast_msg_to_client(std::vector<uint32_t>& pids, uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg);
	
	//广播协议,先将协议发送到缓存,然后发送
	template<class T>
	void add_msg_to_list(T msg)
	{
		m_msglist.push_back(msg_packet_one(msg->packet_id(), msg));
	};
	//boost::shared_ptr<fish_protocols::packetl2c_get_scene_info_result> get_scene_info_msg();
protected:
	void broadcast_msglist_to_client();
	//进入座位
	void bc_enter_seat(int seat_index, LPlayerPtr& player);
	//离开座位
	void bc_leave_seat(int player_id);
private:
	
	logic_room* m_room;

	std::vector<LPlayerPtr> m_players;
	uint16_t m_player_count;
	void inc_dec_count(bool binc = true);

	double m_elapse;
	std::vector<msg_packet_one> m_msglist;
	double m_checksave;

	//////////////////////////////////////////////////////////////////////////
	void create_table();
	bool load_table();
public:
	virtual void init_game_object();//注册属性
	virtual bool store_game_object(bool to_all = false);//非数组对象必须实现此接口
	Tfield<int16_t>::TFieldPtr		TableID;			//桌子id
	Tfield<int64_t>::TFieldPtr		TotalIncome;		//当前收入
	Tfield<int64_t>::TFieldPtr		TotalOutlay;		//当前消耗

	void add_income(int score);
	void add_outlay(int score);
};


EXAMPLE_SPACE_END
