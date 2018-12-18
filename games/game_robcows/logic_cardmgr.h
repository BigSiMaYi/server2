#pragma once

#include "card_def.h"
#include "GameLogic.h"

ROBCOWS_SPACE_BEGIN

class logic_cardmgr
{
public:
	logic_cardmgr(void);
	virtual ~logic_cardmgr(void);

public:
	// ����
	void Reposition();
	// ���ñ�ǩ
	void SetUserLabel(uint32_t uid,int32_t nLabel);
	// ����λ��
	void SetUserPos(  uint32_t uid, int32_t pos);
	// ϴ��
	void Wash_Card();
	// 
	void Wash_Card2();
	void Wash_Card3();
	void Wash_CardNomal();
	void Wash_CardControl();
	// ������
	void PreAllocation();
	void PreAllocationEx();
	// ����
	bool GetUserCard(GameCard &card);
	bool GetUserCard(uint32_t playerid,GameCard &card);
	uint32_t GetCardRate(GameCard &card) const;
	// ����
	bool SwitchCard(uint32_t playerid,GameCard &card/*,bool bLarge=false*/);
	// ����
	uint32_t CompareCard(uint32_t reqid,uint32_t resid);
	// ����
	uint32_t CompareCard(std::vector<uint32_t> vecid);

protected:
	bool WashCard_ModelA();
	bool WashCard_ModelB();
	bool WashCard_ModelC();
	bool WashCard_ModelD();

private:
	CGameLogic		m_GameLogic;

	Vector_Card		m_vecCardData;
	Vec_GameCard	m_GameCardData;
	Vec_GameCard	m_SaveGameCardData;

	Vec_GameCard	m_Card2Supper;	// �����û��ƿ�
	Vec_GameCard	m_Card2RobotL;	// �������ƿ�
	Vec_GameCard	m_Card2RobotS;	// �������ƿ�
	Vec_GameCard	m_Card2UserA;	// ����ƿ�
	Vec_GameCard	m_Card2UserB;	// ����ƿ�
	Vec_GameCard	m_Card2UserC;	// ����ƿ�
private:
	Vec_UserID		m_VecSupperID;	// 
	Vec_UserID		m_VecRobotID;	// 
	Vec_UserID		m_VecUserID_A;
	Vec_UserID		m_VecUserID_B;
	Vec_UserID		m_VecUserID_C;

	Map_PlayerLabel	m_MapUserPos;

	Map_PlayerLabel	m_MapPlayerID;	// 

	Map_PlayerCard	m_MapGameCard;

	int32_t			m_nBasePair;
};

ROBCOWS_SPACE_END
