#include "stdafx.h"
#include "card_analyse.h"

using namespace std;

#include <algorithm>

using namespace std;

card_analyse::card_analyse(int id)
    : preplayer_(nullptr)
    , nextplayer_(nullptr)
    , tempplayer_(nullptr)
    , test_(false)
    , nodiscard_(false)
    , score_(1000)
    , islandlord_(false)
    , id_(id)
{
}

card_analyse::~card_analyse(void)
{
	ClearAnalyse();
}

//��ʼ�µ�һ�֣���һЩ��ʼ�����ϵȵĲ���
void card_analyse::NewGame()
{
    tempplayer_ = std::make_shared<card_analyse>(4);
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

void card_analyse::ClearAnalyse()
{
    analyse_.clear();
}
//�ص�����
bool card_analyse::MyCompare(std::shared_ptr<CardGroup> c1, std::shared_ptr<CardGroup> c2)
{
    if (c1->type_ != c2->type_)
        return c1->type_ < c2->type_;
    else
        return c1->value_ < c2->value_;
}

int card_analyse::GetBaseScore(int questioned, int nowscore)
{
    int sum = CalcCardsWight(cards_);
    
    int result = 0;
    if (sum >= 12)
    {
        result = 3;
    }
    else if (sum >= 7 && sum < 12)
    {
        result = 2;
    }
    else if (sum >= 4 && sum < 7)
    {
        result = 1;
    }

    if (questioned == 2 && nowscore == 0)//���ǰ��λ��δ���ƣ�ֱ��3�ֵ�����;
    {
        if (result == 0)
        {
            result = 1;
        }
    }
    return (result > nowscore ? result : 0);
}

int card_analyse::CalcCardsWight(std::set<int>& cards)
{
    map<int, int> needanalyse;//���������Ȩֵ-��������;
    for (auto& mem : cards)
    {
        ++needanalyse[CardGroup::Translate(mem)];//�������ƹ������������;
    }

    int sum = 0;
    if (needanalyse.find(16) != needanalyse.end()
        && needanalyse.find(17) != needanalyse.end())//������ը;
    {
        sum += 8;
    }
    else if (needanalyse.find(17) != needanalyse.end())//һ�Ŵ���;
    {
        sum += 4;
    }
    else if (needanalyse.find(16) != needanalyse.end())//һ��С��;
    {
        sum += 3;
    }
    if (needanalyse.find(15) != needanalyse.end())//2������;
    {
        if (needanalyse[15] < 4)
        {
            sum += 2 * needanalyse[15];
        }
    }

    for (auto& mem : needanalyse)
    {
        if (mem.second == 4)//ը��;
        {
            if (mem.first > 10)
            {
                sum += 7;
            }
            else
            {
                sum += 6;
            }
        }
    }
    return sum;
}

//����ѡ���Ƿ���Ϲ涨
bool card_analyse::IsValid(card_analyse* lastone)
{
    if (lastone)
    { 
        if (lastone->discard_.count_ != selection_.count_
            && selection_.count_ != 4 
            && selection_.count_ != 2)//���ƣ������������Ҳ�����Ϊը��;
        {
            return false;
        }
    }

    selection_.type_ = Unkown;
    AnalyseSelection();//������ѡ�Ƶ����ͼ�Ȩֵ;

    if (selection_.type_ == Unkown)//��ѡ�Ʋ����Ϲ涨;
        return false;

    if (lastone)
    {
        if (selection_.type_ == Bomb
            && (lastone->discard_.type_ != Bomb || selection_.value_ > lastone->discard_.value_))
        {
            return true;
        }
        if (selection_.type_ != lastone->discard_.type_
            || selection_.count_ != lastone->discard_.count_)//���Ͳ�������������;
        {
            return false;
        }
        if (selection_.value_ <= lastone->discard_.value_)//ѡ�Ʋ������ϼ���;
        {
            return false;
        }
    }
    return true;
}
//��ѡ�ƽ��з���
void card_analyse::AnalyseSelection()
{
    int NumMax = 0,//ͬ������������
        ValueMax = 0;//������������Ȩֵ

    //�ж��Ƿ�Ϊ��ը
    if (selection_.count_ == 2 
        && selection_.group_.find(16) != selection_.group_.end() 
        && selection_.group_.find(17) != selection_.group_.end())
    {
        selection_.type_ = Bomb;
        selection_.value_ = 17;
        return;
    }
    //�ҳ���ͬ�������������������Ȩֵ
    for (auto mem : selection_.group_)
    {
        if (mem.second >= NumMax && mem.first > ValueMax)
        {
            NumMax = mem.second;
            ValueMax = mem.first;
        }
    }
    //����������ͬ����������ж�����
    switch (NumMax)
    {
    case 4:
        if (selection_.count_ == 4)//ը��;
        {
            selection_.type_ = Bomb;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ == 6)//�Ĵ�����;
        {
            selection_.type_ = FourSeq;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ == 8)//�Ĵ�����;
        {
            for (auto mem : selection_.group_)
            {
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
        if (selection_.count_ == 3)//����;
        {
            selection_.type_ = Three;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ == 4)//����һ��;
        {
            selection_.type_ = ThreePlus;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ == 5)//��������;
        {
            for (auto mem : selection_.group_)
            {
                if (mem.second != 3 && mem.second != 2)
                    return;
            }
            selection_.type_ = ThreePlus;
            selection_.value_ = ValueMax;
            return;
        }
        int begin = 0, n = 0;
        for (auto mem : selection_.group_)//�ж�������3��������������;
        {
            if (mem.second == 3)
            {
                if (!begin || begin == mem.first)
                    ++n;
                if (!begin)
                    begin = mem.first;
                if (begin != mem.first && n == 1)
                {
                    n = 1;
                    begin = mem.first;
                }
                ++begin;
            }
        }
        if (selection_.count_ == 3 * n)//��˳;
        {
            selection_.type_ = ThreeSeq;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ == 4 * n)//�ɻ������ŵĳ��;
        {
            selection_.type_ = Airplane;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ == 5 * n)//�ɻ������ӳ��;
        {
            for (auto mem : selection_.group_)
            {
                if (mem.second != 2 && mem.second != 3)//�Ʋ��Ϲ�;
                    return;
            }
            selection_.type_ = Airplane;
            selection_.value_ = ValueMax;
            return;
        }
        return;//�Ʋ��Ϲ�
    }
    case 2:
        if (selection_.count_ == 2)//һ��;
        {
            selection_.type_ = Double;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ >= 6 && !(selection_.count_ % 2))//����;
        {
            int begin = 0;
            for (auto mem : selection_.group_)//ȷ�����������ģ����Ҷ��ǳɶԵ�;
            {
                if (!begin)
                    begin = mem.first;
                if (begin++ != mem.first || mem.second != 2)//�Ʋ����Ϲ涨;
                    return;
            }
            selection_.type_ = DoubleSeq;
            selection_.value_ = ValueMax;
            return;
        }
        return;//�Ʋ����Ϲ涨;
    case 1:
        if (selection_.count_ == 1)//����;
        {
            selection_.type_ = Single;
            selection_.value_ = ValueMax;
            return;
        }
        else if (selection_.count_ >= 5)//�ж��Ƿ�Ϊ˳��;
        {
            int begin = 0;
            for (auto mem : selection_.group_)
            {
                if (!begin)
                    begin = mem.first;
                if (begin++ != mem.first || mem.first >= 15)//�Ʋ��������Ļ��ߴ���2�����ϵ���;
                    return;
            }
            selection_.type_ = SingleSeq;//��˳;
            selection_.value_ = ValueMax;
            return;
        }
    default://���䣬�����Ϲ涨;
        return;
    }
}

//����Ȩֵ���Ӽ����в�����Ӧ0-53���֣�Ȼ��Ӽ�����ɾ�������ظ����֣������ڻ���Ч����-1
int card_analyse::ValueToNum(set<int>& cardscopy, int value)
{
    if (value < 3 || value > 17 || cardscopy.empty())
        throw runtime_error("Value not in set!");

    if (value == 16 && cardscopy.find(52) != cardscopy.end())
    {
        cardscopy.erase(52);
        return 52;
    }
    else if (value == 17 && cardscopy.find(53) != cardscopy.end())
    {
        cardscopy.erase(53);
        return 53;
    }
    else
    {
        for (int i = (value - 3) * 4, j = 0; j < 4; ++j)
        {
            if (cardscopy.find(i + j) != cardscopy.end())
            {
                cardscopy.erase(i + j);
                return i + j;
            }
        }
        throw runtime_error("Value not in set!");
    }
}

//ɾ��������������Ϊ���Ԫ��;
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

//����������Ͳ���ɻ����Ƽ���;
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

    if (needanalyse.find(16) != needanalyse.end() 
        && needanalyse.find(17) != needanalyse.end())//��������������ը;
    {
        auto c = std::make_shared<CardGroup>(Bomb, 17);
        for (int i = 16; i < 18; ++i)
        {
            c->AddNumber(ValueToNum(cardscopy, i));
            needanalyse.erase(i);
        }
        analyse_.push_back(c);
    }

    for (auto& mem : needanalyse)
    {
        if (mem.second == 4)//ը��;
        {    
            auto c = std::make_shared<CardGroup>(Bomb, mem.first);
            for (int i = 0; i < 4; ++i)
            {
                c->AddNumber(ValueToNum(cardscopy, mem.first));
            }
            analyse_.push_back(c);
            needanalyse[mem.first] = 0;
        }
    }
    //ɾ��������������Ϊ���Ԫ��
    FreshenMap(needanalyse);

    //��ǰ����2
    if (needanalyse.find(15) != needanalyse.end())
    {
        auto c = std::make_shared<CardGroup>(Unkown, 15);
        int n = needanalyse[15];
        switch (n)
        {
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
    while (exist && !needanalyse.empty())
    {
        begin = n = 0;
        for (auto itr = needanalyse.begin(); itr != needanalyse.end(); ++itr)
        {
            if (itr->second > 0)//����Ϊ���Ԫ��;
            {
                if (begin == 0)
                    begin = itr->first;
                if (begin == itr->first)
                    ++n;
                ++begin;//ֵ��һ����ʾ��һ����;
            }
            if (n == 5)//������ɵ�˳������;
            {
                auto p = itr;
                if (begin - 1 != itr->first)
                    --p;
                int first = p->first - 4;//��˳�ĵ�һ��
                auto c = std::make_shared<CardGroup>(SingleSeq, p->first);
                for (first; first <= p->first; ++first)
                {
                    c->AddNumber(ValueToNum(cardscopy, first));
                    --needanalyse[first];//��һ
                }
                analyse_.push_back(c);
                exist = true;
                break;//�ӿ�ʼ���²���
            }
            //������������С����������¼����������ѵ�������������Բ�����
            auto end = needanalyse.end();
            if (begin - 1 != itr->first || itr == --end)
            {
                if (itr->second > 0)
                {
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

    //ɾ��������������Ϊ���Ԫ��;
    FreshenMap(needanalyse);
    //����ܣ���������˳�����ʣ����;
    for (auto mem : analyse_)
    {
        if (mem->type_ == SingleSeq)//���ÿ����˳;
        {
            for (auto m : needanalyse)
            {
                if (m.second > 0 && m.first == mem->value_ + 1)//ʣ�����л��бȵ�˳����һ����;
                {
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
    for (auto mem1 : analyse_)
    {
        if (mem1->type_ == SingleSeq)//��˳1;
        {
            for (auto mem2 : analyse_)
            {
                if (mem2->type_ == SingleSeq && mem1 != mem2)//��˳2���Һ͵�˳1����ͬһ��
                {
                    if (mem1->value_ < mem2->value_)//mem1��ǰ;
                    {
                        if (mem1->value_ == mem2->value_ - mem2->count_)//����ƴ��
                        {
                            for (auto m : mem2->cards_)
                                mem1->AddNumber(m);
                            mem1->value_ = mem2->value_;
                            mem2->type_ = Unkown;
                        }
                    }
                    else if (mem1->value_ > mem2->value_)//mem1�ں�;
                    {
                        if (mem2->value_ == mem1->value_ - mem1->count_)
                        {
                            for (auto m : mem1->cards_)
                                mem2->AddNumber(m);
                            mem2->value_ = mem1->value_;
                            mem1->type_ = Unkown;
                        }
                    }
                    else//�����Ƿ���ȫһ�������Ժϲ���˫˳;
                    {
                        if (mem1->count_ == mem2->count_)
                        {
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
    if (needanalyse.empty())//���������ѿգ�����;
    {
        DeleteUnkown();
        sort(analyse_.begin(), analyse_.end(), MyCompare);
        return;
    }

    //˫˳��ֻ�����������ڵ���2�������ƣ�����3����������
    begin = n = 0;
    auto last = --needanalyse.end();
    for (auto b = needanalyse.begin(); b != needanalyse.end(); ++b)
    {
        if (b->second >= 2)
        {
            if (!begin)
                begin = b->first;
            if (begin == b->first)
                ++n;
            ++begin;
        }
        if (begin && begin - 1 != b->first || b == last)//������֮ǰ��������,���ѵ��������;
        {
            if (n >= 3)
            {
                auto p = b;
                if (begin - 1 != b->first)
                    --p;
                auto c = std::make_shared<CardGroup>(DoubleSeq, p->first);
                for (int i = n; i > 0; --i, --p)
                {
                    for (int j = 0; j < 2; ++j)
                    {
                        c->AddNumber(ValueToNum(cardscopy, p->first));
                        --p->second;
                    }
                }
                analyse_.push_back(c);
            }
            if (b->second >= 2)
            {
                n = 1;//��ǰ���������������ϵ�;
                begin = b->first;
                ++begin;
            }
            else
            {
                n = 0;
                begin = 0;
            }
        }
    }

    //ɾ��������������Ϊ���Ԫ��
    FreshenMap(needanalyse);

    //��˳
    //�����Ƿ����غϵĵ�˳��˫˳��ϳ���˳
    for (auto& mem1 : analyse_)
    {
        if (mem1->type_ == SingleSeq)
        {
            for (auto mem2 : analyse_)
            {
                if (mem2->type_ == DoubleSeq)
                {
                    if (mem1->value_ == mem2->value_ && mem1->count_ * 2 == mem2->count_)
                    {
                        for (auto m : mem1->cards_)
                            mem2->AddNumber(m);
                        mem2->type_ = ThreeSeq;
                        mem1->type_ = Unkown;
                    }
                }
            }
        }
    }

    if (needanalyse.empty())
    {
        DeleteUnkown();
        sort(analyse_.begin(), analyse_.end(), MyCompare);
        return;
    }
    //ʣ�����в�����˳
    begin = n = 0;
    last = --needanalyse.end();
    for (auto b = needanalyse.begin(); b != needanalyse.end(); ++b)
    {
        if (b->second == 3)
        {
            if (!begin)
                begin = b->first;
            if (begin == b->first)
                ++n;
            ++begin;
        }
        if (begin && begin - 1 != b->first || b == last)//������֮ǰ��������,���ѵ��������;
        {
            if (n >= 2)//����2�鼰����;
            {
                auto p = b;
                if (begin - 1 != b->first)
                    --p;
                auto c = std::make_shared<CardGroup>(ThreeSeq, p->first);
                for (int i = n; i > 0; --i, --p)
                {
                    for (int j = 0; j < 3; ++j)
                    {
                        c->AddNumber(ValueToNum(cardscopy, p->first));
                        --p->second;
                    }
                }
                analyse_.push_back(c);
                if (b->second == 3)//��ǰ������Ϊ3�ţ�;
                {
                    n = 1;
                    begin = b->first;
                    ++begin;
                }
                else
                {
                    n = 0;
                    begin = 0;
                }
            }
        }
    }
    //����
    for (auto& mem : needanalyse)
    {
        if (mem.second == 3)
        {
            auto c = std::make_shared<CardGroup>(Three, mem.first);
            for (int i = 0; i < 3; ++i)
                c->AddNumber(ValueToNum(cardscopy, mem.first));
            needanalyse[mem.first] = 0;
            analyse_.push_back(c);
        }
    }

    //����
    for (auto& mem : needanalyse)
    {
        if (mem.second == 2)
        {
            auto c = std::make_shared<CardGroup>(Double, mem.first);
            for (int i = 0; i < 2; ++i)
                c->AddNumber(ValueToNum(cardscopy, mem.first));
            needanalyse[mem.first] = 0;
            analyse_.push_back(c);
        }
    }
    //ɾ��������������Ϊ���Ԫ��
    FreshenMap(needanalyse);

    //����
    for (auto& mem : needanalyse)
    {
        if (mem.second != 1)
            throw runtime_error("Still has singleness card");
        auto c = std::make_shared<CardGroup>(Single, mem.first);
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

    for (auto mem : analyse_)
    {
        if (mem->type_ == Single)
            ++singlecount;
        else if (mem->type_ == Double)
            ++doublecount;
    }

    for (auto mem : analyse_)//���Ʒɻ�;
    {
        if (mem->type_ == ThreeSeq)
        {
            n = mem->count_ / 3;
            if (singlecount >= n)
            {
                for (auto temp : analyse_)
                {
                    if (temp->type_ == Single)
                    {
                        for (auto m : temp->cards_)
                            mem->AddNumber(m);
                        temp->type_ = Unkown;
                        --singlecount;
                        --n;
                    }
                    if (!n)
                    {
                        mem->type_ = Airplane;
                        break;
                    }
                }
            }
            else if (doublecount >= n)
            {
                for (auto temp : analyse_)
                {
                    if (temp->type_ == Double)
                    {
                        for (auto m : temp->cards_)
                            mem->AddNumber(m);
                        temp->type_ = Unkown;
                        --doublecount;
                        --n;
                    }
                    if (!n)
                    {
                        mem->type_ = Airplane;
                        break;
                    }
                }
            }
        }
    }
    for (auto mem : analyse_)//��������һ;
    {
        if (mem->type_ == Three)
        {
            if (singlecount)
            {
                for (auto temp : analyse_)
                {
                    if (temp->type_ == Single)
                    {
                        for (auto m : temp->cards_)
                            mem->AddNumber(m);
                        temp->type_ = Unkown;
                        --singlecount;
                        mem->type_ = ThreePlus;
                        break;
                    }
                }
            }
            else if (doublecount)
            {
                for (auto temp : analyse_)
                {
                    if (temp->type_ == Double)
                    {
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
    while (b != analyse_.end())
	{
        if ((*b)->type_ == Unkown)
		{
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

    if (analyse_.size() == 2)//����Ϊ2�������ʺϵ�ը��ֱ�ӳ�;
    {
        for (auto& mem : analyse_)
        {
            if (mem->type_ == Bomb)
            {
                if (lastone != nullptr //����Լ��ǽӱ��˵���;
                    && lastone->discard_.type_ == Bomb  //����������Ϊը��;
                    && mem->value_ <= lastone->discard_.value_)//���Լ���ը�������ڶԷ�ʱ;
                    continue;//����ѡ�����;
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
        Friend(lastone, lastdiscard, selection_);//���ѷ��ƣ������Ƶ����ѷ�,���Ҳ�����ʾ
    }
    else
    {
        auto& lastdiscard = lastone->discard_;//�з�����
        Enemy(lastone, lastdiscard, selection_, hint);//���з����ƻ���ʾ
    }
}

void card_analyse::AddDiscards(std::set<int>& nums)
{
    selection_.Clear();
    for (auto& num : nums)
    {
        if (selection_.cards_.find(num) == selection_.cards_.end())
        {
            selection_.AddNumber(num);
        }
    }
}

void card_analyse::Myself(CardGroup& selection)
{
    if (analyse_.size() == 1)//ʣ���һ����;
    {
        selection = *analyse_[0];
        return;
    }

    if (analyse_.size() == 2)//ʣ�����ƣ�����������;
    {
        //���鿴������������ƣ�ֻΪ����ʣ�����е�����: �жϵз��ƣ�����ҵ����Ƶз�����Ҫ�����������;
        card_analyse* enemy = GetEmemy();
		for (auto& item : analyse_)
        {
            if (item->type_ != Bomb)
            {
                CardGroup next_selection;
                enemy->Enemy(this, *item, next_selection, false);
                if (next_selection.count_ == 0)
                {
                    selection = *item;
                    return;
                }
            }
        }
        selection = *analyse_[0]; //����������Լ��������ƶԷ�������Ҫ�����С��;
        return;
    }

	if (CheckNextPlayer(nextplayer_, selection))
	{
		if (selection.count_ > 0)
		{
			return;
		}
	}
	//����ϼ�����;
	if (CheckEnemyCard(preplayer_, selection))
	{
		if (selection.count_ > 0)
		{
			return;
		}
	}
    //����˳����ƣ�(A���ϵ��ƾ�����ֱ�ӳ���ը����ֱ�ӳ�);
    //���ơ����ӡ�˫˳����˳������������һ���ɻ�;
    for (auto mem : analyse_)
    {
        if ((mem->type_ == Single || mem->type_ == Double)
            && mem->value_ >= 15 || mem->type_ == Bomb)
            continue;
        selection = *mem;
        return;
    }
    if (analyse_.size() > 0)
    {
        selection = *analyse_[0];
    }
}

void card_analyse::Friend(card_analyse* lastone, const CardGroup& lastdiscard, CardGroup& selection)
{
	if (analyse_.size() == 1)//ʣ���һ����;
	{
		auto mem = analyse_[0];
		if (mem->type_ == lastdiscard.type_
			&& mem->count_ == lastdiscard.count_
			&& mem->value_ > lastdiscard.value_)
		{
			selection = *mem;
			return;
		}
		if (mem->type_ == Bomb)
		{
			if (lastdiscard.type_ != Bomb)
			{
				selection = *mem;
				return;
			}
		}
	}

    if (lastdiscard.type_ == Bomb/* || lastdiscard.value_ > 13*/)
    {
        selection_.Clear();
        return;
    }

    if (preplayer_->IsLandlord())//���ϼҡ��ǵ�����Ҫ�ѷ�����;
    {
        if (lastone->analyse_.size() == 1) //�����Ƶ����ѷ�;
        {
            //����Լ�������ը�������ң������ƿ����Ͷ���ֱ���ߣ�����Գ�ը����Ȼ���С�����ѷ���;
            bool is_small = false;
            for (auto& mem : analyse_)
            {
                if (mem->type_ == analyse_[0]->type_
                    && mem->count_ == analyse_[0]->count_
                    && mem->value_ < analyse_[0]->value_)
                {
                    is_small = true;
                    break;
                }
                if (mem->type_ == Bomb)
                {
                    selection_ = *mem;
                }
            }
            if (!is_small)
            {
                selection_.Clear();
            }
            return;
        }
        if (lastone->analyse_.size() == 2)
        {
            //�ж��ѷ��ǲ��Ǹ��Լ����ƣ��������ѷ����ϵ��Ƶ������Գ�;
            auto enemy = GetEmemy();
            if (enemy)
            {
                bool is_small = false;
                for (auto& item : lastone->analyse_)
                {
                    CardGroup check_selection;
                    enemy->Enemy(lastone, *item, check_selection, false);
                    if (check_selection.count_ > 0)
                    {
                        is_small = true;
                    }
                }
                if (!is_small)
                {
                    return;
                }
            }
        }
    }
    if (nextplayer_->IsLandlord()) //�¼��ǵ���;
    {
		//�����Ƶ����ѷ�����ʱ�ж��������¼��Ƿ���Ҫ���������Ҫ���򱾷�Ҳ��Ҫ;
        if (lastone->analyse_.size() == 1) //�����Ƶ����ѷ�����ֻʣһ���ƣ���ʱ�ж��������¼��Ƿ���Ҫ���������Ҫ���򱾷�Ҳ��Ҫ;
        {
            CardGroup temp;
            nextplayer_->Enemy(lastone, lastdiscard, temp, false);
            if (temp.count_ == 0)
            {
                return;
            }
        }
		nextplayer_->ClearAnalyse();
        nextplayer_->DivideIntoGroups();

        if (nextplayer_->analyse_.size() == 1)
        {
            auto& enemy_card = *nextplayer_->analyse_[0];
            if (lastdiscard.type_ == enemy_card.type_ && lastdiscard.value_ < enemy_card.value_)
            {
                Enemy(nextplayer_, enemy_card, selection_, false);
                if (selection_.count_ > 0)
                {
                    return;
                }
            }
            else
            {
                Enemy(lastone, lastdiscard, selection_, false);
                if (selection_.count_ > 0)
                {
                    return;
                }
            }
        }
    }

    for (auto& mem : analyse_)//������Ӧ��;
    {
        if (mem->type_ == lastdiscard.type_
            && mem->count_ == lastdiscard.count_
            && mem->value_ > lastdiscard.value_)
        {
			if (lastdiscard.type_ == Single)
            {
                selection = *mem;
                if (mem->value_ > 9) //����С�ĵ��ſ�ʼ���ϲ��ҵ��ţ��������10�ͳ�;
                {
                    break;
                }
            }
			else if (lastdiscard.type_ == Double)
			{
                selection = *mem;
                if (mem->value_ > 5)
                {
                    break;
                }
            }
            else
            {
                selection = *mem;
                break;
            }
        }
    }

	//if (selection.count_ == 0)//��ֹ�з���С��;
	//{
	//	if (lastdiscard.value_ < 11)//�ѷ���С��11�ĵ��Ż��߶���;
	//	{
	//		std::vector<CardGroup*> cards0;
	//		GetNextPlayerCard(nextplayer_, lastdiscard, 11, cards0);
    //
	//		if (lastdiscard.type_ == Single && cards0.size() > 0)
	//		{
	//			this->NeedSigle(lastdiscard, selection);
	//		}
	//		if (lastdiscard.type_ == Double && cards0.size() > 0)
	//		{
	//			this->NeedDouble(lastdiscard, selection);
	//		}
	//	}
	//}
	
	//�ѷ����Ƴ���10,���ҵз�û�к��ʵ�С�ƣ���ѹ;
	if (lastdiscard.value_ > 10)//�ѷ����ӳ���10���з�����
	{
		std::vector<std::shared_ptr<CardGroup>> cards0;
		GetNextPlayerCard(nextplayer_, lastdiscard, 14, cards0);
		if ((lastdiscard.type_ == Double || lastdiscard.type_ == Three || lastdiscard.type_ == ThreePlus)
			&& cards0.size() == 0)
		{
			selection.Clear();
		}
	}


    if (analyse_.size() > 2 && selection_.value_ > 14)
    {
        if (selection_.value_ - lastdiscard.value_ > 5 || selection_.value_ >= 13 || lastdiscard.value_ >= 13)
        {
            selection.Clear();//������������2��������ѡ��Ȩֵ����14��A�����򲻳���;
        }
    }
    if (analyse_.size() == 2)
    {
        int index = 0;
        for (auto& item : analyse_)
        {
            for (auto& mem : nextplayer_->analyse_)
            {
                if (mem->type_ == item->type_
                    && mem->count_ == item->count_
                    && mem->value_ > item->value_)
                {
                    ++index;
                    break;
                }
            }
        }
        if (index == 2)
        {
            selection.Clear();
        }
    }
    return;
}

void card_analyse::Enemy(const card_analyse* lastone, const CardGroup& lastdiscard, CardGroup& selection, bool hint)
{
    //��ɻ�����;
    ClearAnalyse();
    DivideIntoGroups();
    sort(analyse_.begin(), analyse_.end(), MyCompare);

    for (auto& mem : analyse_)//�鿴�Ƿ�����Ӧ�ƣ�����Ȩֵ��;
    {
        if (mem->type_ == lastdiscard.type_ 
            && mem->count_ == lastdiscard.count_ 
            && mem->value_ > lastdiscard.value_)
        {
            selection = *mem;
            return;
        }
    }
   
    //��Ҫ����;
    switch (lastdiscard.type_)
    {
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

    if (lastdiscard.type_ == Double 
        || lastdiscard.type_ == Three
        || lastdiscard.type_ == ThreePlus)
    {
        if (selection.value_ > 13 && selection.value_ > lastdiscard.value_ + 5)
        {
            auto temp = (card_analyse*)lastone;
            temp->DivideIntoGroups();
            if (lastone->analyse_.size() > 3)
            {
                selection.Clear();
                return;
            }
        }
    }
    
    if (selection.count_)
        return;
    if (analyse_.size() < 3)
    {
        for (auto& mem : analyse_)
        {
            if (mem->type_ == Bomb)
            {
                if (lastdiscard.type_ == Bomb //�������������Ϊը����;
                    && lastdiscard.value_ <= mem->value_)//���Լ���ը�������ڶԷ�ʱ��;
                {
                    selection = *mem;
                    return;
                }
            }
        }
    }
    if (lastone)
    {
        auto temp = (card_analyse*)lastone;
        temp->DivideIntoGroups();
        //�з�����С��5����������С��2, ���ʺϵ�ը�����ͳ�ը��;;
        if (lastone->cards_.size() <= 8 || lastone->analyse_.size() <=3)
        {
			bool selected = false;
			std::vector<std::shared_ptr<CardGroup>> rem_analye; //��ǰ��ȥѡ����ը������;
            for (auto mem : analyse_)
            {
                if (mem->type_ == Bomb)
                {
					if (lastdiscard.type_ == Bomb //�������������Ϊը����;
						&& mem->value_ <= lastdiscard.value_)//���Լ���ը�������ڶԷ�ʱ��;
					{
						rem_analye.push_back(mem);
						continue;//����ѡ�����
					}
                        
					if (!selected)
					{
						selection = *mem;
						selected = true;
						//return;
					}
					else
					{
						rem_analye.push_back(mem);
					}
                }
				else
				{
					rem_analye.push_back(mem);
				}
            }
			//�жϵ�ǰ��ը���Ƿ����;
			int allsize = 0;
			for (auto& item : rem_analye)
			{
				for (auto& mem : lastone->analyse_)//���ҵз������Լ�����Ӧ�ƣ�����Լ��г���������С�ڶԷ����򲻳�ը��;
				{
                    if (mem->type_ == item->type_
                        && mem->count_ == item->count_
                        && mem->value_ > item->value_)
					{
						++allsize;
						break;
					}
					else if (mem->type_ == Bomb)
					{
						if (item->type_ == Bomb && mem->value_ > item->value_)
						{
							++allsize;
							break;
						}
						else
						{
							++allsize;
							break;
						}
					}
				}
			}
			if (allsize >2)
			{
				selection.Clear();
			}
        }
		
		//����з�ֻʣһ���ƣ�����ʣ�µ��������Ǳ������ѷ�����Ҫ����������;
        if (lastone->analyse_.size() == 1)
        {
            auto& e_cards = lastone->analyse_[0];
            if (e_cards->type_ == Bomb)//�з�ʣ��һ������ը�������ж��Լ�����;
            {
                int index = 0;
                for (auto& mem : analyse_)//����Լ���"����1����"С�ڶԷ�����ذ�;
                {
                    if (mem->type_ == Bomb)
                    {
                        if (mem->value_ <= lastdiscard.value_)
                        {
                            ++index;
                        }
                    }
                    else
                    {
                        ++index;
                    }
                }
                if (index > 1)
                {
                    return;
                }
            }
            if (selection.count_ == 0)
            {
	            ForceDevideCards(lastdiscard, selection, true);
				if (selection.count_)
	        		return;
            }
        }
    }
    return;
}

void card_analyse::NeedSigle(const CardGroup& lastdiscard, CardGroup& selection)
{
    for (auto mem : analyse_)
    {
        if (mem->type_ == SingleSeq && mem->count_ > 5)//����,��˳��������5��;
        {
            if (mem->group_.begin()->first > lastdiscard.value_)
            {
                selection.AddNumber(*mem->cards_.begin());
                selection.value_ = mem->group_.begin()->first;
                selection.type_ = Single;
                ClearAnalyse();//�����ˣ�һ��Ҫ���
                return;
            }
            else if (mem->group_.rbegin()->first > lastdiscard.value_)
            {
                selection.AddNumber(*(mem->cards_.rbegin()));
                selection.value_ = mem->value_;
                selection.type_ = Single;
                ClearAnalyse();//�����ˣ�һ��Ҫ���
                return;
            }
        }
    }
    for (auto mem : analyse_)
    {
        if (mem->type_ == Three)//���,������;
        {
            if (mem->group_.begin()->first > lastdiscard.value_)
            {
                selection.AddNumber(*mem->cards_.begin());
                selection.value_ = mem->group_.begin()->first;
                selection.type_ = Single;
                ClearAnalyse();//�����ˣ�һ��Ҫ���
                return;
            }
        }
    }
    for (auto mem : analyse_)
    {
        if (mem->type_ == Double)//����,�����;
        {
            if (mem->group_.begin()->first > lastdiscard.value_)
            {
                selection.AddNumber(*mem->cards_.begin());
                selection.value_ = mem->group_.begin()->first;
                selection.type_ = Single;
                ClearAnalyse();//�����ˣ�һ��Ҫ���
                return;
            }
        }
    }
	auto enemy = GetEmemy();
	if (enemy)
	{
		if (enemy->analyse_.size() < 3 || enemy->cards_.size() < 8)
		{
			for (auto& mem : analyse_)
			{
				if (mem->value_ == 15 && mem->type_ == Bomb)//����,��ը��2;
				{
					if (mem->group_.begin()->first > lastdiscard.value_)
					{
						selection.AddNumber(*mem->cards_.begin());
						selection.value_ = mem->group_.begin()->first;
						selection.type_ = Single;
						ClearAnalyse();//�����ˣ�һ��Ҫ���;
						return;
					}
				}
			}
		}
	}
}

void card_analyse::NeedDouble(const CardGroup& lastdiscard, CardGroup& selection)
{
    for (auto mem : analyse_)
    {
        if (mem->type_ == Three)//������;
        {
            if (mem->group_.begin()->first > lastdiscard.value_)
            {
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
    for (auto mem : analyse_)
    {
        int i = 0, m = 0;
        if (mem->type_ == ThreeSeq)//����˳;
        {
            if (mem->group_.begin()->first > lastdiscard.value_)
            {
                auto b = mem->cards_.begin();
                for (int i = 0; i < 2; ++i)
                    selection.AddNumber(*b++);
                selection.value_ = mem->group_.begin()->first;
                selection.type_ = Double;
                ClearAnalyse();//�����ˣ�һ��Ҫ���
                return;
            }
            else if (mem->group_.rbegin()->first > lastdiscard.value_)
            {
                selection.AddNumber(*(mem->cards_.rbegin()));
                selection.value_ = mem->value_;
                selection.type_ = Double;
                ClearAnalyse();//�����ˣ�һ��Ҫ���
                return;
            }
        }
    }
	auto enemy = GetEmemy();
	if (enemy)
	{
		if (enemy->analyse_.size() < 3 || enemy->cards_.size() < 8)
		{
			for (auto& mem : analyse_)
			{
				if (mem->value_ == 15 && mem->type_ == Bomb)//����,��ը��2;
				{
					if (mem->group_.begin()->first > lastdiscard.value_)
					{
						auto b = mem->cards_.begin();
						for (int i = 0; i < 2; ++i)
							selection.AddNumber(*b++);
						selection.value_ = mem->group_.begin()->first;
						selection.type_ = Double;
						ClearAnalyse();//�����ˣ�һ��Ҫ���;
						return;
					}
				}
			}
		}
	}
}

void card_analyse::NeedSigleSeq(const CardGroup& lastdiscard, CardGroup& selection)
{
    for (auto mem : analyse_)
    {
        if (mem->type_ == SingleSeq &&
            mem->value_ > lastdiscard.value_ &&
            mem->count_ > lastdiscard.count_)//������ĵ�˳;
        {
            if (mem->count_ - (mem->value_ - lastdiscard.value_) >= lastdiscard.count_)
            {
                //����˳�ǴӶ̵�˳�Ŀ�ʼ��Ԫ�ػ��С��Ԫ�ؿ�ʼ��
                for (int i = lastdiscard.value_ - lastdiscard.count_ + 2, j = 0;
                    j < lastdiscard.count_; ++j)
                    selection.AddNumber(ValueToNum(mem->cards_, i + j));
                selection.value_ = lastdiscard.value_ + 1;
                selection.type_ = SingleSeq;
                ClearAnalyse();//�����ˣ�һ��Ҫ���
                return;
            }
            else//����˳�Ŀ�ʼԪ�رȶ̵�˳�Ŀ�ʼԪ�ش�;
            {
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

void card_analyse::NeedThreePlus(const CardGroup& lastdiscard, CardGroup& selection)
{
    auto b = analyse_.begin();
    for (; b != analyse_.end(); ++b)//���ұȶԷ�������ͬ�Ƶ�����������;
    {
        if ((*b)->type_ == Three && (*b)->value_ > lastdiscard.value_)
        {
            break;
        }
    }
    if (b == analyse_.end())//���û��
        return;//����
    if (lastdiscard.count_ == 4)//������Ϊ����һ��;
    {
        if (analyse_[0]->type_ == Single)//�е���;
        {
            for (auto m : analyse_[0]->cards_)
                (*b)->AddNumber(m);
            (*b)->type_ = ThreePlus;
            analyse_[0]->type_ = Unkown;
            selection = **b;
            return;
        }
        else//��Ҫ����;
        {
            for (auto mem : analyse_)
            {
                if (mem->type_ == SingleSeq && mem->count_ > 5)//����,��˳��������5��;
                {
                    selection = **b;
                    selection.AddNumber(*mem->cards_.begin());
                    selection.type_ = ThreePlus;
                    ClearAnalyse();//�����ˣ�һ��Ҫ���
                    return;
                }
            }
            for (auto mem : analyse_)
            {
                if (mem->type_ == Three && mem != *b && mem->value_ < 14)//���,������;
                {
                    selection = **b;
                    selection.AddNumber(*mem->cards_.begin());
                    selection.type_ = ThreePlus;
                    ClearAnalyse();//�����ˣ�һ��Ҫ���
                    return;
                }
            }
            for (auto mem : analyse_)
            {
                if (mem->type_ == Double && mem->value_ < 14)//����,�����;
                {
                    selection = **b;
                    selection.AddNumber(*mem->cards_.begin());
                    selection.type_ = ThreePlus;
                    ClearAnalyse();//�����ˣ�һ��Ҫ���
                    return;
                }
            }
        }
    }
    else//����һ��;
    {
        for (auto mem : analyse_)//���Ҷ���;
        {
            if (mem->type_ == Double && mem->value_ < 14)
            {
                for (auto m : mem->cards_)
                    (*b)->AddNumber(m);
                (*b)->type_ = ThreePlus;
                mem->type_ = Unkown;
                selection = **b;
                return;
            }
        }
        for (auto mem : analyse_)
        {
            if (mem->type_ == Three && mem != *b && mem->value_ < 14)//���,������;
            {
                selection = **b;
                int times = 0;
                for (auto& item : mem->cards_)
                {
                    selection.AddNumber(item);
                    if (++times == 2)
                    {
                        break;
                    }
                }
                //for (int i = 0; i < 3; ++i)
                //    selection.AddNumber(*mem->cards_.begin());//bug: cards ֻ������һ���ƣ�������Ҫ������;
                selection.type_ = ThreePlus;
                ClearAnalyse();//�����ˣ�һ��Ҫ���
                return;
            }
        }
    }
}

void card_analyse::NeedAirplane(const CardGroup& lastdiscard, CardGroup& selection)
{
    ClearAnalyse();
    DivideIntoGroups();
    sort(analyse_.begin(), analyse_.end(), MyCompare);

    int wing = 0,//�������
        n = 0;//��˳�������Ƶĸ���
    for (auto mem : lastdiscard.group_)
    {
        if (mem.second == 3)
            ++n;
    }
    if (lastdiscard.count_ == 5 * n)//�ɻ����Ϊ����
        wing = 2;
    else//�ɻ����Ϊ����;
    {
        while (lastdiscard.count_ != 4 * n)
            --n;
        wing = 1;
    }
    auto b = analyse_.begin();
    for (; b != analyse_.end(); ++b)
    {
        if ((*b)->type_ == ThreeSeq &&
            (*b)->count_ == 3 * n &&
            (*b)->value_ > lastdiscard.value_)
            break;
    }
    if (b == analyse_.end())
        return;
    int count = 0;
    for (auto mem : analyse_)
    {
        if (mem->type_ == (wing == 1 ? Single : Double))
            ++count;
    }
    if (count < n)
        return;
    for (auto mem : analyse_)
    {
        if (mem->type_ == (wing == 1 ? Single : Double))
        {
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
    for (auto b = analyse_.begin(); b != analyse_.end(); ++b)
    {
        if ((*b)->type_ == selection_.type_ &&
            (*b)->value_ == selection_.value_ &&
            (*b)->count_ == selection_.count_)//���ǲ���;
        {
            analyse_.erase(b);
            needclear = false;//����Ҫ���
            break;
        }
    }
    if (needclear)//��Ҫ��գ��´γ���Ҫ���·���
        ClearAnalyse();

    for (auto mem : selection_.cards_)
    {
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
    //�����������;
    return DiscardAndClear();
}
//��ҳ���
bool card_analyse::HumanDiscard(card_analyse* lastone)
{
    if (selection_.count_ == 0
        || !IsValid(lastone)) //ѡ�Ʋ����Ϲ涨;
    {
        selection_.Clear();//���ѡ��;
        return false;//�������;
    }
    //��������������������Ƿ����;
    return DiscardAndClear();
}
//����
void card_analyse::Pass(void)
{
    nodiscard_ = true;
    selection_.Clear();
}

card_analyse* card_analyse::GetEmemy()
{
    card_analyse* enemy = nullptr;
    if (nextplayer_ && nextplayer_->IsLandlord())
    {
        enemy = nextplayer_;
    }
    else if (preplayer_ && preplayer_->IsLandlord())
    {
        enemy = preplayer_;
    }
    else if (IsLandlord())
    {
        enemy = nextplayer_;
    }
    return enemy;
}

card_analyse* card_analyse::GetFriend()
{
    if (IsRobot()) //�ǻ�����;
    {
        if (nextplayer_->IsRobot())
        {
            return nextplayer_;
        }
        else if (preplayer_->IsRobot())
        {
            return preplayer_;
        }
    }
    return nullptr;
}

void card_analyse::ExchangeCards(const card_analyse* lastone, CardGroup& lastdiscard)
{
    auto fdPtr = GetFriend();
    if (fdPtr)
    {
        //���Լ����ƺ��ѷ����ƻ���һ��Ȼ���ڿ��ܷ�Ҫ�������;
        std::set<int> fd_cards = fdPtr->cards_;
        std::set<int> self_cards = cards_;

        tempplayer_->AddCards(fd_cards);
        for (auto& c : self_cards)
        {
            tempplayer_->AddCard(c);
        }
        
        CardGroup temp_selection;
        tempplayer_->Enemy(lastone, lastdiscard, temp_selection, false);
        if (temp_selection.count_ > 0)
        {
            //1.�ҳ�ѡ������� �Ǹ������Լ��ģ�Ȼ����ѷ���������ù�������ӵ��Լ��������У����Ҹ��ѷ����������������;
            std::set<int> fd_ex_cards;
            std::set<int> slef_ex_cards;
            for (auto& c : temp_selection.cards_)
            {
                if (self_cards.find(c) != self_cards.end())
                {
                    slef_ex_cards.insert(c);
                }
                else
                {
                    fd_ex_cards.insert(c);
                }
            }
            EraseCards(self_cards, slef_ex_cards);
            EraseCards(fd_cards, fd_ex_cards);
            
            int size = slef_ex_cards.size() - fd_ex_cards.size();
            //2. ����Լ����ƹ��������ѷ������;
            size_t ex_size = 0;
            if (self_cards.size() >= fd_ex_cards.size() && size >= 0)
            {
                auto itr = self_cards.begin();
                while (itr != self_cards.end())
                {
                    if (++ex_size <= fd_ex_cards.size())
                    {
                        fd_cards.insert(*itr);//���ѷ�������;
                        itr = self_cards.erase(itr);
                    }
                    else
                        break;
                }
                for (auto& item: temp_selection.cards_)
                {
                    self_cards.insert(item);
                }
            }
            else if (fd_cards.size() > slef_ex_cards.size() && size <= 0)
            {
                auto itr = fd_cards.begin();
                while (itr != fd_cards.end())
                {
                    if (++ex_size <= slef_ex_cards.size())
                    {
                        self_cards.insert(*itr);//���ѷ�������;
                        itr = fd_cards.erase(itr);
                    }
                    else
                        break;
                }
                for (auto& item : temp_selection.cards_)
                {
                    fd_cards.insert(item);
                }
            }
            else
            {
                //tempplayer_->AddCards(fd_cards);
                //std::set<int> excards = tempplayer_->GetSingleCards(size);
            }
            //3.���ƽ���;
            this->AddCards(self_cards);
            fdPtr->AddCards(fd_cards);
            Enemy(lastone, lastdiscard, selection_, false);//���з����ƻ���ʾ;
        }
    }
    //CheckEmemy();
}

void card_analyse::EraseCards(std::set<int>& cards, std::set<int>& erasecards)
{
    for (auto& c : erasecards)
    {
        cards.erase(c);
    }
}

void card_analyse::ForceDevideCards(const CardGroup& lastdiscard, CardGroup& selection, bool bigflag)
{
    set<int> cardscopy(cards_);//���Ƹ���;
    map<int, int> needanalyse;//���������Ȩֵ-��������;

    for (auto& mem : cardscopy)
    {
        int trans_val = CardGroup::Translate(mem);
        ++needanalyse[trans_val];//�������ƹ������������;
    }

    if (lastdiscard.type_ == Single || lastdiscard.type_ == Double || lastdiscard.type_ == Three || lastdiscard.type_ == ThreePlus)
    {
        int rem = 0;
        int val = 0;
        std::vector<int> three_cards;

        for (auto& mem : needanalyse)
        {
            bool cmp_cond;
            if (bigflag)
            {
                cmp_cond = mem.first > lastdiscard.value_;
            }
            else
            {
                cmp_cond = mem.first < lastdiscard.value_;
            }

            switch (lastdiscard.type_)
            {
            case Single:
                if (cmp_cond)
                {
                    selection.type_ = Single;
                    selection.value_ = mem.first;
                    selection.AddNumber(ValueToNum(cardscopy, mem.first));
                    return;
                }
                break;
            case Double:
            case Three:
                if (mem.second == lastdiscard.type_ && cmp_cond)
                {
                    selection.type_ = lastdiscard.type_;
                    selection.value_ = mem.first;
                    for (int i = 0; i < mem.second; ++i)
                    {
                        selection.AddNumber(ValueToNum(cardscopy, mem.first));
                    }
                    return;
                }
                break;
            case ThreePlus:
                if (lastdiscard.count_ == 4 && rem == 0)//��һ��;
                {
                    if (mem.second == 1 || mem.second == 2)//�費��Ҫ�жϵ��Ż��߶��ӵķ�Χ < 14;
                    {
                        three_cards.push_back(mem.first);
                        rem = mem.first;
                    }
                }
                if (lastdiscard.count_ == 5 && rem == 0)//��һ��;
                {
                    if (mem.second == 2)
                    {
                        for (int i = 0; i < mem.second; ++i)
                        {
                            three_cards.push_back(mem.first);
                        }
                        rem = mem.first;
                    }
                }
                if (mem.second == 3 && cmp_cond && val == 0) //val > 0 ��ʾ����һ�����Ѿ�����ˣ�������жϣ����ܻ��ظ����;
                {
                    for (int i = 0; i < mem.second; ++i)
                    {
                        three_cards.push_back(mem.first);
                    }
                    val = mem.first;
                }
                break;
            default:
                break;
            }
        }//for (auto& mem : needanalyse)

        if (three_cards.size() > 3)
        {
            selection.type_ = lastdiscard.type_;
            selection.value_ = val;
            for (int i : three_cards)
            {
                selection.AddNumber(ValueToNum(cardscopy, i));
            }
        }
    }
}

bool card_analyse::CheckNextPlayer(card_analyse* nextplayer, CardGroup& selection)
{
    if (nextplayer == nullptr)
    {
        return false;
    }
    nextplayer->DivideIntoGroups();
    if (!nextplayer->IsLandlord() && !IsLandlord())//�¼�Ϊ�ѷ���������ֻ��һ���ƣ���������;
    {
        if (nextplayer->analyse_.size() == 1)//�¼�������Ϊ1;
        {
            ForceDevideCards(*nextplayer->analyse_[0], selection, false);
        }
        else if (nextplayer->analyse_.size() == 2) 
        {
            for (auto& item : analyse_)
            {
                CardGroup temp;
                nextplayer->Enemy(this, *item, temp, false);
                if (temp.count_ > 0 && temp.type_ != Bomb)
                {
                    selection = *item;
                    return true;
                }
            }
        }
    }
    else//if (nextplayer_->IsLandlord() || IsLandlord())//�¼�Ϊ�з����¼�Ϊ���������Լ��ǵ���������Ҫ�¼�Ҫ����;
    {
        if (nextplayer->analyse_.size() == 1) //�¼�������Ϊ1;
        {
            std::vector<std::shared_ptr<CardGroup>> small_cards; //��ǰ�������С�ڵз���������;
            auto& e_card = *nextplayer->analyse_[0];
            for (auto& item : analyse_)
            {
                if (item->type_ == e_card.type_ && item->value_ < e_card.value_)
                {
                    small_cards.push_back(item);
                }
                if (item->type_ != e_card.type_ && item->type_ != Bomb && item->value_ < 14)
                {
                    selection = *item;
                    return true;
                }
            }
            if (e_card.count_ > 1 && small_cards.size() > 1)
            {
                auto mem = small_cards[0];
                if (mem->type_ == Double || mem->type_ == Three)
                {
                    selection.AddNumber(*mem->cards_.begin());
                    selection.value_ = mem->group_.begin()->first;
                    selection.type_ = Single;
                    return true;
                }
            }
            for (auto& item : analyse_)
            {
                if (item->type_ == e_card.type_ && item->value_ < e_card.value_)
                {
                    continue;
                }
                else
                {
                    selection = *item;
                    return true;
                }
                if (item->type_ != e_card.type_)
                {
                    selection = *item;
                    return true;
                }
            }
        }
        if (nextplayer->analyse_.size() == 2)
        {
            int index = 0;
            std::vector<CardGroup>  analyse;
            for (auto& item : analyse_)
            {
                analyse.push_back(*item);
            }
            for (auto& item : analyse)
            {
                CardGroup temp;
                nextplayer->Enemy(this, item, temp, false);
                if (temp.count_ > 0)
                {
                    CardGroup thistemp;
                    this->Enemy(nextplayer, temp, thistemp, false);
                    if (thistemp.count_ > 0)
                    {
                        selection = item;
                        return true;
                    }
                }
            }
        }
        else//�жϵз����ƣ�����¼ҵ��ƱȽ϶࣬��Ӧ�ô���;
        {
            std::map<int, std::vector<int>> type_map;
            for (auto& item : nextplayer_->analyse_)
            {
                if ((item->type_ == Single && item->value_ < 12)//�ҵ��˵���С��K�ĵ���;
                    || (item->type_ == Double && item->value_ < 10)//����С��Q�Ķ���;
                    || (item->type_ == Three && item->value_ < 9)) //С��10������һС��;
                {
                    type_map[item->type_].push_back(item->value_);
                }
            }
            //���Ƴ��Է����ţ�����, ����һ;
            for (auto& mem : analyse_)
            {
                auto itr = type_map.find(mem->type_);
                if (itr != type_map.end())
                {
                    auto& s_card = itr->second;
                    int size = s_card.size() - 1;
                    if (mem->value_ > s_card[size] && mem->value_ < 13)
                    {
                        selection = *mem;
                        return true;
                    }
                }
                else
                {
                    if ((mem->type_ == Single || mem->type_ == Double || mem->type_ == Three || mem->type_ == ThreePlus || mem->type_ != Bomb)
                        && mem->value_ < 15)
                    {
                        selection = *mem;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

 void card_analyse::GetNextPlayerCard(card_analyse* nextplayer, const CardGroup& lastdiscard, int selectval, std::vector<std::shared_ptr<CardGroup>>& cards)
{
    if (nextplayer == nullptr)
    {
        return;
    }
    nextplayer->DivideIntoGroups();
    for (auto& mem : nextplayer->analyse_)//������Ӧ��;
    {
        if (mem->type_ == lastdiscard.type_
            && mem->count_ == lastdiscard.count_
            && mem->value_ > lastdiscard.value_
            && mem->value_ < selectval)
        {
            cards.push_back(mem);
        }
    }
}

 bool card_analyse::CheckEnemyCard(card_analyse* enemy, CardGroup& selection)
 {
	 if (enemy->IsLandlord() || IsLandlord())//�¼�Ϊ�з����¼�Ϊ���������Լ��ǵ���������Ҫ�¼�Ҫ����;
	 {
		 if (enemy->analyse_.size() == 1) //�¼�������Ϊ1;
		 {
			 std::vector<std::shared_ptr<CardGroup>> small_cards; //��ǰ�������С�ڵз���������;
			 auto& e_card = *enemy->analyse_[0];
			 for (auto& item : analyse_)
			 {
				 if (item->type_ == e_card.type_ && item->value_ < e_card.value_)
				 {
					 small_cards.push_back(item);
				 }
				 if (item->type_ != e_card.type_ && item->type_ != Bomb && item->value_ < 14)
				 {
					 selection = *item;
					 return true;
				 }
			 }
			 if (e_card.count_ > 1 && small_cards.size() > 1)
			 {
				 auto& mem = small_cards[0];
				 if (mem->type_ == Double || mem->type_ == Three)
				 {
					 selection.AddNumber(*mem->cards_.begin());
					 selection.value_ = mem->group_.begin()->first;
					 selection.type_ = Single;
					 return true;
				 }
			 }
			 for (auto& item : analyse_)
			 {
				 if (item->type_ == e_card.type_)//ͬ���͵���������¼�С�����ѡ���˾�ֱ�����¼�����;
				 {
					 if (item->value_ < e_card.value_)
					 {
						 continue;
					 }
					 else
					 {
						 selection = *item;
						 return true;
					 }
				 }
				 else
				 {
					 if (small_cards.size() == 1) //С�ڵз�����ֻ��һ�֣������ֱ�ӳ���ͬ���͵���;
					 {
						 selection = *item;
						 return true;
					 }
					 else
					 {
						 if (item->type_ == Bomb)//�з�ֻʣһ���ƣ������ʱѡ�����ը���������ը����ʣ����С��;
						 {
							 if (item->count_ == 4)
							 {
								 bool add = false;
								 int times = 0;
								 for (auto& cads : small_cards)
								 {
									 if (cads->type_ == Single) //�ճ�4��2;
									 {
										 add = true;
										 for (auto m : cads->cards_)
										 {
											 if (++times > 2)
											 {
												 break;
											 }
											 selection.AddNumber(m);
										 }
									 }
									 if (cads->type_ == Double)//�ճ�4��һ��;
									 {
										 add = true;
										 for (auto m : cads->cards_)
										 {
											 selection.AddNumber(m);
										 }
										 break;
									 }
								 }
								 if (add)
								 {
									 for (auto m : item->cards_)
										 selection.AddNumber(m);
									 //selection = *item;
									 return true;
								 }
							 }//if (item->count_ == 4)
							 else //��ը��ֱ�ӳ�����Ϊ�г���������С�ڶԷ�������4��2;
							 {
								 continue;
							 }
						 }//if (item->type_ == Bomb)
						 else
						 {
							 selection = *item;
							 return true;
						 }
					 }
				 }
			 }
		 }
		 if (enemy->analyse_.size() == 2)
		 {
			 //���ԭ���Լ������������ item, �Է����� temp.count > 0, ��Ҳ��Ҫ: thistemp.count > 0;
			 std::vector<CardGroup>  analyse;
			 for (auto& item : analyse_)
			 {
				 analyse.push_back(*item);
			 }
			 //bug: ��Ϊ this->Enemy ���޸� analyse_, ��������������ʧЧ, �������copy�� std::vector<CardGroup>  analyse; fixed;
			 for (auto& item : analyse)
			 {
				 CardGroup temp;
				 enemy->Enemy(this, item, temp, false);
				 if (temp.count_ > 0)
				 {
					 CardGroup thistemp;
					 this->Enemy(enemy, temp, thistemp, false);
					 if (thistemp.count_ > 0)
					 {
						 selection = item; //bug: ���������fixed;
						 return true;
					 }
				 }
			 }
		 }
		 else//�жϵз����ƣ�����¼ҵ��ƱȽ϶࣬��Ӧ�ô���;
		 {
			 std::map<int, std::vector<int>> type_map;
			 for (auto& item : nextplayer_->analyse_)
			 {
				 if ((item->type_ == Single && item->value_ < 12)//�ҵ��˵���С��K�ĵ���;
					 || (item->type_ == Double && item->value_ < 10)//����С��Q�Ķ���;
					 || (item->type_ == Three && item->value_ < 9)) //С��10������һС��;
				 {
					 type_map[item->type_].push_back(item->value_);
				 }
			 }
			 //���Ƴ��Է����ţ�����, ����һ;
			 for (auto& mem : analyse_)
			 {
				 auto itr = type_map.find(mem->type_);
				 if (itr != type_map.end())
				 {
					 auto& s_card = itr->second;
					 int size = s_card.size() - 1;
					 if (mem->value_ > s_card[size] && mem->value_ < 13)
					 {
						 selection = *mem;
						 return true;
					 }
				 }
				 else
				 {
					 if ((mem->type_ == Single || mem->type_ == Double || mem->type_ == Three || mem->type_ == ThreePlus || mem->type_ != Bomb)
						 && mem->value_ < 15)
					 {
						 selection = *mem;
						 return true;
					 }
				 }
			 }
		 }
	 }
 }