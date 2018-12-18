#pragma once
#include "card_def.h"
#include "card_common.h"
LANDLORD_SPACE_BEGIN


// static char _fixpv(const char value ,const char style )
// {
// 	if (POKER_STYLE_EX==style)
// 	{
// 		if(value==POKER_VALUE_JOKER_SMALL)
// 		{
// 			return PV_S;
// 		}
// 		else if(value==POKER_VALUE_JOKER_LARGE)
// 		{
// 			return PV_L;
// 		}
// 	}
// 	if(value==POKER_VALUE_A)
// 	{
// 		return PV_A;
// 	}
// 	else if(value==POKER_VALUE_2)
// 	{
// 		return PV_2;
// 	}
// 
// 	return value;
// }
// 
// // Ext
// inline char _fixpv(const POKER & p)
// {
// 	return _fixpv(p.value,p.style);
// }

//////////////////////////////////////////////////////////////////////////
struct POKER_GREATER
{	
	bool operator()(const POKER& _Left, const POKER& _Right) const
	{	// apply operator> to operands
		if(_fixpv(_Left.value,_Left.style) > _fixpv(_Right.value,_Right.style))
		{
			return true;
		}
		else if(_fixpv(_Left.value,_Left.style) == _fixpv(_Right.value,_Right.style))
		{
			if(_Left.style > _Right.style) return true;
		}
		return false;
	}
};

struct POKER_LESS
{	
	bool operator()(const POKER& _Left, const POKER& _Right) const
	{	// apply operator< to operands
		if(_fixpv(_Left.value,_Left.style) < _fixpv(_Right.value,_Right.style))
		{
			return true;
		}
		else if(_fixpv(_Left.value,_Left.style) == _fixpv(_Right.value,_Right.style))
		{
			if(_Left.style < _Right.style) return true;
		}
		return false;
	}
};

struct POKER_EQUAL
{
	bool operator()(const POKER& _Left, const POKER& _Right) const
	{	// apply operator== to operands
		return _Left.value==_Right.value&&_Left.style==_Right.style;
	}
};
//////////////////////////////////////////////////////////////////////////


LANDLORD_SPACE_END