#pragma once

#include "card_def.h"

LANDLORD_SPACE_BEGIN

class logic_card
{
public:
	logic_card(void);
	virtual ~logic_card(void);

public:
	// ����
	bool CheckIsDanZhang(const VECPOKER &vecpoker);
	// ��
	bool CheckIsDuiZi(const VECPOKER &vecpoker);
	// ����
	bool CheckIsSanZhang(const VECPOKER &vecpoker);
	// ����һ
	bool CheckIsSanDiaYi(const VECPOKER &vecpoker);
	// ������
	bool CheckIsSanDiaEr(const VECPOKER &vecpoker);
	// ˳��
	bool CheckIsShunZi(const VECPOKER &vecpoker);
	// ����
	bool CheckIsLianDui(const VECPOKER &vecpoker);
	// ����������
	bool CheckIsLianShan(const VECPOKER &vecpoker);
	//�ɻ������
	bool CheckIsFeiDiaYi(const VECPOKER &vecpoker);
	bool CheckIsFeiDiaEr(const VECPOKER &vecpoker);
	//�Ĵ���
	bool CheckIsSiDaiER(const VECPOKER &vecpoker);
	bool CheckIsSiDaiERDUI(const VECPOKER &vecpoker);
	// ����ǲ���ը��
	bool CheckIsZhaDan(const VECPOKER &vecpoker);
	// ��ը
	bool CheckIsWangZhaDan(const VECPOKER &vecpoker);

public:
	// �õ��Ƶ�����
	char GetCardType(const VECPOKER &vecpoker);

public:
	// ��ȡ��������ֵ
	char GetAValueFromAConstTypePoke(char type,const VECPOKER &vecpoker);
	// �Ƚϴ�С
	char CompareCard(const VECPOKER &leftpoker ,const VECPOKER &rightpoker);
	// �Ƚϴ�С
	char CompareCard(const char ltype,const char lvalue,const char llen,
		const char rtype,const char rvalue,const char rlen);
	// ɾ����
	char RemoveCard(VECPOKER &vecpoker,VECPOKER&delpoker);

public:
	//
	char fixPokerValue(const POKER &poker);
	//
	char CountSameValuseCard(const VECPOKER &vecpoker,const char value);

};

LANDLORD_SPACE_END
