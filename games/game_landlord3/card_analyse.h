#pragma once

#include "card_group.h"

#include <vector>
#include <map>

class card_covert;

class card_analyse
{
    friend card_covert;

public:
    card_analyse(int id);

    void NewGame(void);//开始新的一局，做一些初始化集合等的操作
    void ClearAnalyse(void);//清空分析牌集合
    int    GetBaseScore(int questioned, int nowscore);//本局是否想当地主，并给出基本分
    void AddCard(int num)//抹牌;
    {
        cards_.insert(num);
    }

    bool IsLandlord()
    {
        return islandlord_;
    }
    void SetLandlord(bool tag)
    {
        islandlord_ = tag;
    }
    void SetRobot(bool type)
    {
        type_ = type;
    }
    bool IsRobot()
    {
        return type_;
    }

    void SetPlayer(card_analyse* preplayer, card_analyse* nextplayer)
    {
        preplayer_ = preplayer;
        nextplayer_ = nextplayer;
    }

    int    GetRemain(void) { return cards_.size(); }//剩余牌数
    bool IsValid(card_analyse* lastone);//判断选择牌是否合格
    void AnalyseSelection(void);//分析选择牌类型及总权值
    void DivideIntoGroups(void); //分析并拆分牌型
    void ThreeplusAndAirplane(void);//从分析后的基本牌型中组合三带一和飞机
    void DeleteUnkown(void);//删除牌型集合中未知类型
    void SelectCards(card_analyse* lastone, bool hint = false);//AI选牌
    CardGroup& GetSelectCards() { return selection_; }

    void Myself(CardGroup& selection);//直接出牌
    void Friend(CardGroup& lastdiscard, CardGroup& selection);//跟友方牌
    void Enemy(CardGroup& lastdiscard, CardGroup& selection, bool hint);//跟敌方牌
    void NeedSigle(CardGroup& lastdiscard, CardGroup& selection);//拆出单张
    void NeedDouble(CardGroup& lastdiscard, CardGroup& selection);
    void NeedSigleSeq(CardGroup& lastdiscard, CardGroup& selection);
    void NeedThreePlus(CardGroup& lastdiscard, CardGroup& selection);
    void NeedAirplane(CardGroup& lastdiscard, CardGroup& selection);

    bool Discard(void); //AI出牌
    bool HumanDiscard(card_analyse* lastone);//玩家出牌
    bool DiscardAndClear();//出牌并重置相应结构
                           //void Hint(void); //提示牌
    void Pass(void);//过牌，重置相应结构
                    //给定权值，从集合中查找相应0-53数字，然后从集合中删除并返回该数字；不存在或无效返回-1
    int ValueToNum(std::set<int> &cardscopy, int value);
    void FreshenMap(std::map<int, int> &m);//删除分析堆中数量为零的元素
    static bool MyCompare(CardGroup *c1, CardGroup *c2);//对分析后牌集合排序的回调函数

private:
    card_analyse* preplayer_;
    card_analyse* nextplayer_;
    int id_;
    bool islandlord_;
    bool   type_;                                          //类型：0：真人，非0：机器人;

    bool test_;                                            //是否试过送下家走;
    bool nodiscard_;                                  //不出标志;
    int score_;                                             //玩家当前分数;
    std::set<int> cards_;                            //手牌;
    std::vector<CardGroup*> analyse_;    //分析后拆分的牌型集合;
    CardGroup selection_;                         //选择牌的集合;
    CardGroup discard_;                            //打出的牌的集合;
};