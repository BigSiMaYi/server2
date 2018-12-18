#pragma once

#include "card_analyse.h"
#include "card_def.h"
#include <memory>

//��Ϸ����״̬;
enum Status
{
    NOTSTART,                       //��Ϸδ��ʼ;
    GETLANDLORD,                //�е����׶�;
    SENDLANDLORDCARD,   //�������ƽ׶�;
    DISCARD,                         //���ƽ׶�;
    GAMEOVER                      //��Ϸ����;
};

class card_covert
{
public:
    card_covert();
    ~card_covert();

public:
    char GetStatus(void);//��ȡ��ǰ��Ϸ����״̬;
    void Start(void);//��ʼ����Ϸ;

public:
    void SendCard(int32_t seatid, std::vector<POKER>& pokers, bool is_robot); //1����;
    int   CalcGetLandScore(int32_t seatid); //��ȡ�е�������;
    void GetLandlord(int pid, int score); //2.�е���;

    void SendLandlordCard(std::vector<POKER>& pokers);          //3.��������;

    void PlayerDiscard(int seatid, const std::vector<POKER>& pokers); //4. ��ҳ�����;
    bool Discard(int32_t& seatid, int32_t& next_cid, std::vector<POKER>& discard);
    bool IsPass(int32_t seatid);
    int32_t Pass(int32_t seatid);

    bool GameOver(void);        //��Ϸ����;

protected:
    void Init(void);//��ʼ����ؽṹ;
    void SetPlayerRelation(int seatid); //����ÿ����ҵ��ϼҺ��¼�;
    int covert(const POKER& poker);
    POKER covert(int num);

private:
    Status status_;              //��Ϸ����;
    card_analyse* player_[3];         //������ұ��Ϊ0;
    card_analyse* landlord_;          //�������е���ʱ����;
    card_analyse* curplayer_;         //��ǰ�������;
    card_analyse* lastone_;            //�����Ʒ�;

    int callscore_[3];             //���ҽе����ķ���;
    int callbegin_;                 //��һ���е��������;
    int basescore_;               //���ֻ�����;
    int times_;                      //���ֱ���;
    int questioned_;            //��ѯ������;
};

