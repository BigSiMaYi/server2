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

    void NewGame(void);//��ʼ�µ�һ�֣���һЩ��ʼ�����ϵȵĲ���
    void ClearAnalyse(void);//��շ����Ƽ���
    int    GetBaseScore(int questioned, int nowscore);//�����Ƿ��뵱������������������
    void AddCard(int num)//Ĩ��;
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

    int    GetRemain(void) { return cards_.size(); }//ʣ������
    bool IsValid(card_analyse* lastone);//�ж�ѡ�����Ƿ�ϸ�
    void AnalyseSelection(void);//����ѡ�������ͼ���Ȩֵ
    void DivideIntoGroups(void); //�������������
    void ThreeplusAndAirplane(void);//�ӷ�����Ļ����������������һ�ͷɻ�
    void DeleteUnkown(void);//ɾ�����ͼ�����δ֪����
    void SelectCards(card_analyse* lastone, bool hint = false);//AIѡ��
    CardGroup& GetSelectCards() { return selection_; }

    void Myself(CardGroup& selection);//ֱ�ӳ���
    void Friend(CardGroup& lastdiscard, CardGroup& selection);//���ѷ���
    void Enemy(CardGroup& lastdiscard, CardGroup& selection, bool hint);//���з���
    void NeedSigle(CardGroup& lastdiscard, CardGroup& selection);//�������
    void NeedDouble(CardGroup& lastdiscard, CardGroup& selection);
    void NeedSigleSeq(CardGroup& lastdiscard, CardGroup& selection);
    void NeedThreePlus(CardGroup& lastdiscard, CardGroup& selection);
    void NeedAirplane(CardGroup& lastdiscard, CardGroup& selection);

    bool Discard(void); //AI����
    bool HumanDiscard(card_analyse* lastone);//��ҳ���
    bool DiscardAndClear();//���Ʋ�������Ӧ�ṹ
                           //void Hint(void); //��ʾ��
    void Pass(void);//���ƣ�������Ӧ�ṹ
                    //����Ȩֵ���Ӽ����в�����Ӧ0-53���֣�Ȼ��Ӽ�����ɾ�������ظ����֣������ڻ���Ч����-1
    int ValueToNum(std::set<int> &cardscopy, int value);
    void FreshenMap(std::map<int, int> &m);//ɾ��������������Ϊ���Ԫ��
    static bool MyCompare(CardGroup *c1, CardGroup *c2);//�Է������Ƽ�������Ļص�����

private:
    card_analyse* preplayer_;
    card_analyse* nextplayer_;
    int id_;
    bool islandlord_;
    bool   type_;                                          //���ͣ�0�����ˣ���0��������;

    bool test_;                                            //�Ƿ��Թ����¼���;
    bool nodiscard_;                                  //������־;
    int score_;                                             //��ҵ�ǰ����;
    std::set<int> cards_;                            //����;
    std::vector<CardGroup*> analyse_;    //�������ֵ����ͼ���;
    CardGroup selection_;                         //ѡ���Ƶļ���;
    CardGroup discard_;                            //������Ƶļ���;
};