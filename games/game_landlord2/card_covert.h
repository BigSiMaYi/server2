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
    void SendCard2(int32_t seatid, std::vector<POKER>& pokers, bool is_robot); //1����;
    int   CalcGetLandScore(int32_t seatid); //��ȡ�е�������;
    void GetLandlord(int pid, int score); //2.�е���;

    void SendLandlordCard(int32_t seatid, std::vector<POKER>& pokers);          //3.��������;

    bool PlayerDiscard(int seatid, const std::vector<POKER>& pokers); //4. ��ҳ�����;
    bool Discard(int32_t& seatid, int32_t& next_cid, std::vector<POKER>& discard, bool is_auto = true);
    bool IsPass(int32_t seatid);
    int32_t Pass(int32_t seatid);

    bool GameOver(void);        //��Ϸ����;

    void  SetRobot(int seatid, bool flag);

	void  SetCurrentPlayer(int seatid);
	void  SetLastPlayer(int seatid);

public:
    void GetCardGroupBomb(std::vector<POKER>& pokers, int32_t& bombsize, int32_t& groupsize);
    bool CheckPokerValue(std::vector<POKER>& pokers);
    static int CalcCardsWight(std::vector<POKER>& pokers);
    void  MixRobotCards(std::vector<POKER>& pokers1, std::vector<POKER>& pokers2);
    void  MakeDivide(std::vector<POKER>& pokers1, std::set<int>& cards, std::map<int, int>& needanalyse);

protected:
    void Init(void);//��ʼ����ؽṹ;
    void SetPlayerRelation(int seatid); //����ÿ����ҵ��ϼҺ��¼�;
    bool CheckSeat(int seatid);

    static int covert(const POKER& poker);
    static POKER covert(int num);
    
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

void poker_to_stringstream(const POKER& p, std::stringstream& sm);
std::string poker_to_string(const std::vector<POKER>& p);