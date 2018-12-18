#include "stdafx.h"
#include "card_covert.h"


card_covert::card_covert()
    : landlord_(nullptr)
    , curplayer_(nullptr)
    , lastone_(nullptr)
    , basescore_(0)
    , times_(0)
    , questioned_(0)
    , status_(NOTSTART)
    , callbegin_(0)
{
    for (int i = 0; i < 3; ++i) 
    {
        callscore_[i] = 0;
        player_[i] = new card_analyse(i);
    }
}

card_covert::~card_covert()
{
    for (int i = 0; i < 3; ++i)
    {
        delete player_[i];
    }
}

char card_covert::GetStatus(void)
{
    return status_;
}

void card_covert::Start(void)
{
    Init();
    status_ = GETLANDLORD;
}

void card_covert::SendCard(int32_t seatid, std::vector<POKER>& pokers, bool is_robot)
{
    auto analyser = player_[seatid];
    for (auto& item : pokers)
    {
        int val = covert(item);
        if (val > 0)
        {
            analyser->AddCard(val);
        }
    }
    analyser->SetRobot(is_robot);
}

int card_covert::CalcGetLandScore(int32_t seatid)
{
    if (seatid < sizeof(player_))
    {
        return player_[seatid]->GetBaseScore(questioned_, basescore_);
    }
    return  -1;
}

void card_covert::GetLandlord(int pid, int score)
{
    curplayer_ = player_[pid];
    callscore_[pid] = score;
    ++questioned_;

    if (score == 3)//给出三分就直接当地主;
    {
        basescore_ = score;
        landlord_ = curplayer_;
        lastone_ = nullptr;
    }
    else if (score > basescore_)//否则，给出分数大于上次给分玩家;
    {
        basescore_ = score;
        lastone_ = curplayer_;//就把该玩家记录下来;
    }

    if (questioned_ == 1)
    {
        callbegin_ = pid;
    }
    if (questioned_ == 3)//所有玩家都已询问过;
    {
        if (lastone_)//给出分数最高的为地主;
        {
            curplayer_ = landlord_ = lastone_;
            lastone_ = nullptr;
        }
        else//若均为叫牌，重新开始游戏;
        {
            status_ = NOTSTART;
        }
    }

    if (landlord_)//地主已经确定，进入给地主发牌阶段;
    {
        SetPlayerRelation(landlord_->id_);
        status_ = SENDLANDLORDCARD;
    }
}

void card_covert::SetPlayerRelation(int landlord_seatid)
{
    for (int i = 0; i < 3; ++i)
    {
        int preId = i - 1;
        int nextid = i + 1;
        if (preId < 0)
        {
            preId += 3;
        }
        if (nextid > 2)
        {
            nextid -= 3;
        }
        player_[i]->SetLandlord(false);
        player_[i]->SetPlayer(player_[preId], player_[nextid]);
    }
    player_[landlord_seatid]->SetLandlord(true);
    landlord_ = player_[landlord_seatid];
}

void card_covert::SendLandlordCard(std::vector<POKER>& pokers)
{
    for (auto& item : pokers)
    {
        int val = covert(item);
        if (val > 0)
        {
            landlord_->AddCard(val);
        }
    }
    status_ = DISCARD;
}

void card_covert::PlayerDiscard(int seatid, const std::vector<POKER>& pokers)
{
    if (seatid < sizeof(player_))
    {
        auto ptr = player_[seatid];
        for (auto& item : pokers)
        {
            int num = covert(item);
            if (num > 0)
            {
                if (ptr->selection_.cards_.find(num) == ptr->selection_.cards_.end())
                {
                    ptr->selection_.AddNumber(num);
                }
            }
        }
    }
}

