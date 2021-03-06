#pragma once

#include "card_analyse.h"
#include "card_def.h"
#include <memory>

//游戏进度状态;
enum Status
{
    NOTSTART,                       //游戏未开始;
    GETLANDLORD,                //叫地主阶段;
    SENDLANDLORDCARD,   //发地主牌阶段;
    DISCARD,                         //出牌阶段;
    GAMEOVER                      //游戏结束;
};

class card_covert
{
public:
    card_covert();
    ~card_covert();

public:
    char GetStatus(void);//获取当前游戏进度状态;
    void Start(void);//开始新游戏;

public:
    void SendCard(int32_t seatid, std::vector<POKER>& pokers, bool is_robot); //1发牌;
    int   CalcGetLandScore(int32_t seatid); //获取叫地主分数;
    void GetLandlord(int pid, int score); //2.叫地主;

    void SendLandlordCard(std::vector<POKER>& pokers);          //3.发地主牌;

    void PlayerDiscard(int seatid, const std::vector<POKER>& pokers); //4. 玩家出的牌;
    bool Discard(int32_t& seatid, int32_t& next_cid, std::vector<POKER>& discard);
    bool IsPass(int32_t seatid);
    int32_t Pass(int32_t seatid);

    bool GameOver(void);        //游戏结束;

protected:
    void Init(void);//初始化相关结构;
    void SetPlayerRelation(int seatid); //设置每个玩家的上家和下家;
    int covert(const POKER& poker);
    POKER covert(int num);

private:
    Status status_;              //游戏进度;
    card_analyse* player_[3];         //真人玩家编号为0;
    card_analyse* landlord_;          //地主：叫地主时产生;
    card_analyse* curplayer_;         //当前出牌玩家;
    card_analyse* lastone_;            //最后出牌方;

    int callscore_[3];             //各家叫地主的分数;
    int callbegin_;                 //第一个叫地主的玩家;
    int basescore_;               //本局基本分;
    int times_;                      //本局倍率;
    int questioned_;            //已询问数量;
};

