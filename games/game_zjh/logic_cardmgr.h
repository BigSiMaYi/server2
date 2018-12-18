#pragma once

#include "card_def.h"
#include "GameLogic.h"

ZJH_SPACE_BEGIN
enum eCardlvl
{
	eCardLvL_Small,
	eCardlvl_Normal,
	eCardLvL_Large,
	eCardLvL_Random,
};
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
	void SetUserPos(uint32_t uid, int32_t pos);
	// ����ϴ�Ƹ���
	void SetRate(std::vector<int>& nCardRate);
	// ϴ��
	void Wash_Card();
	// ϴ��
	void Wash_CardEx();
	//
	void Wash_CardEx2();
	//
	void Wash_CardNomal();
	void Wash_CardControl();
	// ������
	void PreAllocation();
	void PreAllocationEx(bool bSort=true);
	// ����
	bool GetUserCard(GameCard &card,char bLarge=eCardlvl_Normal);
	bool GetUserCard(uint32_t playerid,GameCard &card);
	// ����
	bool SwitchCard(GameCard &card);
	bool SwitchCard(uint32_t playerid,GameCard &card/*,bool bLarge=false*/);
	// ����
	uint32_t CompareCard(uint32_t reqid,uint32_t resid);
	// ����
	uint32_t CompareCard(std::vector<uint32_t> vecid);
	// �Ƚϻ�������
	bool  SmallofBaseCard(GameCard &card);
	// 
	bool  SmallofSingleCard(GameCard &card);

protected:
	bool WashCard_ModelA();
	bool WashCard_ModelB();
	bool WashCard_ModelC();
	bool WashCard_ModelD();
	bool WashCard_ModelE();
	bool WashCard_ModelF();

	bool WashCard_ModelAEx();
	bool WashCard_ModelAEx2();
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

	Vec_GameCard	m_Card2AllUser;
private:
	Vec_UserID		m_VecSupperID;	// 
	Vec_UserID		m_VecRobotID;	// 
	Vec_UserID		m_VecUserID_A;
	Vec_UserID		m_VecUserID_B;
	Vec_UserID		m_VecUserID_C;

	Map_PlayerLabel	m_MapPlayerID;	// 

	Map_PlayerLabel	m_MapUserPos;

	Map_PlayerCard	m_MapGameCard;

	int32_t			m_nBasePair;

	std::vector<int> m_CardMgrRate;
};

ZJH_SPACE_END
