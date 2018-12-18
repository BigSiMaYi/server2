#pragma once
#include "card_def.h"

#define PV_A        (POKER_VALUE_K+1)
#define PV_2        (PV_A+1)
//这个是小王
#define PV_S		(PV_2+1)
//这个大王
#define PV_L        (PV_S+1)

#ifdef DEBUG
    #define TIMEREADY       15
    #define TIMEJIAO           10
    #define TIMEOUTPAI      20
    #define TIMEOUTPAI2    25
#else
    #define TIMEREADY       20
    #define TIMEJIAO           10
    #define TIMEOUTPAI      20
    #define TIMEOUTPAI2    25
#endif

#define TIMEDIFF    3


//这个是得到扑克的大小 在这里没有对花色进行考虑
//这个可能比上面的那个更好理解
static char _fixpv(char value,char style)
{
    if (POKER_STYLE_EX==style)
    {
        if(value==POKER_VALUE_JOKER_SMALL)//16
        {
            return PV_S;
        }
        else if(value==POKER_VALUE_JOKER_LARGE)
        {
            return PV_L;
        }
    }
    if(value==POKER_VALUE_A)
    {
        return PV_A;
    }
    else if(value==POKER_VALUE_2)
    {
        return PV_2;
    }
    return value;
}

//dont change poker
inline char _fixpv(POKER p){
    return _fixpv(p.value,p.style);
}

inline void swappoker(POKER& a,POKER& b){
	POKER c=a;a=b;b=c;
}

//big to small//usually to large compnent
//A is large
static void sortpoker(POKER*p,char n){
	for(char i=0;i<n-1;i++){
		for(char j=i+1;j<n;j++){
			if(_fixpv(p[i])<_fixpv(p[j]))
			{
			    swappoker(p[i],p[j]);
			}
            else if( _fixpv(p[i]) ==_fixpv(p[j]) )
            {
                if(p[i].style < p[j].style) swappoker(p[i],p[j]);
            }
		}
	}
}

static char CountSameValuseCard(char v,POKER* p,char len)
{
    char c=0;
    for (char i=0; i<len; i++)
    {
        if (_fixpv(p[i]) == v)
            c++;
    }
    return c;
}

//这个的方法是从一个给定类型的扑克中得到他的特定的值，这个值可以确定他的大小
static char GetAValueFromAConstTypePoke(char type,POKER* p,char len)
{
    char v=0;
    //只要找到一个能标记出他的大小的值就可以了
    char c=0;
    switch(type)
    {
        //单 单个牌（如红桃 5 ）。
    case STYLE_DAN:
        v=_fixpv(p[0]);
        break;
        //对 数值相同的两张牌（如梅花 4+ 方块 4 ）。
    case STYLE_DUI:
        v=_fixpv(p[0]);
        break;
        //3张 数值相同的三张牌（如三个 J ）。
    case STYLE_3TIAO:
        v=_fixpv(p[0]);
        break;
        //3张带一 数值相同的三张牌 + 一张单牌或一对牌。例如： 333+6 或 444+99
    case STYLE_3TIAO1:
        {
            v=_fixpv(p[0]);
            if (3!=CountSameValuseCard(v,p,len))
                v=_fixpv(p[len-1]);
        }
        break;
        //3张带二 数值相同的三张牌 + 一张单牌或一对牌。例如： 333+6 或 444+99
    case STYLE_3TIAO2:
        {
            v=_fixpv(p[0]);
            if (3!=CountSameValuseCard(v,p,len))
                v=_fixpv(p[len-1]);
        }
        break;
        //顺子 五张或更多的连续单牌（如： 45678 或 78910JQK ）。不包括 2 点和双王
    case STYLE_SHUN:
        v=_fixpv(p[0]);
        break;
        //连对 三对或更多的连续对牌（如： 334455 、 7788991010JJ ）。不包括 2 点和双王
    case STYLE_LIANDUI:
        v=_fixpv(p[0]);
        break;
        //三顺：二个或更多的连续三张牌（如： 333444 、 555666777888 ）。不包括 2 点和双王。
    case STYLE_FEIJI:
        v=_fixpv(p[0]);
        break;
        //飞机带一 三顺＋同数量的单牌（或同数量的对牌）。
        //如： 444555+79 或 333444555+7799JJ
    case STYLE_FEIJI1:
        {
            for (char i=0; i<len; i++){
                v=_fixpv(p[i]);
                c=CountSameValuseCard(v,p,len);
                if (3==c)
                    break;
            }
        }
        break;
        //飞机带二
    case STYLE_FEIJI2:
        {
            for (char i=0; i<len; i++){
                v=_fixpv(p[i]);
                c=CountSameValuseCard(v,p,len);
                if (3==c)
                    break;
            }
        }
        break;
        //四带二：四张牌＋两手牌。（注意：四带二不是炸弹 两个单牌
    case STYLE_BOMB11:
        {
            for (char i=0; i<len; i++){
                v=_fixpv(p[i]);
                c=CountSameValuseCard(v,p,len);
                if (4==c)
                    break;
            }
        }
        break;
        //四带二：四张牌＋两手牌。（注意：四带二不是炸弹 两个对子
    case STYLE_BOMB2:
        {
            for (int32_t i=0; i<len; i++){
                v=_fixpv(p[i]);
                c=CountSameValuseCard(v,p,len);
                if (4==c)
                    break;
            }
        }
        break;
        //炸弹 四张同数值牌（如四个 7 ）。
    case STYLE_BOMB:
        v=_fixpv(p[0]);
        break;
        //王炸弹  即双王（大王和小王），最大的牌
    case STYLE_BOMBX:
        v=_fixpv(p[0]);
        break;
    default:
        break;

    }

    return v;
}

