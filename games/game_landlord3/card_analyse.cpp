#include "stdafx.h"
#include "card_analyse.h"

using namespace std;

#include <algorithm>

using namespace std;

card_analyse::card_analyse(int id)
    : preplayer_(nullptr)
    , nextplayer_(nullptr)
    , test_(false)
    , nodiscard_(false)
    , score_(1000)
    , islandlord_(false)
    , id_(id)
{
}
//��ʼ�µ�һ�֣���һЩ��ʼ�����ϵȵĲ���
void card_analyse::NewGame()
{
    preplayer_ = nullptr;
    nextplayer_ = nullptr;
    islandlord_ = false;
    test_ = false;
    nodiscard_ = false;
    cards_.clear();
    ClearAnalyse();
    selection_.Clear();
    discard_.Clear();
}
//��շ����Ƽ���
void card_analyse::ClearAnalyse()
{
    if (analyse_.empty())
        return;

    for (auto mem : analyse_)
        delete mem;
    analyse_.clear();
}

//�ص�����
bool card_analyse::MyCompare(CardGroup *c1, CardGroup *c2)
{
    if (c1->type_ != c2->type_)
        return c1->type_ < c2->type_;
    else
        return c1->value_ < c2->value_;
}

int card_analyse::GetBaseScore(int questioned, int nowscore)
{
    if (questioned == 2 && nowscore == 0)//���ǰ��λ��δ���ƣ�ֱ��3�ֵ��������㶮��~
        return 3;

    int sum = 0;
    map<int, int> needanalyse;//���������Ȩֵ-��������
    for (auto mem : cards_)
        ++needanalyse[CardGroup::Translate(mem)];//�������ƹ������������

    if (needanalyse.find(16) != needanalyse.end() &&
        needanalyse.find(17) != needanalyse.end())//������ը
        sum += 8;
    else if (needanalyse.find(16) != needanalyse.end())//һ��С��
        sum += 3;
    else if (needanalyse.find(17) != needanalyse.end())//һ�Ŵ���
        sum += 4;

    if (needanalyse.find(15) != needanalyse.end())//2������
        sum += 2 * needanalyse[15];

    for (auto mem : needanalyse) {
        if (mem.second == 4)//ը��
            sum += 6;
    }
    int result;
    if (sum >= 7)
        result = 3;
    else if (sum >= 5 && sum < 7)
        result = 2;
    else if (sum >= 3 && sum < 5)
        result = 1;
    else
        result = 0;
    return (result > nowscore ? result : 0);
}
//����ѡ���Ƿ���Ϲ涨
bool card_analyse::IsValid(card_analyse* lastone)
{
    if (lastone && lastone->discard_.count_ != selection_.count_ &&
        selection_.count_ != 4 && selection_.count_ != 2)//���ƣ������������Ҳ�����Ϊը��
        return false;

    selection_.type_ = Unkown;
    AnalyseSelection();//������ѡ�Ƶ����ͼ�Ȩֵ

    if (selection_.type_ == Unkown)//��ѡ�Ʋ����Ϲ涨
        return false;

    if (lastone) {
        if (selection_.type_ == Bomb &&
            (lastone->discard_.type_ != Bomb ||
                selection_.value_ > lastone->discard_.value_))
            return true;
        if (selection_.type_ != lastone->discard_.type_ ||
            selection_.count_ != lastone->discard_.count_)//���Ͳ�������������
            return false;
        if (selection_.value_ <= lastone->discard_.value_)//ѡ�Ʋ������ϼ���
            return false;
    }
    return true;
}
//��ѡ�ƽ��з���
void card_analyse::AnalyseSelection()
{
    int NumMax = 0,//ͬ������������
        ValueMax = 0;//������������Ȩֵ

                     //�ж��Ƿ�Ϊ��ը
    if (selection_.count_ == 2 &&
        selection_.group_.find(16) != selection_.group_.end() &&
        selection_.group_.find(17) != selection_.group_.end()) {
        selection_.type_ = Bomb;
        selection_.value_ = 17;
        return;
    }
    //�ҳ���ͬ�������������������Ȩֵ
    for (auto mem : selection_.group_) {
        if (mem.second >= NumMax && mem.first > ValueMax) {
            NumMax = mem.second;
            ValueMax = mem.first;
        }
    }
    //����������ͬ����������ж�����
    switch (NumMax) {
    case 4:
        if (selection_.count_ == 4) {//ը��
            selection_.type_ = Bomb;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ == 6) {//�Ĵ�����
            selection_.type_ = FourSeq;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ == 8) {//�Ĵ�����
            for (auto mem : selection_.group_) {
                if (mem.second != 2 && mem.second != 4)//���治�Ϲ�
                    return;
            }
            selection_.type_ = FourSeq;
            selection_.value_ = ValueMax;
            return;
        }
        return;//���治�Ϲ�
    case 3:
    {
        if (selection_.count_ == 3) {//����
            selection_.type_ = Three;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ == 4) {//����һ��
            selection_.type_ = ThreePlus;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ == 5) {//��������
            for (auto mem : selection_.group_) {
                if (mem.second != 3 && mem.second != 2)
                    return;
            }
            selection_.type_ = ThreePlus;
            selection_.value_ = ValueMax;
            return;
        }
        int begin = 0, n = 0;
        for (auto mem : selection_.group_) {//�ж�������3��������������
            if (mem.second == 3) {
                if (!begin || begin == mem.first)
                    ++n;
                if (!begin)
                    begin = mem.first;
                if (begin != mem.first && n == 1) {
                    n = 1;
                    begin = mem.first;
                }
                ++begin;
            }
        }
        if (selection_.count_ == 3 * n) {//��˳
            selection_.type_ = ThreeSeq;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ == 4 * n) {//�ɻ������ŵĳ��
            selection_.type_ = Airplane;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ == 5 * n) {//�ɻ������ӳ��
            for (auto mem : selection_.group_) {
                if (mem.second != 2 && mem.second != 3)//�Ʋ��Ϲ�
                    return;
            }
            selection_.type_ = Airplane;
            selection_.value_ = ValueMax;
            return;
        }
        return;//�Ʋ��Ϲ�
    }
    case 2:
        if (selection_.count_ == 2) {//һ��
            selection_.type_ = Double;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ >= 6 && !(selection_.count_ % 2)) {//����
            int begin = 0;
            for (auto mem : selection_.group_) {//ȷ�����������ģ����Ҷ��ǳɶԵ�
                if (!begin)
                    begin = mem.first;
                if (begin++ != mem.first || mem.second != 2)//�Ʋ����Ϲ涨
                    return;
            }
            selection_.type_ = DoubleSeq;
            selection_.value_ = ValueMax;
            return;
        }
        return;//�Ʋ����Ϲ涨
    case 1:
        if (selection_.count_ == 1) {//����
            selection_.type_ = Single;
            selection_.value_ = ValueMax;
            return;
        }
        else if (selection_.count_ >= 5) {//�ж��Ƿ�Ϊ˳��
            int begin = 0;
            for (auto mem : selection_.group_) {
                if (!begin)
                    begin = mem.first;
                if (begin++ != mem.first || mem.first >= 15)//�Ʋ��������Ļ��ߴ���2�����ϵ���
                    return;
            }
            selection_.type_ = SingleSeq;//��˳
            selection_.value_ = ValueMax;
            return;
        }
    default://���䣬�����Ϲ涨
        return;
    }
}

