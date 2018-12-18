#include "stdafx.h"
#include "card_covert.h"
#include "server_log.h"
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

int card_covert::CalcCardsWight(std::vector<POKER>& pokers)
{
    std::set<int> cards;
    for (auto& item : pokers)
    {
        int num = covert(item);
        if (num >= 0)
        {
            cards.insert(num);
        }
    }
    return card_analyse::CalcCardsWight(cards);
}

void card_covert::SendCard(int32_t seatid, std::vector<POKER>& pokers, bool is_robot)
{
    auto analyser = player_[seatid];
    for (auto& item : pokers)
    {
        int val = covert(item);
        if (val >= 0)
        {
            analyser->AddCard(val);
        }
    }
    analyser->SetAutoDiscard(is_robot);
}

void card_covert::SendCard2(int32_t seatid, std::vector<POKER>& pokers, bool is_robot)
{
    std::set<int> cards;
    auto analyser = player_[seatid];
    for (auto& item : pokers)
    {
        int val = covert(item);
        if (val >= 0)
        {
            cards.insert(val);
        }
    }
    analyser->AddCards(cards);
    analyser->SetAutoDiscard(is_robot);
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
        else if(landlord_ == nullptr)//若均为叫牌，重新开始游戏;
        {
            basescore_ = 1;
            landlord_ = player_[callbegin_];
            curplayer_ = landlord_;
            lastone_ = nullptr;
            //status_ = NOTSTART;
        }
    }

    if (landlord_)//地主已经确定，进入给地主发牌阶段;
    {
        SetPlayerRelation(landlord_->id_);
        status_ = SENDLANDLORDCARD;
        landlord_->SetLandlord(true);
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
}

bool card_covert::CheckSeat(int seatid)
{
	auto seat_size = sizeof(player_)/sizeof(player_[0]);
    if (seatid < seat_size && seatid >= 0)
    {
        auto player = player_[seatid];
        if (player == curplayer_)
        {
            return true;
        }
    }
    return false;
}

void card_covert::SendLandlordCard(int32_t seatid, std::vector<POKER>& pokers)
{
	if (landlord_ == nullptr || (landlord_ != nullptr && landlord_->id_ != seatid))
	{
		landlord_ = player_[seatid];
		SetPlayerRelation(landlord_->id_);
		landlord_->SetLandlord(true);
		curplayer_ = landlord_;
		lastone_ = nullptr;
	}
    for (auto& item : pokers)
    {
        int val = covert(item);
        if (val >= 0)
        {
            landlord_->AddCard(val);
        }
    }
    status_ = DISCARD;
}

bool card_covert::PlayerDiscard(int seatid, const std::vector<POKER>& pokers)
{
    if (CheckSeat(seatid))
    {
        std::set<int> cards;
        for (auto& item : pokers)
        {
            int num = covert(item);
            if (num >= 0)
            {
                cards.insert(num);
            }
        }
        
        curplayer_->AddDiscards(cards);
        auto lastplayer = (curplayer_ == lastone_ ? nullptr : lastone_);
        if (curplayer_->IsValid(lastplayer))//玩家已选牌并且符合规定;
        {
            int32_t seatid;
            int32_t next_cid;
            std::vector<POKER> discard;
            Discard(seatid, next_cid, discard, false);
            //GetDiscard();
            return true;
        }
        else
        {//记录出错日志
		    std::string str;
            if (lastplayer)
            {
                std::vector<POKER> lastcards;
                for (auto& c : lastplayer->discard_.cards_)
                {
                    lastcards.push_back(this->covert(c));
                }
                str = poker_to_string(lastcards);
            }
			else
			{
			    str = "last player is null";
			}
			SLOG_CRITICAL << "---------- seatid: " << seatid << ", discards: "<< str;
        }
        curplayer_->selection_.Clear();
    }
    else
    {
		SLOG_CRITICAL << "---------- seatid: " << seatid << ", error ";
    }
    
    return false;
}

