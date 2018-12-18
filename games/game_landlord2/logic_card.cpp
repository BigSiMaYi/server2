#include "stdafx.h"
#include "logic_card.h"
#include "logic_common.h"

LANDLORD_SPACE_USING

POKER g_pokers[GPOKERS]={
		{POKER_VALUE_A,POKER_STYLE_SPADES},{POKER_VALUE_2,POKER_STYLE_SPADES},{POKER_VALUE_3,POKER_STYLE_SPADES},{POKER_VALUE_4,POKER_STYLE_SPADES},
		{POKER_VALUE_5,POKER_STYLE_SPADES},{POKER_VALUE_6,POKER_STYLE_SPADES},{POKER_VALUE_7,POKER_STYLE_SPADES},{POKER_VALUE_8,POKER_STYLE_SPADES},
		{POKER_VALUE_9,POKER_STYLE_SPADES},{POKER_VALUE_10,POKER_STYLE_SPADES},{POKER_VALUE_J,POKER_STYLE_SPADES},{POKER_VALUE_Q,POKER_STYLE_SPADES},
		{POKER_VALUE_K,POKER_STYLE_SPADES},
		{POKER_VALUE_A,POKER_STYLE_HEARTS},{POKER_VALUE_2,POKER_STYLE_HEARTS},{POKER_VALUE_3,POKER_STYLE_HEARTS},{POKER_VALUE_4,POKER_STYLE_HEARTS},
		{POKER_VALUE_5,POKER_STYLE_HEARTS},{POKER_VALUE_6,POKER_STYLE_HEARTS},{POKER_VALUE_7,POKER_STYLE_HEARTS},{POKER_VALUE_8,POKER_STYLE_HEARTS},
		{POKER_VALUE_9,POKER_STYLE_HEARTS},{POKER_VALUE_10,POKER_STYLE_HEARTS},{POKER_VALUE_J,POKER_STYLE_HEARTS},{POKER_VALUE_Q,POKER_STYLE_HEARTS},
		{POKER_VALUE_K,POKER_STYLE_HEARTS},
		{POKER_VALUE_A,POKER_STYLE_CLUBS},{POKER_VALUE_2,POKER_STYLE_CLUBS},{POKER_VALUE_3,POKER_STYLE_CLUBS},{POKER_VALUE_4,POKER_STYLE_CLUBS},
		{POKER_VALUE_5,POKER_STYLE_CLUBS},{POKER_VALUE_6,POKER_STYLE_CLUBS},{POKER_VALUE_7,POKER_STYLE_CLUBS},{POKER_VALUE_8,POKER_STYLE_CLUBS},
		{POKER_VALUE_9,POKER_STYLE_CLUBS},{POKER_VALUE_10,POKER_STYLE_CLUBS},{POKER_VALUE_J,POKER_STYLE_CLUBS},{POKER_VALUE_Q,POKER_STYLE_CLUBS},
		{POKER_VALUE_K,POKER_STYLE_CLUBS},
		{POKER_VALUE_A,POKER_STYLE_DIAMONDS},{POKER_VALUE_2,POKER_STYLE_DIAMONDS},{POKER_VALUE_3,POKER_STYLE_DIAMONDS},{POKER_VALUE_4,POKER_STYLE_DIAMONDS},
		{POKER_VALUE_5,POKER_STYLE_DIAMONDS},{POKER_VALUE_6,POKER_STYLE_DIAMONDS},{POKER_VALUE_7,POKER_STYLE_DIAMONDS},{POKER_VALUE_8,POKER_STYLE_DIAMONDS},
		{POKER_VALUE_9,POKER_STYLE_DIAMONDS},{POKER_VALUE_10,POKER_STYLE_DIAMONDS},{POKER_VALUE_J,POKER_STYLE_DIAMONDS},{POKER_VALUE_Q,POKER_STYLE_DIAMONDS},
		{POKER_VALUE_K,POKER_STYLE_DIAMONDS},
		{POKER_VALUE_JOKER_SMALL,POKER_STYLE_EX},{POKER_VALUE_JOKER_LARGE,POKER_STYLE_EX}
};

logic_card::logic_card(void)
{
}


logic_card::~logic_card(void)
{
}