//����Ȩֵ���Ӽ����в�����Ӧ0-53���֣�Ȼ��Ӽ�����ɾ�������ظ����֣������ڻ���Ч����-1
int card_analyse::ValueToNum(set<int>& cardscopy, int value)
{
    if (value < 3 || value > 17 || cardscopy.empty())
        throw runtime_error("Value not in set!");

    if (value == 16 && cardscopy.find(52) != cardscopy.end()) {
        cardscopy.erase(52);
        return 52;
    }
    else if (value == 17 && cardscopy.find(53) != cardscopy.end()) {
        cardscopy.erase(53);
        return 53;
    }
    else {
        for (int i = (value - 3) * 4, j = 0; j < 4; ++j) {
            if (cardscopy.find(i + j) != cardscopy.end()) {
                cardscopy.erase(i + j);
                return i + j;
            }
        }
        throw runtime_error("Value not in set!");
    }
}

//ɾ��������������Ϊ���Ԫ��
void card_analyse::FreshenMap(map<int, int>& m)
{
    for (auto itr = m.begin(); itr != m.end();)
    {
        if (itr->second == 0)
        {
            itr = m.erase(itr);
            continue;
        }
        ++itr;
    }
}

//����������Ͳ���ɻ����Ƽ���
void card_analyse::DivideIntoGroups(void)
{
    if (!analyse_.empty())//���ͼ��Ϸǿգ�����
        return;

    set<int> cardscopy(cards_);//���Ƹ���
    map<int, int> needanalyse;//���������Ȩֵ-��������

    for (auto& mem : cardscopy)
    {
        int trans_val = CardGroup::Translate(mem);
        ++needanalyse[trans_val];//�������ƹ������������
    }

    if (needanalyse.find(16) != needanalyse.end() &&
        needanalyse.find(17) != needanalyse.end()) {//��������������ը
        CardGroup *c = new CardGroup(Bomb, 17);
        for (int i = 16; i < 18; ++i) {
            c->AddNumber(ValueToNum(cardscopy, i));
            needanalyse.erase(i);
        }
        analyse_.push_back(c);
    }

    for (auto& mem : needanalyse) {
        if (mem.second == 4) {    //ը��
            CardGroup *c = new CardGroup(Bomb, mem.first);
            for (int i = 0; i < 4; ++i) {
                c->AddNumber(ValueToNum(cardscopy, mem.first));
            }
            analyse_.push_back(c);
            needanalyse[mem.first] = 0;
        }
    }
    //ɾ��������������Ϊ���Ԫ��
    FreshenMap(needanalyse);

    //��ǰ����2
    if (needanalyse.find(15) != needanalyse.end()) {
        CardGroup *c = new CardGroup(Unkown, 15);
        int n = needanalyse[15];
        switch (n) {
        case 3:
            c->type_ = Three;
            break;
        case 2:
            c->type_ = Double;
            break;
        case 1:
            c->type_ = Single;
            break;
        }
        for (int i = 0; i < n; ++i)
            c->AddNumber(ValueToNum(cardscopy, 15));
        needanalyse.erase(15);
        analyse_.push_back(c);
    }
    //���ҵ�˳
    int begin, n;
    bool exist = true;
    while (exist && !needanalyse.empty()) {
        begin = n = 0;
        for (auto itr = needanalyse.begin(); itr != needanalyse.end(); ++itr) {
            if (itr->second > 0) {//����Ϊ���Ԫ��
                if (begin == 0)
                    begin = itr->first;
                if (begin == itr->first)
                    ++n;
                ++begin;//ֵ��һ����ʾ��һ����;
            }
            if (n == 5) {//������ɵ�˳������
                auto p = itr;
                if (begin - 1 != itr->first)
                    --p;
                int first = p->first - 4;//��˳�ĵ�һ��
                CardGroup *c = new CardGroup(SingleSeq, p->first);
                for (first; first <= p->first; ++first) {
                    c->AddNumber(ValueToNum(cardscopy, first));
                    --needanalyse[first];//��һ
                }
                analyse_.push_back(c);
                exist = true;
                break;//�ӿ�ʼ���²���
            }
            //������������С����������¼����������ѵ�������������Բ�����
            auto end = needanalyse.end();
            if (begin - 1 != itr->first || itr == --end) {
                if (itr->second > 0) {
                    begin = itr->first;
                    ++begin;
                    n = 1;
                }
                else
                    begin = n = 0;
                exist = false;
            }
        }//for
    }

    //ɾ��������������Ϊ���Ԫ��
    FreshenMap(needanalyse);
    //����ܣ���������˳�����ʣ����
    for (auto mem : analyse_) {
        if (mem->type_ == SingleSeq) {//���ÿ����˳
            for (auto m : needanalyse) {
                if (m.second > 0 && m.first == mem->value_ + 1) {//ʣ�����л��бȵ�˳����һ����
                    mem->AddNumber(ValueToNum(cardscopy, m.first));
                    ++mem->value_;
                    --needanalyse[m.first];

                }
            }
        }
    }
    //ɾ��������������Ϊ���Ԫ��
    FreshenMap(needanalyse);

    //�����е�˳���п��ԶԽӳɸ����ĵ�˳����������˳Ԫ����ͬ����ϳ�˫˳
    for (auto mem1 : analyse_) {
        if (mem1->type_ == SingleSeq) {//��˳1
            for (auto mem2 : analyse_) {
                if (mem2->type_ == SingleSeq && mem1 != mem2) {//��˳2���Һ͵�˳1����ͬһ��
                    if (mem1->value_ < mem2->value_) {//mem1��ǰ
                        if (mem1->value_ == mem2->value_ - mem2->count_) {//����ƴ��
                            for (auto m : mem2->cards_)
                                mem1->AddNumber(m);
                            mem1->value_ = mem2->value_;
                            mem2->type_ = Unkown;
                        }
                    }
                    else if (mem1->value_ > mem2->value_) {//mem1�ں�
                        if (mem2->value_ == mem1->value_ - mem1->count_) {
                            for (auto m : mem1->cards_)
                                mem2->AddNumber(m);
                            mem2->value_ = mem1->value_;
                            mem1->type_ = Unkown;
                        }
                    }
                    else {//�����Ƿ���ȫһ�������Ժϲ���˫˳
                        if (mem1->count_ == mem2->count_) {
                            for (auto m : mem2->cards_)
                                mem1->AddNumber(m);
                            mem1->type_ = DoubleSeq;
                            mem2->type_ = Unkown;
                        }
                    }
                }
            }
        }
    }
    if (needanalyse.empty()) {//���������ѿգ�����
        DeleteUnkown();
        sort(analyse_.begin(), analyse_.end(), MyCompare);
        return;
    }

    //˫˳��ֻ�����������ڵ���2�������ƣ�����3����������
    begin = n = 0;
    auto last = --needanalyse.end();
    for (auto b = needanalyse.begin(); b != needanalyse.end(); ++b) {
        if (b->second >= 2) {
            if (!begin)
                begin = b->first;
            if (begin == b->first)
                ++n;
            ++begin;
        }
        if (begin && begin - 1 != b->first || b == last) {//������֮ǰ��������,���ѵ��������
            if (n >= 3) {
                auto p = b;
                if (begin - 1 != b->first)
                    --p;
                CardGroup *c = new CardGroup(DoubleSeq, p->first);
                for (int i = n; i > 0; --i, --p) {
                    for (int j = 0; j < 2; ++j) {
                        c->AddNumber(ValueToNum(cardscopy, p->first));
                        --p->second;
                    }
                }
                analyse_.push_back(c);
            }
            if (b->second >= 2) {
                n = 1;//��ǰ���������������ϵ�
                begin = b->first;
                ++begin;
            }
            else {
                n = 0;
                begin = 0;
            }
        }
    }

    //ɾ��������������Ϊ���Ԫ��
    FreshenMap(needanalyse);

    //��˳
    //�����Ƿ����غϵĵ�˳��˫˳��ϳ���˳
    for (auto& mem1 : analyse_) {
        if (mem1->type_ == SingleSeq) {
            for (auto mem2 : analyse_) {
                if (mem2->type_ == DoubleSeq) {
                    if (mem1->value_ == mem2->value_ && mem1->count_ * 2 == mem2->count_) {
                        for (auto m : mem1->cards_)
                            mem2->AddNumber(m);
                        mem2->type_ = ThreeSeq;
                        mem1->type_ = Unkown;
                    }
                }
            }
        }
    }

    if (needanalyse.empty()) {
        DeleteUnkown();
        sort(analyse_.begin(), analyse_.end(), MyCompare);
        return;
    }
    //ʣ�����в�����˳
    begin = n = 0;
    last = --needanalyse.end();
    for (auto b = needanalyse.begin(); b != needanalyse.end(); ++b) {
        if (b->second == 3) {
            if (!begin)
                begin = b->first;
            if (begin == b->first)
                ++n;
            ++begin;
        }
        if (begin && begin - 1 != b->first || b == last) {//������֮ǰ��������,���ѵ��������
            if (n >= 2) {//����2�鼰����
                auto p = b;
                if (begin - 1 != b->first)
                    --p;
                CardGroup *c = new CardGroup(ThreeSeq, p->first);
                for (int i = n; i > 0; --i, --p) {
                    for (int j = 0; j < 3; ++j) {
                        c->AddNumber(ValueToNum(cardscopy, p->first));
                        --p->second;
                    }
                }
                analyse_.push_back(c);
                if (b->second == 3) {//��ǰ������Ϊ3�ţ�
                    n = 1;
                    begin = b->first;
                    ++begin;
                }
                else {
                    n = 0;
                    begin = 0;
                }
            }
        }
    }
    //����
    for (auto& mem : needanalyse) {
        if (mem.second == 3) {
            CardGroup *c = new CardGroup(Three, mem.first);
            for (int i = 0; i < 3; ++i)
                c->AddNumber(ValueToNum(cardscopy, mem.first));
            needanalyse[mem.first] = 0;
            analyse_.push_back(c);
        }
    }

    //����
    for (auto& mem : needanalyse) {
        if (mem.second == 2) {
            CardGroup *c = new CardGroup(Double, mem.first);
            for (int i = 0; i < 2; ++i)
                c->AddNumber(ValueToNum(cardscopy, mem.first));
            needanalyse[mem.first] = 0;
            analyse_.push_back(c);
        }
    }
    //ɾ��������������Ϊ���Ԫ��
    FreshenMap(needanalyse);

    //����
    for (auto& mem : needanalyse) {
        if (mem.second != 1)
            throw runtime_error("Still has singleness card");
        CardGroup *c = new CardGroup(Single, mem.first);
        c->AddNumber(ValueToNum(cardscopy, mem.first));
        needanalyse[mem.first] = 0;
        analyse_.push_back(c);
    }
    //ɾ��������������Ϊ���Ԫ��
    FreshenMap(needanalyse);

    DeleteUnkown();
    sort(analyse_.begin(), analyse_.end(), MyCompare);
}

