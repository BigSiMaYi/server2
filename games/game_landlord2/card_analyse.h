#pragma once

#include "card_group.h"

#include <vector>
#include <map>
#include <memory>

class card_covert;

class card_analyse
{
    friend card_covert;

public:
    card_analyse(int id);
	~card_analyse(void);

    void NewGame(void);//��ʼ�µ�һ�֣���һЩ��ʼ�����ϵȵĲ���
    void ClearAnalyse(void);//��շ����Ƽ���
    int    GetBaseScore(int questioned, int nowscore);//�����Ƿ��뵱������������������;
    static int    CalcCardsWight(std::set<int>& cards);

    void AddCard(int num)//Ĩ��;
    {
        cards_.insert(num);
    }
    void AddCards(std::set<int>& cards)//Ĩ��;
    {
        cards_ = cards;
    }
    bool IsLandlord()
    {
        return islandlord_;
    }
    void SetLandlord(bool tag)
    {
        islandlord_ = tag;
    }
    void SetAutoDiscard(bool type)
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

    int    GetRemain(void) { return cards_.size(); }//ʣ������
    bool IsValid(card_analyse* lastone);//�ж�ѡ�����Ƿ�ϸ�
    void AnalyseSelection(void);//����ѡ�������ͼ���Ȩֵ
    void DivideIntoGroups(void); //�������������
    void ThreeplusAndAirplane(void);//�ӷ�����Ļ����������������һ�ͷɻ�
    void DeleteUnkown(void);//ɾ�����ͼ�����δ֪����
    void SelectCards(card_analyse* lastone, bool hint = false);//AIѡ��
    void AddDiscards(std::set<int>& nums);

    CardGroup& GetSelectCards() { return selection_; }

    void Myself(CardGroup& selection);//ֱ�ӳ���
    void Friend(card_analyse* lastone, const CardGroup& lastdiscard, CardGroup& selection);//���ѷ���
    void Enemy(const card_analyse* lastone, const CardGroup& lastdiscard, CardGroup& selection, bool hint);//���з���
    
    
    void NeedSigle(const CardGroup& lastdiscard, CardGroup& selection);//�������
    void NeedDouble(const CardGroup& lastdiscard, CardGroup& selection);
    void NeedSigleSeq(const CardGroup& lastdiscard, CardGroup& selection);
    void NeedThreePlus(const CardGroup& lastdiscard, CardGroup& selection);
    void NeedAirplane(const CardGroup& lastdiscard, CardGroup& selection);

    bool Discard(void); //AI����
    bool HumanDiscard(card_analyse* lastone);//��ҳ���
    bool DiscardAndClear();//���Ʋ�������Ӧ�ṹ
                           //void Hint(void); //��ʾ��
    void Pass(void);//���ƣ�������Ӧ�ṹ
                    //����Ȩֵ���Ӽ����в�����Ӧ0-53���֣�Ȼ��Ӽ�����ɾ�������ظ����֣������ڻ���Ч����-1
    static int ValueToNum(std::set<int> &cardscopy, int value);
    void FreshenMap(std::map<int, int> &m);//ɾ��������������Ϊ���Ԫ��
    static bool MyCompare(std::shared_ptr<CardGroup> c1, std::shared_ptr<CardGroup> c2);//�Է������Ƽ�������Ļص�����
    
protected:
	card_analyse* GetEmemy();
    card_analyse* GetFriend();
    void ExchangeCards(const card_analyse* lastone, CardGroup& lastdiscard);
	void EraseCards(std::set<int>& cards, std::set<int>& erasecards);
	bool CheckNextPlayer(card_analyse* enemy, CardGroup& selection);
    void GetNextPlayerCard(card_analyse* enemy, const CardGroup& lastdiscard, int selectval, std::vector<std::shared_ptr<CardGroup>>& cards);
    void ForceDevideCards(const CardGroup& lastdiscard, CardGroup& selection, bool bigflag = true);
	bool CheckEnemyCard(card_analyse* enemy, CardGroup& selection);

protected:
    
private:
    card_analyse* preplayer_;
    card_analyse* nextplayer_;
    std::shared_ptr<card_analyse> tempplayer_;
    int id_;
    bool islandlord_;
    bool   type_;                                          //���ͣ�0�����ˣ���0��������;

    bool test_;                                            //�Ƿ��Թ����¼���;
    bool nodiscard_;                                  //������־;
    int score_;                                             //��ҵ�ǰ����;
    std::set<int> cards_;                            //����;
    std::vector<std::shared_ptr<CardGroup> > analyse_;    //�������ֵ����ͼ���;
    CardGroup selection_;                         //ѡ���Ƶļ���;
    CardGroup discard_;                            //������Ƶļ���;
};