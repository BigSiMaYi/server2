#pragma once
#include <i_game_def.h>
#include <vector>
class db_base;

//游戏引擎回调到服务器接口
class i_game_ehandler
{
public:
	i_game_ehandler();
	virtual ~i_game_ehandler();

	//游戏引擎初始化完成（仅成功调用）
	virtual void on_init_engine(uint16_t game_id, const std::string& game_ver ,int32_t room_id=-1,int32_t roomchild_id=-1,uint16_t max_player =600) = 0;

	//服务器关闭 游戏引擎退出
	virtual void on_exit_engine() = 0;

	//请求机器人 请求的机器人不一定及时返回  
	//要求的vip 要求的gold  自定义标志tag
	virtual void request_robot(int tag, GOLD_TYPE needgold, int needvip = 0) = 0;
	
	//当不需要使用机器人时 只要退出到房间选择然后调用此函数
	virtual void release_robot(int playerid) = 0;

	//获取当前机器人数
	virtual int get_robot_count() = 0;

	template<class T>
	int broadcast_msg_to_client(std::vector<uint32_t>& pids,T msg)
	{
		return broadcast_msg_to_client(pids, msg->packet_id(), msg);
	};

	//发送协议到客户端
	virtual int broadcast_msg_to_client(std::vector<uint32_t>& pids, uint16_t packet_id, boost::shared_ptr<google::protobuf::Message> msg) = 0;

	//发送协议列表
	virtual int broadcast_msglist_to_client(std::vector<uint32_t>& pids, std::vector<msg_packet_one>& msglist) = 0;

	virtual bool setGameDb(db_base *dgGame) = 0;

	// 返回当前的局数id
	virtual int64_t getCurId(const std::string& key) = 0;
	//@add by hunter 2017/08/01;
	//@比赛发送奖励;
	virtual bool sendToWorld(uint16_t packet_id,boost::shared_ptr<google::protobuf::Message> msg) = 0;
	
	//@更新用户游戏状态接口;
	//@用户游戏状态列表;
	//@param pgameStatus  同如下enume;
	//enum e_p_game_state {
	//	e_p_state_free,		//@用户空闲;	
	//	e_p_state_ready,		//@用户准备;
	//	e_p_state_playing,		//@游戏过程中;
	//	e_p_state_end,			//@结算，结束状态;
	//	e_p_state_standby,		//@旁观用户;
	//};
	virtual bool sync_userGameStatus(int32_t pid ,int32_t gameID, int32_t roomID, int32_t tableID, int32_t chairID, int32_t pgameStatus) =0;
	
	//@写日志接口;
	virtual int sendLogInfo(const std::string & info ,const int32_t gameID =0) = 0;
	//@获得对应房间税点;
	virtual int32_t GetRoomCommissionRate(int32_t roomID) =0;
	//@同步游戏人数接口;
	virtual int32_t syncRoomPcnt(int32_t curRoomCnt, int32_t roomID) = 0;
};