//����������˳���Ƴ�����һ�ͷɻ������ҵ��ƣ����Ҷ��ӣ��������ͱ���ԭ��
void card_analyse::ThreeplusAndAirplane()
{
    int n,
        doublecount = 0,//ͳ�ƶ��ӵ��������������������
        singlecount = 0;//ͳ�Ƶ�������

    for (auto mem : analyse_) {
        if (mem->type_ == Single)
            ++singlecount;
        else if (mem->type_ == Double)
            ++doublecount;
    }

    for (auto mem : analyse_) {//���Ʒɻ�
        if (mem->type_ == ThreeSeq) {
            n = mem->count_ / 3;
            if (singlecount >= n) {
                for (auto temp : analyse_) {
                    if (temp->type_ == Single) {
                        for (auto m : temp->cards_)
                            mem->AddNumber(m);
                        temp->type_ = Unkown;
                        --singlecount;
                        --n;
                    }
                    if (!n) {
                        mem->type_ = Airplane;
                        break;
                    }
                }
            }
            else if (doublecount >= n) {
                for (auto temp : analyse_) {
                    if (temp->type_ == Double) {
                        for (auto m : temp->cards_)
                            mem->AddNumber(m);
                        temp->type_ = Unkown;
                        --doublecount;
                        --n;
                    }
                    if (!n) {
                        mem->type_ = Airplane;
                        break;
                    }
                }
            }
        }
    }
    for (auto mem : analyse_) {//��������һ
        if (mem->type_ == Three) {
            if (singlecount) {
                for (auto temp : analyse_) {
                    if (temp->type_ == Single) {
                        for (auto m : temp->cards_)
                            mem->AddNumber(m);
                        temp->type_ = Unkown;
                        --singlecount;
                        mem->type_ = ThreePlus;
                        break;
                    }
                }
            }
            else if (doublecount) {
                for (auto temp : analyse_) {
                    if (temp->type_ == Double) {
                        for (auto m : temp->cards_)
                            mem->AddNumber(m);
                        temp->type_ = Unkown;
                        --doublecount;
                        mem->type_ = ThreePlus;
                        break;
                    }
                }
            }
        }
    }
}