bool card_covert::Discard(int32_t& seatid, int32_t& next_cid, std::vector<POKER>& discard, bool is_auto)
{
    if (status_ == GAMEOVER)
    {
		SLOG_CRITICAL << "---------- " << "game_over ";
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

    if (/*curplayer_->AutoDiscard() || */is_auto)
    {
        curplayer_->SelectCards(lastone_);
    }

    if (curplayer_->Discard())
    {
        lastone_ = curplayer_;
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

    if (lastone_->cards_.empty())//最后出牌方已无手牌;
        status_ = GAMEOVER;//游戏结束;
    else
        curplayer_ = curplayer_->nextplayer_;//下家继续出牌;
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
		SLOG_CRITICAL << "---------- seatid: " << seatid << ", error ";
        //std::throw_exception("seat id is invalid ");
    }
}

int32_t card_covert::Pass(int32_t seatid)
{
    if (CheckSeat(seatid))
    {
        curplayer_->Pass();
        curplayer_ = curplayer_->nextplayer_;
        return curplayer_->id_;
    }

    return -1;
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

void card_covert::SetRobot(int seatid, bool flag)
{
    if (seatid < sizeof(player_))
    {
        player_[seatid]->SetAutoDiscard(flag);
    }
}

void card_covert::SetCurrentPlayer(int seatid)
{
	auto cur = player_[seatid];
	if (cur != curplayer_)
	{
		curplayer_ = cur;
	}
}

void card_covert::SetLastPlayer(int seatid)
{
	auto last = player_[seatid];
	if (last != lastone_)
	{
		lastone_ = last;
	}
}

void card_covert::GetCardGroupBomb(std::vector<POKER>& pokers, int32_t& bombsize, int32_t& groupsize)
{
    groupsize = 0;
    bombsize = 0;
    std::set<int> cards;
    for (auto& item : pokers)
    {
        int val = covert(item);
        cards.insert(val);
    }

    std::map<int, int> needanalyse;//方便分析的权值-数量集合;
    for (auto& mem : cards)
    {
        ++needanalyse[CardGroup::Translate(mem)];//根据手牌构造待分析集合;
    }

    if (needanalyse.find(16) != needanalyse.end()
        && needanalyse.find(17) != needanalyse.end())//存在王炸;
    {
        ++bombsize;
    }
    for (auto& mem : needanalyse)
    {
        if (mem.second == 4)//炸弹;
        {
            ++bombsize;
        }
    }

    card_analyse analyser(0);
    analyser.AddCards(cards);
    analyser.DivideIntoGroups(); //这里调用出现异常: ;

    groupsize= analyser.analyse_.size();
}

bool card_covert::CheckPokerValue(std::vector<POKER>& pokers)
{
    std::set<int> cards;
    for (auto& item : pokers)
    {
        int val = covert(item);
        cards.insert(val);
    }
    if (cards.size() == pokers.size())
    {
        return true;
    }
    return false;
}

void GetCards(std::set<int>& cards, std::map<int, int>& needanalyse, int num, std::set<int>& selectcards)
{
    auto itr = needanalyse.find(num);
    if (itr != needanalyse.end())
    {
        auto val = itr->first;
        for (int i = 0; i < itr->second; ++i)
        {
            int card = card_analyse::ValueToNum(cards, val);
            selectcards.insert(card);
        }
        needanalyse.erase(itr);
    }
}

void card_covert::MixRobotCards(std::vector<POKER>& pokers1, std::vector<POKER>& pokers2)
{
    std::set<int> cards1;
    std::map<int, int> needanalyse;

    MakeDivide(pokers1, cards1, needanalyse);
    std::set<int> cards2;
    MakeDivide(pokers2, cards2, needanalyse);
    std::set<int> cards = cards1;
    for (auto& i : cards2)
    {
        cards.insert(i);
    }

	int bomb_counter = 0;
	std::set<int> selectcards;
	//炸弹：去掉王炸;
#if 0
	if (needanalyse.find(16) != needanalyse.end()
		&& needanalyse.find(17) != needanalyse.end())
	{
		bomb_counter++;
	}
#endif
	auto itr = needanalyse.find(15);
	if (itr != needanalyse.end())
	{
		if (itr->second == 4)
		{
			bomb_counter++;
		}
	}

    GetCards(cards, needanalyse, 15, selectcards);
    GetCards(cards, needanalyse, 16, selectcards);
    GetCards(cards, needanalyse, 17, selectcards);
#if 1
	if (bomb_counter == 0)
	{
		for (auto& mem : needanalyse)
		{
			if (mem.second == 4)//炸弹;
			{
				if (++bomb_counter > 1)
				{
					break;
				}
				for (int i = 0; i < 4; ++i)
				{
					int card = card_analyse::ValueToNum(cards, mem.first);
					selectcards.insert(card);
				}
				needanalyse[mem.first] = 0;
			}
		}
	}
#endif

    if (selectcards.size() == 0)
    {
        return;
    }

   int rem = pokers1.size() - selectcards.size();
   
   std::vector<int> rem_cards;
   std::set<int> replenish;
    int size = 10;
    do 
    {
        rem_cards.clear();
        replenish.clear();
        for (auto& c : cards)
        {
            rem_cards.push_back(c);
        }

        std::random_device rd;
        std::mt19937_64 g(rd());
        std::shuffle(rem_cards.begin(), rem_cards.end(), g);
        
        int index = 0;
        
        for (int i = 0; i < rem; ++i)
        {
            int c = rem_cards.back();
            rem_cards.pop_back();
            replenish.insert(c);
        }

        card_analyse analyser(0);
        analyser.AddCards(replenish);
        analyser.DivideIntoGroups();
        if (analyser.analyse_.size() < 6)
        {
		    for (auto& item : analyser.analyse_)
			{
				if (item->type_ == Bomb)
				{
					continue;
				}
			}
            break;
        }
    } while (--size);


    pokers1.clear();
    for (auto& item : rem_cards)
    {
        auto poke = covert(item);
        pokers1.push_back(poke);
    }
    pokers2.clear();
    for (auto& item : selectcards)
    {
        auto poke = covert(item);
        pokers2.push_back(poke);
    }
    for (auto& item : replenish)
    {
        auto poke = covert(item);
        pokers2.push_back(poke);
    }
    
// 
//     if (rem > 10)
//     {
//         for (auto& item : analyser.analyse_)
//         {
//             if (item->type_ ==SingleSeq
//                 || item->type_ == DoubleSeq
//                 || item->type_ == ThreeSeq)
//             {
//                 if (rem > item->count_ && rem > 10)
//                 {
//                     for (auto& c : item->cards_)
//                     {
//                         selectcards.insert(c);
//                         --rem;
//                     }
//                 }
//             }
//         }
//     }
//     else if (rem > 5)
//     {
//         for (auto& item : analyser.analyse_)
//         {
//             if (item->type_ == Double
//                 || item->type_ == Three
//                 || item->type_ == ThreeSeq)
//             {
//                 if (rem > item->count_ && rem > 5)
//                 {
//                     for (auto& c : item->cards_)
//                     {
//                         selectcards.insert(c);
//                         --rem;
//                     }
//                 }
//             }
//         }
//     }
//     else
//     {
//         for (auto& item : analyser.analyse_)
//         {
//             if (item->type_ == Single)
//             {
//                 if (rem > item->count_ && rem > 0)
//                 {
//                     for (auto& c : item->cards_)
//                     {
//                         selectcards.insert(c);
//                         --rem;
//                     }
//                 }
//             }
//         }
//     }
//     
//     int ret_size = cards.size() - selectcards.size();
// 
//     std::vector<int> ret(ret_size);
//     std::set_difference(cards.begin(), cards.end(), selectcards.begin(), selectcards.end(), ret.begin());
// 
//     std::vector<POKER> pokers1_new;
//     std::vector<POKER> pokers2_new;
//     for (auto& item : ret)
//     {
//         auto poke = covert(item);
//         pokers1_new.push_back(poke);
//     }
// 
//     for (auto& item : selectcards)
//     {
//         auto poke = covert(item);
//         pokers2_new.push_back(poke);
//     }
}

void card_covert::MakeDivide(std::vector<POKER>& pokers1, std::set<int>& cards, std::map<int, int>& needanalyse)
{
    for (auto& item : pokers1)
    {
        int val = covert(item);
        cards.insert(val);
    }

    for (auto& mem : cards)
    {
        ++needanalyse[CardGroup::Translate(mem)];//根据手牌构造待分析集合;
    }

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
        ret = (poker.value-3)*4 + poker.style;
    }
    else if (poker.value == POKER_VALUE_A && poker.style < POKER_STYLE_EX)
    {
        ret = (14 - 3) * 4 + poker.style;
    }
    else if (poker.value == POKER_VALUE_2 && poker.style < POKER_STYLE_EX)
    {
        ret = (15 - 3) * 4 + poker.style;
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
        int value = num / 4 + 3;
        int style = num % 4;
        if (value == 14)
        {
            value = POKER_VALUE_A;
        }
        if (value == 15)
        {
            value = POKER_VALUE_2;
        }
        poker.style = style;
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

void poker_to_stringstream(const POKER& p, std::stringstream& sm)
{
    if (p.value == POKER_VALUE_J)
    {
        sm << "J ";
    }
    else if (p.value == POKER_VALUE_Q)
    {
        sm << "Q ";
    }
    else if (p.value == POKER_VALUE_K)
    {
        sm << "K ";
    }
    else if (p.value == POKER_VALUE_A && p.style < POKER_STYLE_EX)
    {
        sm << "A ";
    }
    else if (p.value == POKER_VALUE_JOKER_SMALL && p.style == POKER_STYLE_EX)
    {
        sm << "S_Joker ";
    }
    else if (p.value == POKER_VALUE_JOKER_LARGE && p.style == POKER_STYLE_EX)
    {
        sm << "L_Joker ";
    }
    else
    {
        sm << (int)p.value << " ";
    }
}

std::string poker_to_string(const std::vector<POKER>& p)
{
    std::stringstream sm;
    for (auto& item : p)
    {
        poker_to_stringstream(item, sm);
    }
    return sm.str();
}