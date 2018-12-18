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
	// 重置
	void Reposition();
	// 设置标签
	void SetUserLabel(uint32_t uid,int32_t nLabel);
	// 设置位置
	void SetUserPos(  uint32_t uid, int32_t pos);
	// 洗牌
	void Wash_Card();
	// 
	void Wash_Card2();
	void Wash_Card3();
	void Wash_CardNomal();
	void Wash_CardControl();
	// 分配牌
	void PreAllocation();
	void PreAllocationEx();
	// 发牌
	bool GetUserCard(GameCard &card);
	bool GetUserCard(uint32_t playerid,GameCard &card);
	uint32_t GetCardRate(GameCard &card) const;
	// 换牌
	bool SwitchCard(uint32_t playerid,GameCard &card/*,bool bLarge=false*/);
	// 比牌
	uint32_t CompareCard(uint32_t reqid,uint32_t resid);
	// 比牌
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

	Vec_GameCard	m_Card2Supper;	// 超级用户牌库
	Vec_GameCard	m_Card2RobotL;	// 机器人牌库
	Vec_GameCard	m_Card2RobotS;	// 机器人牌库
	Vec_GameCard	m_Card2UserA;	// 玩家牌库
	Vec_GameCard	m_Card2UserB;	// 玩家牌库
	Vec_GameCard	m_Card2UserC;	// 玩家牌库
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
