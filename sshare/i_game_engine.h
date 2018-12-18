#pragma once
#include <i_game_def.h>
#include <enable_xml_config.h>


//每个游戏必须实现的引擎
class i_game_engine
{
public:
	i_game_engine();
	virtual ~i_game_engine();

	//初始化引擎
	virtual bool init_engine( enable_xml_config& config) = 0;

	//每帧调用
	virtual void heartbeat( double elapsed ) = 0;

	//退出引擎
	virtual void exit_engine() = 0;

	//////////////////////////////////////////////////////////////////////////
	//服务器通知游戏逻辑
	//玩家进入游戏
	virtual bool player_enter_game(iGPlayerPtr igplayer) = 0;

	//玩家离开游戏
	virtual void player_leave_game(uint32_t playerid) = 0;
	
	//@踢人下线;
	//@comment by hunter 2017/11/02;
	//@ bforce =0 ,玩家正常离开；
	//@ bforce =1,当前游戏局结束将其提出房间,需要调用正常写分流程;
	//@ bforce =2,立马踢出,游戏按逃跑处理;
	virtual void player_kick_player(uint32_t playerid, int bforce = 0) = 0;

	//@add by Hunter 2017/11/03;
	//@gm管理消息回调;
	//@opttype == 10杀分 opttype == 100停服,gameID与游戏类型对应每次都有值,rootID 如果为0,则此游戏对应的所有子房间全部开杀,如果非零则对应某个子房间;
	virtual void gmPlatformOpt(int32_t optype, int32_t gameID, int32_t roomID = 0,
		int32_t cutRound =20 ,int32_t exData1 =0 , int32_t exData2 =0	) =0;

	//玩家进入好友的桌子
	virtual int player_join_friend_game(iGPlayerPtr igplayer, uint32_t friendid) = 0;

	virtual uint16_t get_gameid() =0;

	//返回一个机器人 返回的机器人未进入房间？
	virtual void response_robot(int32_t playerid, int tag) = 0;
public:
	//要在init_engine之前调用
	void set_handler(i_game_ehandler* ehandler);
	i_game_ehandler* get_handler();
private:
	i_game_ehandler* m_ehandler;

};