//ɾ������δ֪���͵�����
void card_analyse::DeleteUnkown(void)
{
    auto b = analyse_.begin();
    while (b != analyse_.end()) {
        if ((*b)->type_ == Unkown) {
            delete *b;
            b = analyse_.erase(b);
        }
        else
            ++b;
    }
}

//����ѡ��
void card_analyse::SelectCards(card_analyse* lastone, bool hint)
{
    if (analyse_.empty())//�Ƿ���Ҫ���·�������
        DivideIntoGroups();
    ThreeplusAndAirplane();
    DeleteUnkown();
    sort(analyse_.begin(), analyse_.end(), MyCompare);

    if (analyse_.size() == 2) {//����Ϊ2�������ʺϵ�ը��ֱ�ӳ�
        for (auto mem : analyse_) {
            if (mem->type_ == Bomb) {
                if (lastone != nullptr &&//����Լ��ǽӱ��˵���
                    lastone->discard_.type_ == Bomb &&//����������Ϊը����
                    mem->value_ <= lastone->discard_.value_)//���Լ���ը�������ڶԷ�ʱ��
                    continue;//����ѡ�����
                selection_ = *mem;
                return;
            }
        }
    }

    if (lastone == nullptr)
    {
        Myself(selection_);//ֱ�ӳ���
    }
    else if (!hint && !IsLandlord() && !lastone->IsLandlord())
    {
        auto lastdiscard = lastone->discard_;//�з�����
        Friend(lastdiscard, selection_);//���ѷ��ƣ������Ƶ����ѷ�,���Ҳ�����ʾ
    }
    else
    {
        auto lastdiscard = lastone->discard_;//�з�����
        Enemy(lastdiscard, selection_, hint);//���з����ƻ���ʾ
    }
}