char landlord_space::logic_card::fixPokerValue(const POKER &poker)
{
	return _fixpv(poker);
}


char landlord_space::logic_card::CountSameValuseCard(const VECPOKER &vecpoker,const char value)
{
	char count = 0;
	for(auto poker : vecpoker)
	{
		if(fixPokerValue(poker) == value)
			count++;
	}
	return count;
}


char landlord_space::logic_card::GetCardType(const VECPOKER &vecpoker)
{
    VECPOKER& poker = (VECPOKER&)vecpoker;
    std::sort(poker.begin(), poker.end(), POKER_GREATER());
	uint32_t nLen = vecpoker.size();
	if(nLen<1 || nLen>MAXPOKER)
		return STYLE_NO;
	if(CheckIsWangZhaDan(vecpoker))
		return STYLE_BOMBX;
	else if(CheckIsZhaDan(vecpoker))
		return STYLE_BOMB;
	else if(CheckIsSiDaiERDUI(vecpoker))
		return STYLE_BOMB2;
	else if(CheckIsSiDaiER(vecpoker))
		return STYLE_BOMB11;
	else if(CheckIsFeiDiaEr(vecpoker))
		return STYLE_FEIJI2;
	else if(CheckIsFeiDiaYi(vecpoker))
		return STYLE_FEIJI1;
	else if(CheckIsLianShan(vecpoker))
		return STYLE_FEIJI;
	else if(CheckIsLianDui(vecpoker))
		return STYLE_LIANDUI;
	else if(CheckIsShunZi(vecpoker))
		return STYLE_SHUN;
	else if(CheckIsSanDiaEr(vecpoker))
		return STYLE_3TIAO2;
	else if(CheckIsSanDiaYi(vecpoker))
		return STYLE_3TIAO1;
	else if(CheckIsSanZhang(vecpoker))
		return STYLE_3TIAO;
	else if(CheckIsDuiZi(vecpoker))
		return STYLE_DUI;
	else if(CheckIsDanZhang(vecpoker))
		return STYLE_DAN;

	return STYLE_NO;
}


char landlord_space::logic_card::GetAValueFromAConstTypePoke(char type,const VECPOKER &vecpoker)
{
	uint32_t len = vecpoker.size();
	char val=0;
	switch(type)
	{
	case STYLE_DAN:
	case STYLE_DUI:
	case STYLE_3TIAO:
		val=fixPokerValue(vecpoker[0]);
		break;
	case STYLE_3TIAO1:
	case STYLE_3TIAO2:
		{
			val=fixPokerValue(vecpoker[0]);
			if (3!=CountSameValuseCard(vecpoker,val))
				val=fixPokerValue(vecpoker[len-1]);
		}
		break;
	case STYLE_SHUN:
		val=fixPokerValue(vecpoker[0]);
		break;
	case STYLE_LIANDUI:
		val=fixPokerValue(vecpoker[0]);
		break;
	case STYLE_FEIJI:
		val=fixPokerValue(vecpoker[0]);
		break;
	case STYLE_FEIJI1:
	case STYLE_FEIJI2:
		{
			for (uint32_t i=0; i<len; i++)
			{
				val=fixPokerValue(vecpoker[i]);
				char c=CountSameValuseCard(vecpoker,val);
				if (3==c) break;
			}
		}
		break;
	case STYLE_BOMB11:
	case STYLE_BOMB2:
		{
			for (uint32_t i=0; i<len; i++)
			{
				val=fixPokerValue(vecpoker[i]);
				char c=CountSameValuseCard(vecpoker,val);
				if (4==c) break;
			}
		}
		break;
	case STYLE_BOMB:
		val=fixPokerValue(vecpoker[0]);
		break;
	case STYLE_BOMBX:
		val=fixPokerValue(vecpoker[0]);
		break;
	default:
		break;

	}

	return val;

}

char landlord_space::logic_card::CompareCard(const VECPOKER &leftpoker ,const VECPOKER &rightpoker)
{
	char ltype = GetCardType(leftpoker);
	char lvalue = GetAValueFromAConstTypePoke(ltype,leftpoker);
	char llen = leftpoker.size();

	char rtype = GetCardType(rightpoker);
	char rvalue = GetAValueFromAConstTypePoke(rtype,rightpoker);
	char rlen = rightpoker.size();
	return CompareCard(ltype,lvalue,llen,rtype,rvalue,rlen);
}

