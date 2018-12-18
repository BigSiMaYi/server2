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

//开始新的一局，做一些初始化集合等的操作
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
//回调函数
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

    if (questioned == 2 && nowscore == 0)//如果前两位都未叫牌，直接3分当地主;
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
    map<int, int> needanalyse;//方便分析的权值-数量集合;
    for (auto& mem : cards)
    {
        ++needanalyse[CardGroup::Translate(mem)];//根据手牌构造待分析集合;
    }

    int sum = 0;
    if (needanalyse.find(16) != needanalyse.end()
        && needanalyse.find(17) != needanalyse.end())//存在王炸;
    {
        sum += 8;
    }
    else if (needanalyse.find(17) != needanalyse.end())//一张大王;
    {
        sum += 4;
    }
    else if (needanalyse.find(16) != needanalyse.end())//一张小王;
    {
        sum += 3;
    }
    if (needanalyse.find(15) != needanalyse.end())//2的数量;
    {
        if (needanalyse[15] < 4)
        {
            sum += 2 * needanalyse[15];
        }
    }

    for (auto& mem : needanalyse)
    {
        if (mem.second == 4)//炸弹;
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

//分析选牌是否符合规定
bool card_analyse::IsValid(card_analyse* lastone)
{
    if (lastone)
    { 
        if (lastone->discard_.count_ != selection_.count_
            && selection_.count_ != 4 
            && selection_.count_ != 2)//跟牌，但数量不符且不可能为炸弹;
        {
            return false;
        }
    }

    selection_.type_ = Unkown;
    AnalyseSelection();//分析所选牌的类型及权值;

    if (selection_.type_ == Unkown)//所选牌不符合规定;
        return false;

    if (lastone)
    {
        if (selection_.type_ == Bomb
            && (lastone->discard_.type_ != Bomb || selection_.value_ > lastone->discard_.value_))
        {
            return true;
        }
        if (selection_.type_ != lastone->discard_.type_
            || selection_.count_ != lastone->discard_.count_)//类型不符或数量不符;
        {
            return false;
        }
        if (selection_.value_ <= lastone->discard_.value_)//选牌不大于上家牌;
        {
            return false;
        }
    }
    return true;
}
//对选牌进行分析
void card_analyse::AnalyseSelection()
{
    int NumMax = 0,//同牌面的最大数量
        ValueMax = 0;//最大数量的最大权值

    //判断是否为王炸
    if (selection_.count_ == 2 
        && selection_.group_.find(16) != selection_.group_.end() 
        && selection_.group_.find(17) != selection_.group_.end())
    {
        selection_.type_ = Bomb;
        selection_.value_ = 17;
        return;
    }
    //找出相同牌面的最大数量，和最大权值
    for (auto mem : selection_.group_)
    {
        if (mem.second >= NumMax && mem.first > ValueMax)
        {
            NumMax = mem.second;
            ValueMax = mem.first;
        }
    }
    //根据牌面相同的最大数量判断类型
    switch (NumMax)
    {
    case 4:
        if (selection_.count_ == 4)//炸弹;
        {
            selection_.type_ = Bomb;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ == 6)//四带两张;
        {
            selection_.type_ = FourSeq;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ == 8)//四带两对;
        {
            for (auto mem : selection_.group_)
            {
                if (mem.second != 2 && mem.second != 4)//牌面不合规
                    return;
            }
            selection_.type_ = FourSeq;
            selection_.value_ = ValueMax;
            return;
        }
        return;//牌面不合规
    case 3:
    {
        if (selection_.count_ == 3)//三条;
        {
            selection_.type_ = Three;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ == 4)//三带一张;
        {
            selection_.type_ = ThreePlus;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ == 5)//三带两张;
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
        for (auto mem : selection_.group_)//判断连续的3张牌面的最大数量;
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
        if (selection_.count_ == 3 * n)//三顺;
        {
            selection_.type_ = ThreeSeq;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ == 4 * n)//飞机带单张的翅膀;
        {
            selection_.type_ = Airplane;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ == 5 * n)//飞机带对子翅膀;
        {
            for (auto mem : selection_.group_)
            {
                if (mem.second != 2 && mem.second != 3)//牌不合规;
                    return;
            }
            selection_.type_ = Airplane;
            selection_.value_ = ValueMax;
            return;
        }
        return;//牌不合规
    }
    case 2:
        if (selection_.count_ == 2)//一对;
        {
            selection_.type_ = Double;
            selection_.value_ = ValueMax;
            return;
        }
        if (selection_.count_ >= 6 && !(selection_.count_ % 2))//连对;
        {
            int begin = 0;
            for (auto mem : selection_.group_)//确定牌是连续的，并且都是成对的;
            {
                if (!begin)
                    begin = mem.first;
                if (begin++ != mem.first || mem.second != 2)//牌不符合规定;
                    return;
            }
            selection_.type_ = DoubleSeq;
            selection_.value_ = ValueMax;
            return;
        }
        return;//牌不符合规定;
    case 1:
        if (selection_.count_ == 1)//单张;
        {
            selection_.type_ = Single;
            selection_.value_ = ValueMax;
            return;
        }
        else if (selection_.count_ >= 5)//判断是否为顺子;
        {
            int begin = 0;
            for (auto mem : selection_.group_)
            {
                if (!begin)
                    begin = mem.first;
                if (begin++ != mem.first || mem.first >= 15)//牌不是连续的或者带了2及以上的牌;
                    return;
            }
            selection_.type_ = SingleSeq;//单顺;
            selection_.value_ = ValueMax;
            return;
        }
    default://下落，不符合规定;
        return;
    }
}

//给定权值，从集合中查找相应0-53数字，然后从集合中删除并返回该数字；不存在或无效返回-1
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

//删除分析堆中数量为零的元素;
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

//拆分手牌牌型并组成基本牌集合;
void card_analyse::DivideIntoGroups(void)
{
    if (!analyse_.empty())//牌型集合非空，返回
        return;

    set<int> cardscopy(cards_);//手牌副本
    map<int, int> needanalyse;//方便分析的权值-数量集合

    for (auto& mem : cardscopy)
    {
        int trans_val = CardGroup::Translate(mem);
        ++needanalyse[trans_val];//根据手牌构造待分析集合
    }

    if (needanalyse.find(16) != needanalyse.end() 
        && needanalyse.find(17) != needanalyse.end())//满足条件存在王炸;
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
        if (mem.second == 4)//炸弹;
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
    //删除分析堆中数量为零的元素
    FreshenMap(needanalyse);

    //提前处理2
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
    //查找单顺
    int begin, n;
    bool exist = true;
    while (exist && !needanalyse.empty())
    {
        begin = n = 0;
        for (auto itr = needanalyse.begin(); itr != needanalyse.end(); ++itr)
        {
            if (itr->second > 0)//跳过为零的元素;
            {
                if (begin == 0)
                    begin = itr->first;
                if (begin == itr->first)
                    ++n;
                ++begin;//值加一，表示下一张牌;
            }
            if (n == 5)//满足组成单顺的数量;
            {
                auto p = itr;
                if (begin - 1 != itr->first)
                    --p;
                int first = p->first - 4;//单顺的第一个
                auto c = std::make_shared<CardGroup>(SingleSeq, p->first);
                for (first; first <= p->first; ++first)
                {
                    c->AddNumber(ValueToNum(cardscopy, first));
                    --needanalyse[first];//减一
                }
                analyse_.push_back(c);
                exist = true;
                break;//从开始重新查找
            }
            //连续牌面数量小于五个，重新计数；或者已到集合最后数量仍不满足
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

    //删除分析堆中数量为零的元素;
    FreshenMap(needanalyse);
    //如可能，继续往单顺中添加剩余牌;
    for (auto mem : analyse_)
    {
        if (mem->type_ == SingleSeq)//针对每个单顺;
        {
            for (auto m : needanalyse)
            {
                if (m.second > 0 && m.first == mem->value_ + 1)//剩余牌中还有比单顺最大大一的牌;
                {
                    mem->AddNumber(ValueToNum(cardscopy, m.first));
                    ++mem->value_;
                    --needanalyse[m.first];
                }
            }
        }
    }
    //删除分析堆中数量为零的元素
    FreshenMap(needanalyse);

    //如现有单顺中有可以对接成更长的单顺；或两个单顺元素相同，组合成双顺
    for (auto mem1 : analyse_)
    {
        if (mem1->type_ == SingleSeq)//单顺1;
        {
            for (auto mem2 : analyse_)
            {
                if (mem2->type_ == SingleSeq && mem1 != mem2)//单顺2，且和单顺1不是同一个
                {
                    if (mem1->value_ < mem2->value_)//mem1在前;
                    {
                        if (mem1->value_ == mem2->value_ - mem2->count_)//可以拼接
                        {
                            for (auto m : mem2->cards_)
                                mem1->AddNumber(m);
                            mem1->value_ = mem2->value_;
                            mem2->type_ = Unkown;
                        }
                    }
                    else if (mem1->value_ > mem2->value_)//mem1在后;
                    {
                        if (mem2->value_ == mem1->value_ - mem1->count_)
                        {
                            for (auto m : mem1->cards_)
                                mem2->AddNumber(m);
                            mem2->value_ = mem1->value_;
                            mem1->type_ = Unkown;
                        }
                    }
                    else//测试是否完全一样，可以合并成双顺;
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
    if (needanalyse.empty())//分析集合已空，返回;
    {
        DeleteUnkown();
        sort(analyse_.begin(), analyse_.end(), MyCompare);
        return;
    }

    //双顺，只查找数量大于等于2的连续牌，并且3个以上相连
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
        if (begin && begin - 1 != b->first || b == last)//出现与之前不连续的,或已到集合最后;
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
                n = 1;//当前分析牌是两张以上的;
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

    //删除分析堆中数量为零的元素
    FreshenMap(needanalyse);

    //三顺
    //查找是否有重合的单顺和双顺组合成三顺
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
    //剩余牌中查找三顺
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
        if (begin && begin - 1 != b->first || b == last)//出现与之前不连续的,或已到集合最后;
        {
            if (n >= 2)//存在2组及以上;
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
                if (b->second == 3)//当前分析牌为3张，;
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
    //三条
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

    //对子
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
    //删除分析堆中数量为零的元素
    FreshenMap(needanalyse);

    //单牌
    for (auto& mem : needanalyse)
    {
        if (mem.second != 1)
            throw runtime_error("Still has singleness card");
        auto c = std::make_shared<CardGroup>(Single, mem.first);
        c->AddNumber(ValueToNum(cardscopy, mem.first));
        needanalyse[mem.first] = 0;
        analyse_.push_back(c);
    }
    //删除分析堆中数量为零的元素
    FreshenMap(needanalyse);

    DeleteUnkown();
    sort(analyse_.begin(), analyse_.end(), MyCompare);
}

//由三条、三顺完善成三带一和飞机；先找单牌，再找对子，均不够就保持原样
void card_analyse::ThreeplusAndAirplane()
{
    int n,
        doublecount = 0,//统计对子的数量，方便下面的整合
        singlecount = 0;//统计单张数量

    for (auto mem : analyse_)
    {
        if (mem->type_ == Single)
            ++singlecount;
        else if (mem->type_ == Double)
            ++doublecount;
    }

    for (auto mem : analyse_)//完善飞机;
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
    for (auto mem : analyse_)//完善三带一;
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

//删除所有未知类型的牌型
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

//电脑选牌
void card_analyse::SelectCards(card_analyse* lastone, bool hint)
{
    if (analyse_.empty())//是否需要重新分析手牌
        DivideIntoGroups();
    ThreeplusAndAirplane();
    DeleteUnkown();
    sort(analyse_.begin(), analyse_.end(), MyCompare);

    if (analyse_.size() == 2)//手数为2，且有适合的炸弹直接出;
    {
        for (auto& mem : analyse_)
        {
            if (mem->type_ == Bomb)
            {
                if (lastone != nullptr //如果自己是接别人的牌;
                    && lastone->discard_.type_ == Bomb  //别人最后出牌为炸弹;
                    && mem->value_ <= lastone->discard_.value_)//且自己的炸弹不大于对方时;
                    continue;//不能选择改牌;
                selection_ = *mem;
                return;
            }
        }
    }

    if (lastone == nullptr)
    {
        Myself(selection_);//直接出牌
    }
    else if (!hint && !IsLandlord() && !lastone->IsLandlord())
    {
        auto lastdiscard = lastone->discard_;//敌方出牌
        Friend(lastone, lastdiscard, selection_);//跟友方牌：最后出牌的是友方,并且不是提示
    }
    else
    {
        auto& lastdiscard = lastone->discard_;//敌方出牌
        Enemy(lastone, lastdiscard, selection_, hint);//跟敌方的牌或提示
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
    if (analyse_.size() == 1)//剩最后一手牌;
    {
        selection = *analyse_[0];
        return;
    }

    if (analyse_.size() == 2)//剩两手牌，出最大的那组;
    {
        //“查看”其它玩家手牌，只为分析剩余牌中的最大的: 判断敌方牌，如果我的手牌敌方不能要，则出这手牌;
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
        selection = *analyse_[0]; //上面检测如果自己的所有牌对方都可以要，则出小牌;
        return;
    }

	if (CheckNextPlayer(nextplayer_, selection))
	{
		if (selection.count_ > 0)
		{
			return;
		}
	}
	//检测上家手牌;
	if (CheckEnemyCard(preplayer_, selection))
	{
		if (selection.count_ > 0)
		{
			return;
		}
	}
    //正常顺序出牌：(A以上的牌尽量不直接出、炸弹不直接出);
    //单牌→对子→双顺→单顺→三条、三带一、飞机;
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
	if (analyse_.size() == 1)//剩最后一手牌;
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

    if (preplayer_->IsLandlord())//“上家”是地主不要友方的牌;
    {
        if (lastone->analyse_.size() == 1) //最后出牌的是友方;
        {
            //如果自己手上有炸弹，并且，手上牌可以送队友直接走，则可以出炸弹，然后出小牌送友方走;
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
            //判断友方是不是给自己送牌：依据是友方手上的牌地主可以吃;
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
    if (nextplayer_->IsLandlord()) //下家是地主;
    {
		//最后出牌的是友方，此时判断这手牌下家是否能要，如果不能要，则本方也不要;
        if (lastone->analyse_.size() == 1) //最后出牌的是友方，他只剩一手牌，此时判断这手牌下家是否能要，如果不能要，则本方也不要;
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

    for (auto& mem : analyse_)//查找相应牌;
    {
        if (mem->type_ == lastdiscard.type_
            && mem->count_ == lastdiscard.count_
            && mem->value_ > lastdiscard.value_)
        {
			if (lastdiscard.type_ == Single)
            {
                selection = *mem;
                if (mem->value_ > 9) //从最小的单张开始向上查找单张，如果超过10就出;
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

	//if (selection.count_ == 0)//防止敌方过小牌;
	//{
	//	if (lastdiscard.value_ < 11)//友方出小于11的单张或者对子;
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
	
	//友方大牌超过10,并且敌方没有合适的小牌，不压;
	if (lastdiscard.value_ > 10)//友方对子超过10，敌方手牌
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
            selection.Clear();//手牌手数大于2，并且所选牌权值大于14（A），则不出牌;
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
    //拆成基本牌;
    ClearAnalyse();
    DivideIntoGroups();
    sort(analyse_.begin(), analyse_.end(), MyCompare);

    for (auto& mem : analyse_)//查看是否有相应牌，并且权值大;
    {
        if (mem->type_ == lastdiscard.type_ 
            && mem->count_ == lastdiscard.count_ 
            && mem->value_ > lastdiscard.value_)
        {
            selection = *mem;
            return;
        }
    }
   
    //需要拆牌;
    switch (lastdiscard.type_)
    {
    case Single://敌方出的是单牌
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
    case ThreePlus://三带一
        NeedThreePlus(lastdiscard, selection);
        break;
    case Airplane://飞机，需要组合
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
                if (lastdiscard.type_ == Bomb //如果别人最后出牌为炸弹，;
                    && lastdiscard.value_ <= mem->value_)//且自己的炸弹不大于对方时，;
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
        //敌方手牌小于5，或者手数小于2, 有适合的炸弹，就出炸弹;;
        if (lastone->cards_.size() <= 8 || lastone->analyse_.size() <=3)
        {
			bool selected = false;
			std::vector<std::shared_ptr<CardGroup>> rem_analye; //当前除去选出的炸弹的牌;
            for (auto mem : analyse_)
            {
                if (mem->type_ == Bomb)
                {
					if (lastdiscard.type_ == Bomb //如果别人最后出牌为炸弹，;
						&& mem->value_ <= lastdiscard.value_)//且自己的炸弹不大于对方时，;
					{
						rem_analye.push_back(mem);
						continue;//不能选择该牌
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
			//判断当前出炸弹是否合理;
			int allsize = 0;
			for (auto& item : rem_analye)
			{
				for (auto& mem : lastone->analyse_)//查找敌方大于自己的相应牌，如果自己有超过两手牌小于对方，则不出炸弹;
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
		
		//如果敌方只剩一手牌，并且剩下的这手牌是本方或友方可以要，则必须拆牌;
        if (lastone->analyse_.size() == 1)
        {
            auto& e_cards = lastone->analyse_[0];
            if (e_cards->type_ == Bomb)//敌方剩余一手牌是炸弹，则判断自己手牌;
            {
                int index = 0;
                for (auto& mem : analyse_)//如果自己有"超过1手牌"小于对方，则必败;
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
        if (mem->type_ == SingleSeq && mem->count_ > 5)//首先,拆单顺数量大于5的;
        {
            if (mem->group_.begin()->first > lastdiscard.value_)
            {
                selection.AddNumber(*mem->cards_.begin());
                selection.value_ = mem->group_.begin()->first;
                selection.type_ = Single;
                ClearAnalyse();//拆牌了，一定要清空
                return;
            }
            else if (mem->group_.rbegin()->first > lastdiscard.value_)
            {
                selection.AddNumber(*(mem->cards_.rbegin()));
                selection.value_ = mem->value_;
                selection.type_ = Single;
                ClearAnalyse();//拆牌了，一定要清空
                return;
            }
        }
    }
    for (auto mem : analyse_)
    {
        if (mem->type_ == Three)//其次,拆三条;
        {
            if (mem->group_.begin()->first > lastdiscard.value_)
            {
                selection.AddNumber(*mem->cards_.begin());
                selection.value_ = mem->group_.begin()->first;
                selection.type_ = Single;
                ClearAnalyse();//拆牌了，一定要清空
                return;
            }
        }
    }
    for (auto mem : analyse_)
    {
        if (mem->type_ == Double)//再者,拆对子;
        {
            if (mem->group_.begin()->first > lastdiscard.value_)
            {
                selection.AddNumber(*mem->cards_.begin());
                selection.value_ = mem->group_.begin()->first;
                selection.type_ = Single;
                ClearAnalyse();//拆牌了，一定要清空
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
				if (mem->value_ == 15 && mem->type_ == Bomb)//再者,拆炸弹2;
				{
					if (mem->group_.begin()->first > lastdiscard.value_)
					{
						selection.AddNumber(*mem->cards_.begin());
						selection.value_ = mem->group_.begin()->first;
						selection.type_ = Single;
						ClearAnalyse();//拆牌了，一定要清空;
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
        if (mem->type_ == Three)//拆三条;
        {
            if (mem->group_.begin()->first > lastdiscard.value_)
            {
                auto b = mem->cards_.begin();
                for (int i = 0; i < 2; ++i)
                    selection.AddNumber(*b++);
                selection.value_ = mem->group_.begin()->first;
                selection.type_ = Double;
                ClearAnalyse();//拆牌了，一定要清空
                return;
            }
        }
    }
    for (auto mem : analyse_)
    {
        int i = 0, m = 0;
        if (mem->type_ == ThreeSeq)//拆三顺;
        {
            if (mem->group_.begin()->first > lastdiscard.value_)
            {
                auto b = mem->cards_.begin();
                for (int i = 0; i < 2; ++i)
                    selection.AddNumber(*b++);
                selection.value_ = mem->group_.begin()->first;
                selection.type_ = Double;
                ClearAnalyse();//拆牌了，一定要清空
                return;
            }
            else if (mem->group_.rbegin()->first > lastdiscard.value_)
            {
                selection.AddNumber(*(mem->cards_.rbegin()));
                selection.value_ = mem->value_;
                selection.type_ = Double;
                ClearAnalyse();//拆牌了，一定要清空
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
				if (mem->value_ == 15 && mem->type_ == Bomb)//再者,拆炸弹2;
				{
					if (mem->group_.begin()->first > lastdiscard.value_)
					{
						auto b = mem->cards_.begin();
						for (int i = 0; i < 2; ++i)
							selection.AddNumber(*b++);
						selection.value_ = mem->group_.begin()->first;
						selection.type_ = Double;
						ClearAnalyse();//拆牌了，一定要清空;
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
            mem->count_ > lastdiscard.count_)//拆更长的单顺;
        {
            if (mem->count_ - (mem->value_ - lastdiscard.value_) >= lastdiscard.count_)
            {
                //长单顺是从短单顺的开始的元素或更小的元素开始的
                for (int i = lastdiscard.value_ - lastdiscard.count_ + 2, j = 0;
                    j < lastdiscard.count_; ++j)
                    selection.AddNumber(ValueToNum(mem->cards_, i + j));
                selection.value_ = lastdiscard.value_ + 1;
                selection.type_ = SingleSeq;
                ClearAnalyse();//拆牌了，一定要清空
                return;
            }
            else//长单顺的开始元素比短单顺的开始元素大;
            {
                int i = 0;
                auto b = mem->cards_.begin();
                for (; i < lastdiscard.count_; ++i, ++b)
                    selection.AddNumber(*b);
                selection.value_ = CardGroup::Translate(*--b);
                selection.type_ = SingleSeq;
                ClearAnalyse();//拆牌了，一定要清空
                return;
            }
        }
    }
}

void card_analyse::NeedThreePlus(const CardGroup& lastdiscard, CardGroup& selection)
{
    auto b = analyse_.begin();
    for (; b != analyse_.end(); ++b)//查找比对方三张相同牌的牌面大的三条;
    {
        if ((*b)->type_ == Three && (*b)->value_ > lastdiscard.value_)
        {
            break;
        }
    }
    if (b == analyse_.end())//如果没有
        return;//跳出
    if (lastdiscard.count_ == 4)//最后出牌为三带一张;
    {
        if (analyse_[0]->type_ == Single)//有单牌;
        {
            for (auto m : analyse_[0]->cards_)
                (*b)->AddNumber(m);
            (*b)->type_ = ThreePlus;
            analyse_[0]->type_ = Unkown;
            selection = **b;
            return;
        }
        else//需要拆牌;
        {
            for (auto mem : analyse_)
            {
                if (mem->type_ == SingleSeq && mem->count_ > 5)//首先,拆单顺数量大于5的;
                {
                    selection = **b;
                    selection.AddNumber(*mem->cards_.begin());
                    selection.type_ = ThreePlus;
                    ClearAnalyse();//拆牌了，一定要清空
                    return;
                }
            }
            for (auto mem : analyse_)
            {
                if (mem->type_ == Three && mem != *b && mem->value_ < 14)//其次,拆三条;
                {
                    selection = **b;
                    selection.AddNumber(*mem->cards_.begin());
                    selection.type_ = ThreePlus;
                    ClearAnalyse();//拆牌了，一定要清空
                    return;
                }
            }
            for (auto mem : analyse_)
            {
                if (mem->type_ == Double && mem->value_ < 14)//再者,拆对子;
                {
                    selection = **b;
                    selection.AddNumber(*mem->cards_.begin());
                    selection.type_ = ThreePlus;
                    ClearAnalyse();//拆牌了，一定要清空
                    return;
                }
            }
        }
    }
    else//三带一对;
    {
        for (auto mem : analyse_)//先找对子;
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
            if (mem->type_ == Three && mem != *b && mem->value_ < 14)//其次,拆三条;
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
                //    selection.AddNumber(*mem->cards_.begin());//bug: cards 只加入了一张牌，三带二要加两张;
                selection.type_ = ThreePlus;
                ClearAnalyse();//拆牌了，一定要清空
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

    int wing = 0,//翅膀类型
        n = 0;//单顺中三张牌的个数
    for (auto mem : lastdiscard.group_)
    {
        if (mem.second == 3)
            ++n;
    }
    if (lastdiscard.count_ == 5 * n)//飞机翅膀为对子
        wing = 2;
    else//飞机翅膀为单张;
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
//出牌并重置分析集合
bool card_analyse::DiscardAndClear()
{
    discard_ = selection_;//把选牌放入出牌区：打出选牌
    bool needclear = true;//本次出牌是否为拆牌，需要更新分析牌堆
    for (auto b = analyse_.begin(); b != analyse_.end(); ++b)
    {
        if ((*b)->type_ == selection_.type_ &&
            (*b)->value_ == selection_.value_ &&
            (*b)->count_ == selection_.count_)//不是拆牌;
        {
            analyse_.erase(b);
            needclear = false;//不需要清空
            break;
        }
    }
    if (needclear)//需要清空，下次出牌要重新分析
        ClearAnalyse();

    for (auto mem : selection_.cards_)
    {
        cards_.erase(mem);//从手牌中删除打出牌
    }
    selection_.Clear();//清空选牌区
    return true;
}
//电脑出牌
bool card_analyse::Discard(void)
{
    if (selection_.count_ == 0)//电脑选牌区为空，说明不出;
    {
        nodiscard_ = true;
        return false;
    }
    //否则正常打出;
    return DiscardAndClear();
}
//玩家出牌
bool card_analyse::HumanDiscard(card_analyse* lastone)
{
    if (selection_.count_ == 0
        || !IsValid(lastone)) //选牌不符合规定;
    {
        selection_.Clear();//清空选牌;
        return false;//不允许出;
    }
    //否则正常打出，并分析是否拆牌;
    return DiscardAndClear();
}
//过牌
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
    if (IsRobot()) //是机器人;
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
        //把自己的牌和友方的牌混在一起，然后在看能否要起玩家牌;
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
            //1.找出选择的牌中 那个不是自己的，然后把友方的这个牌拿过来，添加到自己的手牌中，并且给友方补充相等数量的牌;
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
            //2. 检查自己手牌够不够给友方补充的;
            size_t ex_size = 0;
            if (self_cards.size() >= fd_ex_cards.size() && size >= 0)
            {
                auto itr = self_cards.begin();
                while (itr != self_cards.end())
                {
                    if (++ex_size <= fd_ex_cards.size())
                    {
                        fd_cards.insert(*itr);//给友方补充牌;
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
                        self_cards.insert(*itr);//给友方补充牌;
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
            //3.换牌结束;
            this->AddCards(self_cards);
            fdPtr->AddCards(fd_cards);
            Enemy(lastone, lastdiscard, selection_, false);//跟敌方的牌或提示;
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
    set<int> cardscopy(cards_);//手牌副本;
    map<int, int> needanalyse;//方便分析的权值-数量集合;

    for (auto& mem : cardscopy)
    {
        int trans_val = CardGroup::Translate(mem);
        ++needanalyse[trans_val];//根据手牌构造待分析集合;
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
                if (lastdiscard.count_ == 4 && rem == 0)//带一张;
                {
                    if (mem.second == 1 || mem.second == 2)//需不需要判断单张或者对子的范围 < 14;
                    {
                        three_cards.push_back(mem.first);
                        rem = mem.first;
                    }
                }
                if (lastdiscard.count_ == 5 && rem == 0)//带一对;
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
                if (mem.second == 3 && cmp_cond && val == 0) //val > 0 表示三带一的三已经添加了，如果不判断，可能会重复添加;
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
    if (!nextplayer->IsLandlord() && !IsLandlord())//下家为友方，并且它只有一手牌，则送它走;
    {
        if (nextplayer->analyse_.size() == 1)//下家手牌数为1;
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
    else//if (nextplayer_->IsLandlord() || IsLandlord())//下家为敌方：下家为地主或者自己是地主，出牌要下家要不起;
    {
        if (nextplayer->analyse_.size() == 1) //下家手牌数为1;
        {
            std::vector<std::shared_ptr<CardGroup>> small_cards; //当前玩家手上小于敌方的所有牌;
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
        else//判断敌方手牌，如果下家单牌比较多，则不应该打单张;
        {
            std::map<int, std::vector<int>> type_map;
            for (auto& item : nextplayer_->analyse_)
            {
                if ((item->type_ == Single && item->value_ < 12)//找敌人单张小于K的单牌;
                    || (item->type_ == Double && item->value_ < 10)//对子小于Q的对子;
                    || (item->type_ == Three && item->value_ < 9)) //小于10的三带一小于;
                {
                    type_map[item->type_].push_back(item->value_);
                }
            }
            //出牌出对方单张，对子, 三带一;
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
    for (auto& mem : nextplayer->analyse_)//查找相应牌;
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
	 if (enemy->IsLandlord() || IsLandlord())//下家为敌方：下家为地主或者自己是地主，出牌要下家要不起;
	 {
		 if (enemy->analyse_.size() == 1) //下家手牌数为1;
		 {
			 std::vector<std::shared_ptr<CardGroup>> small_cards; //当前玩家手上小于敌方的所有牌;
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
				 if (item->type_ == e_card.type_)//同类型的牌如果比下家小，如果选到了就直接送下家走了;
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
					 if (small_cards.size() == 1) //小于敌方的牌只有一手，则可以直接出不同类型的牌;
					 {
						 selection = *item;
						 return true;
					 }
					 else
					 {
						 if (item->type_ == Bomb)//敌方只剩一手牌，如果此时选择的是炸弹，则出了炸弹还剩两手小牌;
						 {
							 if (item->count_ == 4)
							 {
								 bool add = false;
								 int times = 0;
								 for (auto& cads : small_cards)
								 {
									 if (cads->type_ == Single) //凑成4带2;
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
									 if (cads->type_ == Double)//凑成4带一对;
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
							 else //王炸不直接出，因为有超过两手牌小于对方，不能4带2;
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
			 //检测原则：自己打出的这手牌 item, 对方吃了 temp.count > 0, 我也能要: thistemp.count > 0;
			 std::vector<CardGroup>  analyse;
			 for (auto& item : analyse_)
			 {
				 analyse.push_back(*item);
			 }
			 //bug: 因为 this->Enemy 会修改 analyse_, 导致容器迭代器失效, 因此上面copy到 std::vector<CardGroup>  analyse; fixed;
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
						 selection = item; //bug: 这里崩溃：fixed;
						 return true;
					 }
				 }
			 }
		 }
		 else//判断敌方手牌，如果下家单牌比较多，则不应该打单张;
		 {
			 std::map<int, std::vector<int>> type_map;
			 for (auto& item : nextplayer_->analyse_)
			 {
				 if ((item->type_ == Single && item->value_ < 12)//找敌人单张小于K的单牌;
					 || (item->type_ == Double && item->value_ < 10)//对子小于Q的对子;
					 || (item->type_ == Three && item->value_ < 9)) //小于10的三带一小于;
				 {
					 type_map[item->type_].push_back(item->value_);
				 }
			 }
			 //出牌出对方单张，对子, 三带一;
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