void card_analyse::Myself(CardGroup& selection)
{
    if (analyse_.size() == 1) {//ʣ���һ����
        selection = *analyse_[0];
        return;
    }

    if (analyse_.size() == 2) {//ʣ�����ƣ�����������
                               //���鿴������������ƣ�ֻΪ����ʣ�����е�����: �жϵз��ƣ�����ҵ����Ƶз�����Ҫ�����������;
        int maxNum = 0;

        for (auto& item : analyse_)
        {
            CardGroup next_selection;
            nextplayer_->Enemy(*item, next_selection, false);
            if (next_selection.count_ == 0)
            {
                selection = *item;
                return;
            }
        }

        //���򣬴��������������
        selection = *analyse_[1];
        return;
    }

    if (nextplayer_->analyse_.size() == 1)//�¼�������Ϊ1
    {
        if (nextplayer_->IsLandlord() || IsLandlord())//�¼�Ϊ���������Լ��ǵ���������Ҫ�¼�Ҫ����;
        {
            for (auto& item : analyse_)//Ӧ�ô�С�ƿ�ʼ;
            {
                CardGroup next_selection;
                nextplayer_->Enemy(*item, next_selection, false);
                if (next_selection.count_ == 0)
                {
                    selection = *item;
                    return;
                }
            }
        }
    }

    //����˳����ƣ�(A���ϵ��ƾ�����ֱ�ӳ���ը����ֱ�ӳ�)
    //���ơ����ӡ�˫˳����˳������������һ���ɻ�
    for (auto mem : analyse_) {
        if ((mem->type_ == Single || mem->type_ == Double) &&
            mem->value_ >= 15 || mem->type_ == Bomb)
            continue;
        selection = *mem;
        return;
    }
    if (analyse_.size()>0)
    {
        selection = *analyse_[0];
    }
}