bool card_covert::Discard(int32_t& seatid, int32_t& next_cid, std::vector<POKER>& discard)
{
    if (status_ == GAMEOVER)
    {
        return false;
    }
    if (lastone_ == curplayer_)//该玩家出牌没人压死，新一轮出牌;
    {
        lastone_ = nullptr;
        for (int i = 0; i < 3; ++i)//清空出牌区;
        {
            player_[i]->discard_.Clear();
            player_[i]->nodiscard_ = false;
        }
    }
    else//清空当前出牌玩家出牌区;
    {
        curplayer_->discard_.Clear();
        curplayer_->nodiscard_ = false;
    }
    if (!curplayer_->IsRobot())
    {
        if (curplayer_->HumanDiscard(this->lastone_))//玩家已选牌并且符合规定;
        {
            lastone_ = curplayer_;
        }
        else//否则继续等待玩家选牌;
        {
            return false;
        }
    }
    else
    {
        curplayer_->SelectCards(lastone_);

        if (curplayer_->Discard())
        {
            lastone_ = curplayer_;
        }
        else//机器人没选牌;
        {
            int a = 0;
            ++a;
            //return false;
        }
    }

    if (curplayer_->discard_.type_ == Bomb)//如果出牌为炸弹，增加倍率;
        ++times_;

    //转换机器人的牌;
    seatid = curplayer_->id_;
    for (auto& val : curplayer_->discard_.cards_)
    {
        auto poker = covert(val);
        discard.push_back(poker);
    }

    if (lastone_->cards_.empty())//最后出牌方已无手牌
        status_ = GAMEOVER;//游戏结束
    else
        curplayer_ = curplayer_->nextplayer_;//下家继续出牌
    next_cid = curplayer_->id_;

    return true;
}

bool card_covert::IsPass(int32_t seatid)
{
    if (seatid< sizeof(player_))
    {
        return player_[seatid]->nodiscard_;
    }
    else
    {
        //std::throw_exception("seat id is invalid ");
    }
}

int32_t card_covert::Pass(int32_t seatid)
{
    if (seatid < sizeof(player_))
    {
        player_[seatid]->Pass();
        return player_[seatid]->nextplayer_->id_;
    }
    else
    {
    }
}

bool card_covert::GameOver(void)
{
    int score = basescore_ * times_;
    bool IsPeopleWin = false;

    if (landlord_->cards_.size()) {//农民胜利
        landlord_->score_ -= score * 2;
        landlord_->preplayer_->score_ += score;
        landlord_->nextplayer_->score_ += score;
        if (player_[0] != landlord_)
            IsPeopleWin = true;
    }
    else {//地主胜利
        landlord_->score_ += score * 2;
        landlord_->preplayer_->score_ -= score;
        landlord_->nextplayer_->score_ -= score;
        if (player_[0] == landlord_)
            IsPeopleWin = true;
    }
    status_ = NOTSTART;
    return IsPeopleWin;
}

void card_covert::Init(void)
{
    landlord_ = curplayer_ = lastone_ = nullptr;
    basescore_ = questioned_ = 0;
    times_ = 1;

    for (auto& item : player_)
    {
        item->NewGame();
    }
}

int card_covert::covert(const POKER& poker)
{
    int ret = -1;
    if (poker.value >= POKER_VALUE_3 && poker.value <= POKER_VALUE_K && poker.style < POKER_STYLE_EX)
    {
        ret = poker.value - 3;
    }
    else if (poker.value == POKER_VALUE_A && poker.style < POKER_STYLE_EX)
    {
        ret = 11;
    }
    else if (poker.value == POKER_VALUE_2 && poker.style < POKER_STYLE_EX)
    {
        ret = 12;
    }

    if (poker.style < POKER_STYLE_EX)
    {
        ret += poker.style * 13;
    }

    if (poker.value == POKER_VALUE_JOKER_SMALL && poker.style == POKER_STYLE_EX)
    {
        ret = 52;
    }
    else if (poker.value == POKER_VALUE_JOKER_LARGE && poker.style == POKER_STYLE_EX)
    {
        ret = 53;
    }

    return ret;
}

POKER card_covert::covert(int num)
{
    POKER poker;

    if (num < 52)
    {
        int value = num % 13 + 3;
        if (value == 14)
        {
            value = POKER_VALUE_A;
        }
        if (value == 15)
        {
            value = POKER_VALUE_2;
        }
        poker.style = num / 13;
        poker.value = value;
    }
    else  if (num == 52)
    {
        poker.style = POKER_STYLE_EX;
        poker.value = POKER_VALUE_JOKER_SMALL;
    }
    else  if (num == 53)
    {
        poker.style = POKER_STYLE_EX;
        poker.value = POKER_VALUE_JOKER_LARGE;
    }
    return poker;
}
