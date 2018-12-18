#pragma once

#include "card_def.h"

LANDLORD_SPACE_BEGIN

class logic_card
{
public:
	logic_card(void);
	virtual ~logic_card(void);

public:
	// 单张
	bool CheckIsDanZhang(const VECPOKER &vecpoker);
	// 对
	bool CheckIsDuiZi(const VECPOKER &vecpoker);
	// 三张
	bool CheckIsSanZhang(const VECPOKER &vecpoker);
	// 三带一
	bool CheckIsSanDiaYi(const VECPOKER &vecpoker);
	// 三带二
	bool CheckIsSanDiaEr(const VECPOKER &vecpoker);
	// 顺子
	bool CheckIsShunZi(const VECPOKER &vecpoker);
	// 连对
	bool CheckIsLianDui(const VECPOKER &vecpoker);
	// 连续三张牌
	bool CheckIsLianShan(const VECPOKER &vecpoker);
	//飞机带翅膀
	bool CheckIsFeiDiaYi(const VECPOKER &vecpoker);
	bool CheckIsFeiDiaEr(const VECPOKER &vecpoker);
	//四带二
	bool CheckIsSiDaiER(const VECPOKER &vecpoker);
	bool CheckIsSiDaiERDUI(const VECPOKER &vecpoker);
	// 检查是不是炸弹
	bool CheckIsZhaDan(const VECPOKER &vecpoker);
	// 王炸
	bool CheckIsWangZhaDan(const VECPOKER &vecpoker);

public:
	// 得到牌的类型
	char GetCardType(const VECPOKER &vecpoker);

public:
	// 获取牌型最大的值
	char GetAValueFromAConstTypePoke(char type,const VECPOKER &vecpoker);
	// 比较大小
	char CompareCard(const VECPOKER &leftpoker ,const VECPOKER &rightpoker);
	// 比较大小
	char CompareCard(const char ltype,const char lvalue,const char llen,
		const char rtype,const char rvalue,const char rlen);
	// 删除牌
	char RemoveCard(VECPOKER &vecpoker,VECPOKER&delpoker);

public:
	//
	char fixPokerValue(const POKER &poker);
	//
	char CountSameValuseCard(const VECPOKER &vecpoker,const char value);

};

LANDLORD_SPACE_END