void card_analyse::Friend(CardGroup& lastdiscard, CardGroup& selection)
{
    for (auto mem : analyse_) {//������Ӧ��
        if (mem->type_ == lastdiscard.type_ &&
            mem->count_ == lastdiscard.count_ &&
            mem->value_ > lastdiscard.value_) {

            selection_ = *mem;
            break;
        }
    }
    if (analyse_.size() > 2 && selection_.value_ > 14)
        selection_.Clear();//������������2��������ѡ��Ȩֵ����14��A�����򲻳���
    return;
}

void card_analyse::Enemy(CardGroup& lastdiscard, CardGroup& selection, bool hint)
{
    //��ɻ�����
    ClearAnalyse();
    DivideIntoGroups();
    sort(analyse_.begin(), analyse_.end(), MyCompare);

    for (auto mem : analyse_) {//�鿴�Ƿ�����Ӧ�ƣ�����Ȩֵ��
        if (mem->type_ == lastdiscard.type_ &&
            mem->count_ == lastdiscard.count_ &&
            mem->value_ > lastdiscard.value_) {

            selection = *mem;
            return;
        }
    }
    //��Ҫ����
    switch (lastdiscard.type_) {
    case Single://�з������ǵ���
        NeedSigle(lastdiscard, selection);
        break;
    case Double:
        NeedDouble(lastdiscard, selection);
        break;
    case SingleSeq:
        NeedSigleSeq(lastdiscard, selection);
        break;
    case Three:
        break;
    case ThreePlus://����һ
        NeedThreePlus(lastdiscard, selection);
        break;
    case Airplane://�ɻ�����Ҫ���
        NeedAirplane(lastdiscard, selection);
        break;
    default:
        break;
    }
    if (selection.count_)
        return;
    //�з�ʣһ���ƣ������ʺϵ�ը�����ͳ�ը��
    if (hint || lastdiscard.count_ > 3 || lastdiscard.value_ > 14) {
        for (auto mem : analyse_) {
            if (mem->type_ == Bomb) {
                if (lastdiscard.type_ == Bomb &&//�������������Ϊը����
                    mem->value_ <= lastdiscard.value_)//���Լ���ը�������ڶԷ�ʱ��
                    continue;//����ѡ�����
                selection = *mem;
                return;
            }
        }
    }
    return;
}

void card_analyse::NeedSigle(CardGroup& lastdiscard, CardGroup& selection)
{
    for (auto mem : analyse_) {
        if (mem->type_ == SingleSeq && mem->count_ > 5) {//����,��˳��������5��
            if (mem->group_.begin()->first > lastdiscard.value_) {
                selection.AddNumber(*mem->cards_.begin());
                selection.value_ = mem->group_.begin()->first;
                selection.type_ = Single;
                ClearAnalyse();//�����ˣ�һ��Ҫ���
                return;
            }
            else if (mem->group_.rbegin()->first > lastdiscard.value_) {
                selection.AddNumber(*(mem->cards_.rbegin()));
                selection.value_ = mem->value_;
                selection.type_ = Single;
                ClearAnalyse();//�����ˣ�һ��Ҫ���
                return;
            }
        }
    }
    for (auto mem : analyse_) {
        if (mem->type_ == Three) {//���,������
            if (mem->group_.begin()->first > lastdiscard.value_) {
                selection.AddNumber(*mem->cards_.begin());
                selection.value_ = mem->group_.begin()->first;
                selection.type_ = Single;
                ClearAnalyse();//�����ˣ�һ��Ҫ���
                return;
            }
        }
    }
    for (auto mem : analyse_) {
        if (mem->type_ == Double) {//����,�����
            if (mem->group_.begin()->first > lastdiscard.value_) {
                selection.AddNumber(*mem->cards_.begin());
                selection.value_ = mem->group_.begin()->first;
                selection.type_ = Single;
                ClearAnalyse();//�����ˣ�һ��Ҫ���
                return;
            }
        }
    }
}

