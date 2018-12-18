#include "stdafx.h"
#include "card_group.h"


CardGroup::CardGroup()
    :type_(Unkown)
    , value_(0)
    , count_(0)
{
}

CardGroup::CardGroup(Type t, int v)
    : type_(t)
    , value_(v)
    , count_(0)
{
}

CardGroup& CardGroup::operator=(CardGroup& cg)
{
    this->group_ = cg.group_;
    this->cards_ = cg.cards_;
    this->type_ = cg.type_;
    this->value_ = cg.value_;
    this->count_ = cg.count_;
    return *this;
}

void CardGroup::Clear(void)
{
    group_.clear();
    cards_.clear();
    type_ = Unkown;
    value_ = 0;
    count_ = 0;
}

void CardGroup::AddNumber(int num)
{
    ++count_;
    cards_.insert(num);
    ++group_[Translate(num)];
}

void CardGroup::DeleteNumber(int num)
{
    if (cards_.find(num) == cards_.end())//确定要去掉的牌在结构内
        return;
    --count_;
    cards_.erase(num);
    if (--group_[Translate(num)] == 0)
        group_.erase(Translate(num));
}

int CardGroup::Translate(int num)
{
    if (num < 52)
        return num / 4 + 3;
    else
        return num - 36;
}