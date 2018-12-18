#pragma once
#include <i_game_def.h>
#include <enable_xml_config.h>


//ÿ����Ϸ����ʵ�ֵ�����
class i_game_engine
{
public:
	i_game_engine();
	virtual ~i_game_engine();

	//��ʼ������
	virtual bool init_engine( enable_xml_config& config) = 0;

	//ÿ֡����
	virtual void heartbeat( double elapsed ) = 0;

	//�˳�����
	virtual void exit_engine() = 0;

	//////////////////////////////////////////////////////////////////////////
	//������֪ͨ��Ϸ�߼�
	//��ҽ�����Ϸ
	virtual bool player_enter_game(iGPlayerPtr igplayer) = 0;

	//����뿪��Ϸ
	virtual void player_leave_game(uint32_t playerid) = 0;
	
	//@��������;
	//@comment by hunter 2017/11/02;
	//@ bforce =0 ,��������뿪��
	//@ bforce =1,��ǰ��Ϸ�ֽ��������������,��Ҫ��������д������;
	//@ bforce =2,�����߳�,��Ϸ�����ܴ���;
	virtual void player_kick_player(uint32_t playerid, int bforce = 0) = 0;

	//@add by Hunter 2017/11/03;
	//@gm������Ϣ�ص�;
	//@opttype == 10ɱ�� opttype == 100ͣ��,gameID����Ϸ���Ͷ�Ӧÿ�ζ���ֵ,rootID ���Ϊ0,�����Ϸ��Ӧ�������ӷ���ȫ����ɱ,����������Ӧĳ���ӷ���;
	virtual void gmPlatformOpt(int32_t optype, int32_t gameID, int32_t roomID = 0,
		int32_t cutRound =20 ,int32_t exData1 =0 , int32_t exData2 =0	) =0;

	//��ҽ�����ѵ�����
	virtual int player_join_friend_game(iGPlayerPtr igplayer, uint32_t friendid) = 0;

	virtual uint16_t get_gameid() =0;

	//����һ�������� ���صĻ�����δ���뷿�䣿
	virtual void response_robot(int32_t playerid, int tag) = 0;
public:
	//Ҫ��init_engine֮ǰ����
	void set_handler(i_game_ehandler* ehandler);
	i_game_ehandler* get_handler();
private:
	i_game_ehandler* m_ehandler;

};