char landlord_space::logic_card::CompareCard(const char ltype,const char lvalue,const char llen,
											 const char rtype,const char rvalue,const char rlen)
{
	if(rtype > ltype)
	{
		if(rtype >= STYLE_BOMB)
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}
	else if(rtype == ltype)
	{
		if(rtype>=STYLE_DAN && rtype<=STYLE_BOMB2)
		{
			if(rlen != llen)
			{
				return -1;
			}
			else if(rlen >= llen)
			{
				return 1;
			}
		}
		else
		{
			if(rtype==STYLE_BOMB)
			{
				if(rvalue > lvalue) return 1;
				return -1;
			}
			else if(rtype==STYLE_BOMBX)
			{
				return 1;
			}
		}
	}
	else
	{
		return -1;
	}
	return -1;
}


bool landlord_space::logic_card::CheckIsDanZhang(const VECPOKER &vecpoker)
{
	uint32_t nLen = vecpoker.size();
	return nLen==1;
}

bool landlord_space::logic_card::CheckIsDuiZi(const VECPOKER &vecpoker)
{
	uint32_t nLen = vecpoker.size();
	if(nLen!=2) return false;
	if(fixPokerValue(vecpoker[0]) != fixPokerValue(vecpoker[1]))
		return false;
	return true;
}

bool landlord_space::logic_card::CheckIsSanZhang(const VECPOKER &vecpoker)
{
	uint32_t nLen = vecpoker.size();
	if(nLen!=3) return false;
	char va = fixPokerValue(vecpoker[0]);
	char vb = fixPokerValue(vecpoker[1]);
	char vc = fixPokerValue(vecpoker[2]);
	if((va==vb) && (vb==vc)) return true;
	return false;
}

bool landlord_space::logic_card::CheckIsSanDiaYi(const VECPOKER &vecpoker)
{
	uint32_t nLen = vecpoker.size();
	if(nLen!=4) return false;
	char va = fixPokerValue(vecpoker[0]);
	char vb = fixPokerValue(vecpoker[1]);
	char vc = fixPokerValue(vecpoker[2]);
	char vd = fixPokerValue(vecpoker[3]);
	if( va==vb)
	{
		if((vb==vc) && (vc!=vd)) return true;
	}
	else
	{
		if((vb==vc) && (vc==vd)) return true;
	}
	return false;
}

bool landlord_space::logic_card::CheckIsSanDiaEr(const VECPOKER &vecpoker)
{
	uint32_t nLen = vecpoker.size();
	if (nLen!=5) return false;

	char a=fixPokerValue(vecpoker[0]);
	char b=fixPokerValue(vecpoker[1]);
	char c=fixPokerValue(vecpoker[2]);
	char d=fixPokerValue(vecpoker[3]);
	char e=fixPokerValue(vecpoker[4]);

	if (a==c)
	{
		if (b==c)
		{
			if ((c!=d)&&(d==e))
				return true;
		}
	}
	else
	{
		if (a==b)
		{
			if ((b!=c)&&(c==d)&&(d==e))
				return true;
		}
	}

	return false;
}

bool landlord_space::logic_card::CheckIsShunZi(const VECPOKER &vecpoker)
{
	uint32_t nLen = vecpoker.size();
	if (nLen<5) return false;

	//取第一个数
	char v0 = fixPokerValue(vecpoker[0]);
	if (v0 > PV_A) return false;
	//取最后一个数
	char v1=fixPokerValue(vecpoker[nLen-1]);

	//验证长度怎么样
	if (nLen!=(v0-v1+1))
		return false;

	//验证他们是不是相连的
	for (uint32_t i=1;i<nLen;i++)
	{
		v1=fixPokerValue(vecpoker[i]);
		if (v0!=(v1+1))
			return false;
		v0=v1;
	}

	return true;
}

