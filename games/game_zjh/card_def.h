#pragma once
ZJH_SPACE_BEGIN

#include "logic_def.h"


typedef enum 
{
	CARDVALUE_MIN = 2,
	CARDVALUE_2 = CARDVALUE_MIN,
	CARDVALUE_3,
	CARDVALUE_4,
	CARDVALUE_5,
	CARDVALUE_6,
	CARDVALUE_7,
	CARDVALUE_8,
	CARDVALUE_9,
	CARDVALUE_10,
	CARDVALUE_J,
	CARDVALUE_Q,
	CARDVALUE_K,
	CARDVALUE_A,
}CARDVALUE;

#define ZJH_RANGE_VALUE		(CARDVALUE_A-CARDVALUE_MIN+1)

//enum class eCardLevel:UCHAR
enum 
{
	CARDLEVEL_ERROR = 0,	//������û��
	CARDLEVEL_DANPAI,		//����
	CARDLEVEL_DUIZI,		//����
	CARDLEVEL_SHUNZI,		//˳��
	CARDLEVEL_JINHUA,		//��
	CARDLEVEL_SHUNJIN,		//˳��
	CARDLEVEL_BAOZI,		//����

};

typedef std::vector<BYTE> Vector_Card;
typedef std::vector<BYTE>::iterator Vector_CardIterator;

#define MAX_GAME_CARD	17

#define DEL_GAME_CARD	7
#define DEL_GAME_CARDEx	5

#define	GET_GAME_CARD	10

#define ROBOT_GAME_CARD	2
typedef struct _GAMECARD_
{
	// ����
	int32_t nModel;
	// ��ֵ
	std::array<int32_t,MAX_CARDS_HAND> nCard;
	//
	std::array<int32_t,MAX_CARDS_HAND> nCardIdx;
	// ��һ���ƵĻ�ɫ
	int32_t	nHua;
	// �Ƿ��Ѿ���ʹ��
	bool bUseing;
	// 
	char bCheck;
	// 17�����е����к�
	int32_t	nIndex;
	// 
	_GAMECARD_()
	{
		nModel = 0;
		nHua = 0;
		bUseing = 0;
		nIndex = 0;
		nCard.fill(0);
		nCardIdx.fill(0);
	}
	bool operator <(const _GAMECARD_ & other) 
	{
		if(other.nModel < nModel)
		{
			return false;
		}
		else if(other.nModel==nModel)
		{
			switch (nModel)
			{
			case CARDLEVEL_BAOZI:
			case CARDLEVEL_SHUNJIN:
			case CARDLEVEL_SHUNZI:
				{ // �Ƚϵ�һ��--�Ƚϻ�ɫ
					if(nCardIdx[0] > other.nCardIdx[0])
					{
						return false;
					}
					else if (nCardIdx[0] == other.nCardIdx[0])
					{
						if(nHua > other.nHua)
							return false;
					}
				}
				break;
			case CARDLEVEL_JINHUA:
			case CARDLEVEL_DANPAI:
				{ // �Ƚϵ�һ��
					if(nCardIdx[0] > other.nCardIdx[0])
					{//��һ�ű��Ҵ�
						return false;
					}
					else if(nCardIdx[0] < other.nCardIdx[0])
					{//
						
					}
					else if(nCardIdx[1] > other.nCardIdx[1])
					{//�ڶ��ű��Ҵ�
						return false;
					}
					else if(nCardIdx[1] < other.nCardIdx[1])
					{

					}
					else if(nCardIdx[2] > other.nCardIdx[2])
					{//�����ű��Ҵ�
						return false;
					}
					else if(nCardIdx[2] < other.nCardIdx[2])
					{
					}
					else if(nHua > other.nHua)
					{
						return false;
					}
				}
				break;
			case CARDLEVEL_DUIZI:
				{
					if(nCardIdx[0] > other.nCardIdx[0])
					{//��һ�ű��Ҵ�
						return false;
					}
					else if(nCardIdx[0] < other.nCardIdx[0])
					{
					}
					else if(nCardIdx[1] > other.nCardIdx[1])
					{
						return false;
					}
					else if(nCardIdx[1] < other.nCardIdx[1])
					{
					}
					else if(nHua > other.nHua)
					{
						return false;
					}
				}
				break;
			default:
				break;
			}
		}

		return true;
	}

	bool operator ==(const _GAMECARD_ & other)
	{
		if (nCard == other.nCard)
			return true;
		return false;
	}
}GameCard;

typedef std::vector<GameCard> Vec_GameCard;

typedef std::vector<uint32_t> Vec_UserID;

typedef std::map<uint32_t,int32_t> Map_PlayerLabel;
typedef std::pair<uint32_t,int32_t> PAIR;
struct  CmpByValue{
	bool operator()(const PAIR& lhs, const PAIR& rhs) {
		return lhs.second < rhs.second;
	}
};

typedef std::map<uint32_t,Vec_GameCard> Map_PlayerCard;

ZJH_SPACE_END