void card_analyse::NeedDouble(CardGroup& lastdiscard, CardGroup& selection)
{
    for (auto mem : analyse_) {
        if (mem->type_ == Three) {//������
            if (mem->group_.begin()->first > lastdiscard.value_) {
                auto b = mem->cards_.begin();
                for (int i = 0; i < 2; ++i)
                    selection.AddNumber(*b++);
                selection.value_ = mem->group_.begin()->first;
                selection.type_ = Double;
                ClearAnalyse();//�����ˣ�һ��Ҫ���
                return;
            }
        }
    }
    for (auto mem : analyse_) {
        int i = 0, m = 0;
        if (mem->type_ == ThreeSeq) {//����˳
            if (mem->group_.begin()->first > lastdiscard.value_) {
                auto b = mem->cards_.begin();
                for (int i = 0; i < 2; ++i)
                    selection.AddNumber(*b++);
                selection.value_ = mem->group_.begin()->first;
                selection.type_ = Double;
                ClearAnalyse();//�����ˣ�һ��Ҫ���
                return;
            }
            else if (mem->group_.rbegin()->first > lastdiscard.value_) {
                selection.AddNumber(*(mem->cards_.rbegin()));
                selection.value_ = mem->value_;
                selection.type_ = Double;
                ClearAnalyse();//�����ˣ�һ��Ҫ���
                return;
            }
        }
    }
}

void card_analyse::NeedSigleSeq(CardGroup& lastdiscard, CardGroup& selection)
{
    for (auto mem : analyse_) {
        if (mem->type_ == SingleSeq &&
            mem->value_ > lastdiscard.value_ &&
            mem->count_ > lastdiscard.count_) {//������ĵ�˳
            if (mem->count_ - (mem->value_ - lastdiscard.value_) >= lastdiscard.count_) {
                //����˳�ǴӶ̵�˳�Ŀ�ʼ��Ԫ�ػ��С��Ԫ�ؿ�ʼ��
                for (int i = lastdiscard.value_ - lastdiscard.count_ + 2, j = 0;
                    j < lastdiscard.count_; ++j)
                    selection.AddNumber(ValueToNum(mem->cards_, i + j));
                selection.value_ = lastdiscard.value_ + 1;
                selection.type_ = SingleSeq;
                ClearAnalyse();//�����ˣ�һ��Ҫ���
                return;
            }
            else {//����˳�Ŀ�ʼԪ�رȶ̵�˳�Ŀ�ʼԪ�ش�
                int i = 0;
                auto b = mem->cards_.begin();
                for (; i < lastdiscard.count_; ++i, ++b)
                    selection.AddNumber(*b);
                selection.value_ = CardGroup::Translate(*--b);
                selection.type_ = SingleSeq;
                ClearAnalyse();//�����ˣ�һ��Ҫ���
                return;
            }
        }
    }
}