bool landlord_space::logic_card::CheckIsLianDui(const VECPOKER &vecpoker)
{
	//三对或三对以上相连的牌，如：33 44 55，JJ QQ KK AA。注意：2 不可以在连队中.
	uint32_t nLen = vecpoker.size();
	if (nLen<6) return false;
	if ((nLen%2)!=0) return false;

	//取第一个数
	char v0=fixPokerValue(vecpoker[0]);
	if (v0>PV_A) return false;

	//取最后一个数
	char v1=fixPokerValue(vecpoker[nLen-1]);

	//验证长度怎么样
	if ((nLen/2)!=(v0-v1+1))
		return false;

	//验证他们是不是相连的
	uint32_t i=0;//+=2
	//char c=0;
	while(i<nLen){
		//比较两个相同的
		if (fixPokerValue(vecpoker[i])!=fixPokerValue(vecpoker[i+1]))
			return false;
		//与下一组相比
		if (i<(nLen-2)){
			if (fixPokerValue(vecpoker[i])!=fixPokerValue(vecpoker[i+2])+1)
				return false;
		}
		////这是没有下一组了
		else
		{
			break;
		}
		i+=2;
	}

	return true;
}

bool landlord_space::logic_card::CheckIsLianShan(const VECPOKER &vecpoker)
{
	uint32_t nLen = vecpoker.size();
	if (nLen<6) return false;

	if ((nLen%3)!=0) return false;

	//取第一个数
	char v0=fixPokerValue(vecpoker[0]);
	if (v0>PV_A) return false;

	//取最后一个数
	char v1=fixPokerValue(vecpoker[nLen-1]);
	//验证长度怎么样
	if ((nLen/3)!=(v0-v1+1))
		return false;

	//验证他们是不是相连的
	uint32_t i=0;
	//char c=0;
	while(i<nLen)
	{
		//比较三个相同的
		if (fixPokerValue(vecpoker[i])!=fixPokerValue(vecpoker[i+1])
			||fixPokerValue(vecpoker[i])!=fixPokerValue(vecpoker[i+2]))
			return false;
		//与下一组相比
		if (i<(nLen-3))
		{
			if (fixPokerValue(vecpoker[i])!=fixPokerValue(vecpoker[i+3])+1)
				return false;
		}
		////这是没有下一组了
		else
		{
			break;
		}
		i+=3;
	}

	return true;
}

bool landlord_space::logic_card::CheckIsFeiDiaYi(const VECPOKER &vecpoker)
{
	uint32_t nLen = vecpoker.size();
	if(nLen<8) return false;
	if((nLen%4) != 0) return false;
	// 三顺的长度
	char len3 = nLen/4;

	char val[MAXPOKER] = {0};
	char a=0,b=0,c=0,d=0;
	char vidx = -1;
	for(uint32_t i=0;i<nLen;i++)
	{
		a = fixPokerValue(vecpoker[i]);
		if(vidx>-1)
		{
			b = val[vidx];
			if(a==b)
			{
				continue;
			}
			else
			{
				c = CountSameValuseCard(vecpoker,a);
				if(3==c)
				{
					vidx++;
					val[vidx] = a;
				}
				else if(c>3)
				{
					return false;
				}
			}
		}
		else
		{
			// 查找第一个三
			c = CountSameValuseCard(vecpoker,a);
			if(3==c)
			{
				if(a>PV_A) return false;

				vidx++;
				val[vidx] = a;
			}
			else if(c>3)
			{
				return false;
			}
		}
	}

	if(vidx+1 != len3)
	{
		return false;
	}
	a = val[0];
	for (char i=1;i<len3;i++)
	{
		d = val[i];
		if(a!=d+1) return false;

		a = d;
	}

	return true;
}