//t_poke 比 t_Comparepoke大返回 1 相等返回 0 小返回-1
//int32_t GameCompareCardType(int32_t type, int32_t flage, int32_t lengtn,int32_t Comparetype, int32_t Comparetype_flage, int32_t Comparelengtn)
static char GameCompareCardType(char style, char val, char len,char newstyle,char newval,char newlen)
{
    if (newstyle>style)
    {
        //bomb is large than all other style
        if (newstyle>=STYLE_BOMB)
        {
            return 1;
        }
        else//must be same style
        {
            return -1;
        }
    }
    else if (newstyle == style)
    {
        //看看是不是炸弹
        //不是炸弹的要单独处理
        if ((newstyle>=STYLE_DAN) && (newstyle<=STYLE_BOMB2))
        {
            if (newlen != len)
            {
                return -1;
            }
            if (newval>val)
            {
                return 1;
            }
            return -1;
        }
        //是炸弹
        else
        {
            switch(newstyle)
            {
            case STYLE_BOMB://这个好办看看他们的大小就可以了
                {
                    if (newval>val)
                        return 1;
                    return -1;
                }
                //这个可是最大的了
            case STYLE_BOMBX:
                return 1;
            default:
                break;
            }
        }
    }

    //newstyle < style

    return -1;
}

static char removeCardFromHand(POKER hand[],uint32_t uHand,POKER out[],uint32_t uOut)
{
    uint32_t uRemove = 0;
    if(hand==nullptr || out==nullptr) return 0;
    POKER tmp[MAXPOKER];
	memset(tmp,0,sizeof(tmp));
    if(uHand>MAXPOKER || uOut>MAXPOKER) return 0;
    memcpy(tmp,hand,sizeof(POKER)*uHand);

    for(uint32_t i=0;i<uOut;i++)
    {
        for(uint32_t j=0;j<uHand;j++)
        {
            if(out[i] == tmp[j])
            {
				memset(&tmp[j],0,sizeof(POKER));
                uRemove++;
            }
        }
    }
    //
    assert(uOut!=uRemove);
    int nPos = 0;
    for(uint32_t i=0;i<uHand;i++)
    {
        if(tmp[i].style==0 && tmp[i].value==0)
        {
            hand[nPos++] = tmp[i];
        }
    }
    return  (uHand - uRemove);
}