void card_analyse::NeedThreePlus(CardGroup& lastdiscard, CardGroup& selection)
{
    auto b = analyse_.begin();
    for (; b != analyse_.end(); ++b) {//���ұȶԷ�������ͬ�Ƶ�����������
        if ((*b)->type_ == Three && (*b)->value_ > lastdiscard.value_) {
            break;
        }
    }
    if (b == analyse_.end())//���û��
        return;//����
    if (lastdiscard.count_ == 4) {//������Ϊ����һ��
        if (analyse_[0]->type_ == Single) {//�е���
            for (auto m : analyse_[0]->cards_)
                (*b)->AddNumber(m);
            (*b)->type_ = ThreePlus;
            analyse_[0]->type_ = Unkown;
            selection = **b;
            return;
        }
        else {//��Ҫ����
            for (auto mem : analyse_) {
                if (mem->type_ == SingleSeq && mem->count_ > 5) {//����,��˳��������5��
                    selection = **b;
                    selection.AddNumber(*mem->cards_.begin());
                    selection.type_ = ThreePlus;
                    ClearAnalyse();//�����ˣ�һ��Ҫ���
                    return;
                }
            }
            for (auto mem : analyse_) {
                if (mem->type_ == Three && mem != *b && mem->value_ < 14) {//���,������
                    selection = **b;
                    selection.AddNumber(*mem->cards_.begin());
                    selection.type_ = ThreePlus;
                    ClearAnalyse();//�����ˣ�һ��Ҫ���
                    return;
                }
            }
            for (auto mem : analyse_) {
                if (mem->type_ == Double && mem->value_ < 14) {//����,�����
                    selection = **b;
                    selection.AddNumber(*mem->cards_.begin());
                    selection.type_ = ThreePlus;
                    ClearAnalyse();//�����ˣ�һ��Ҫ���
                    return;
                }
            }
        }
    }
    else {//����һ��
        for (auto mem : analyse_) {//���Ҷ���
            if (mem->type_ == Double && mem->value_ < 14) {
                for (auto m : mem->cards_)
                    (*b)->AddNumber(m);
                (*b)->type_ = ThreePlus;
                mem->type_ = Unkown;
                selection = **b;
                return;
            }
        }
        for (auto mem : analyse_) {
            if (mem->type_ == Three && mem != *b && mem->value_ < 14) {//���,������
                selection = **b;
                for (int i = 0; i < 3; ++i)
                    selection.AddNumber(*mem->cards_.begin());
                selection.type_ = ThreePlus;
                ClearAnalyse();//�����ˣ�һ��Ҫ���
                return;
            }
        }
    }
}

void card_analyse::NeedAirplane(CardGroup& lastdiscard, CardGroup& selection)
{
    ClearAnalyse();
    DivideIntoGroups();
    sort(analyse_.begin(), analyse_.end(), MyCompare);

    int wing = 0,//�������
        n = 0;//��˳�������Ƶĸ���
    for (auto mem : lastdiscard.group_) {
        if (mem.second == 3)
            ++n;
    }
    if (lastdiscard.count_ == 5 * n)//�ɻ����Ϊ����
        wing = 2;
    else {//�ɻ����Ϊ����
        while (lastdiscard.count_ != 4 * n)
            --n;
        wing = 1;
    }
    auto b = analyse_.begin();
    for (; b != analyse_.end(); ++b) {
        if ((*b)->type_ == ThreeSeq &&
            (*b)->count_ == 3 * n &&
            (*b)->value_ > lastdiscard.value_)
            break;
    }
    if (b == analyse_.end())
        return;
    int count = 0;
    for (auto mem : analyse_) {
        if (mem->type_ == (wing == 1 ? Single : Double))
            ++count;
    }
    if (count < n)
        return;
    for (auto mem : analyse_) {
        if (mem->type_ == (wing == 1 ? Single : Double)) {
            for (auto m : mem->cards_)
                (*b)->AddNumber(m);
            mem->type_ = Unkown;
            --n;
        }
        if (!n)
            break;
    }
    (*b)->type_ = Airplane;
    selection = **b;
}
//���Ʋ����÷�������
bool card_analyse::DiscardAndClear()
{
    discard_ = selection_;//��ѡ�Ʒ�������������ѡ��
    bool needclear = true;//���γ����Ƿ�Ϊ���ƣ���Ҫ���·����ƶ�
    for (auto b = analyse_.begin(); b != analyse_.end(); ++b) {
        if ((*b)->type_ == selection_.type_ &&
            (*b)->value_ == selection_.value_ &&
            (*b)->count_ == selection_.count_) {//���ǲ���
            delete (*b);
            analyse_.erase(b);
            needclear = false;//����Ҫ���
            break;
        }
    }
    if (needclear)//��Ҫ��գ��´γ���Ҫ���·���
        ClearAnalyse();

    for (auto mem : selection_.cards_) {
        cards_.erase(mem);//��������ɾ�������
    }
    selection_.Clear();//���ѡ����
    return true;
}
//���Գ���
bool card_analyse::Discard(void)
{
    if (selection_.count_ == 0)//����ѡ����Ϊ�գ�˵������;
    {
        nodiscard_ = true;
        return false;
    }
    //�����������
    return DiscardAndClear();
}
//��ҳ���
bool card_analyse::HumanDiscard(card_analyse* lastone)
{ 
    if (selection_.count_ == 0 
        || !IsValid(lastone)) //ѡ�Ʋ����Ϲ涨;
    {
        selection_.Clear();//���ѡ��
        return false;//�������
    }
    //��������������������Ƿ����
    return DiscardAndClear();
}
//����
void card_analyse::Pass(void)
{
    nodiscard_ = true;
    selection_.Clear();
}