bool landlord_space::logic_card::CheckIsFeiDiaEr(const VECPOKER &vecpoker)
{
	uint32_t nLen = vecpoker.size();
	if(nLen<10) return false;
	if((nLen%5) != 0) return false;
	// 三顺的长度
	char len3 = nLen/5;

	char val[MAXPOKER] = {0};
	char a=0,b=0,c=0,d=0;
	char vidx = -1;
	for(uint32_t i=0;i<nLen;i++)
	{
		a = fixPokerValue(vecpoker[i]);
		if(vidx>-1)
		{
			b = val[vidx];
			if(a==b)
			{
				continue;
			}
			else
			{
				c = CountSameValuseCard(vecpoker,a);
				if(3==c)
				{
					vidx++;
					val[vidx] = a;
				}
			}
		}
		else
		{
			// 查找第一个三
			c = CountSameValuseCard(vecpoker,a);
			if(3==c)
			{
				if(a>PV_A) return false;

				vidx++;
				val[vidx] = a;
			}
		}
	}

	if(vidx+1 != len3) return false;

	a = val[0];
	for (char i=1;i<len3;i++)
	{
		d = val[i];
		if(a!=d+1) return false;

		a = d;
	}
	// 检测对子
	vidx = -1;
	memset(val,0,sizeof(val));

	for (uint32_t i = 0; i < nLen; i++)
	{
		a = fixPokerValue(vecpoker[i]);
		if(vidx>-1)
		{
			b = val[vidx];
			if(a==b)
			{
				continue;
			}
			else
			{
				c = CountSameValuseCard(vecpoker,a);
				if(c==2)
				{
					vidx++;
					val[vidx] = a;
				}
				else if(c==4)
				{
					vidx += 2;
					val[vidx] = a;
				}
			}
		}
		else
		{
			c = CountSameValuseCard(vecpoker,a);
			if(c==2)
			{
				vidx++;
				val[vidx] = a;
			}
			else if(c==4)
			{
				vidx += 2;
				val[vidx] = a;
			}
		}
	}

	if(vidx+1 != len3) return false;

	return true;
}

bool landlord_space::logic_card::CheckIsSiDaiER(const VECPOKER &vecpoker)
{
	uint32_t nLen = vecpoker.size();
	if (nLen<6) return false;

	char val = fixPokerValue(vecpoker[3]);
	char c = CountSameValuseCard(vecpoker,val);
	if (c!=4) return false;
	return true;
}

bool landlord_space::logic_card::CheckIsSiDaiERDUI(const VECPOKER &vecpoker)
{
	uint32_t len = vecpoker.size();
	if (len!=8) return false;

	//先找到第一个再找到最后一个，从最后一个再找到中间的那个
	// 0 1 2 3 4 5 6 7
	char v=fixPokerValue(vecpoker[0]);
	char c=CountSameValuseCard(vecpoker,v);

	if (c==4)
	{
		v=fixPokerValue(vecpoker[4]);
		c=CountSameValuseCard(vecpoker,v);
		if (c!=2)
			return false;

		v=fixPokerValue(vecpoker[6]);
		c=CountSameValuseCard(vecpoker,v);
		if (c!=2)
			return false;
	}
	else if (c==2)
	{
		v=fixPokerValue(vecpoker[2]);
		c=CountSameValuseCard(vecpoker,v);
		if (c==4)
		{
			v=fixPokerValue(vecpoker[6]);
			c=CountSameValuseCard(vecpoker,v);
			if (c!=2)
				return false;
		}
		else if (c==2)
		{
			v=fixPokerValue(vecpoker[4]);
			c=CountSameValuseCard(vecpoker,v);
			if (c!=4)
				return false;
		}
		else
			return false;
	}
	else
		return false;

	return true;
}

bool landlord_space::logic_card::CheckIsZhaDan(const VECPOKER &vecpoker)
{
	uint32_t nLen = vecpoker.size();
	if(nLen!=4) return false;

	char val = fixPokerValue(vecpoker[0]);
	for (auto poker : vecpoker)
	{
		if(fixPokerValue(poker) != val)
			return false;
	}
	return true;
}

bool landlord_space::logic_card::CheckIsWangZhaDan(const VECPOKER &vecpoker)
{
	uint32_t nLen = vecpoker.size();
	if(nLen!=2) return false;

	return (fixPokerValue(vecpoker[0])==PV_L && fixPokerValue(vecpoker[1])==PV_S);
}

char landlord_space::logic_card::RemoveCard(VECPOKER &vecpoker,VECPOKER&delpoker)
{
	char cbCount = 0;
	uint32_t nAllLen = vecpoker.size();
	uint32_t nDelLen = delpoker.size();
	if(nAllLen==0 || nDelLen==0) return cbCount;
	for (auto del : delpoker)
	{
		auto itret = std::find(std::begin(vecpoker),std::end(vecpoker),del);
		if(itret!=vecpoker.end())
		{
			vecpoker.erase(itret);
			cbCount++;
		}
	}
	return cbCount;
}


