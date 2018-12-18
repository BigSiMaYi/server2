// stdafx.cpp : 只包括标准包含文件的源文件
// game_zjh.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

// TODO: 在 STDAFX.H 中
// 引用任何所需的附加头文件，而不是在此文件中引用

bool isVaildSeat(uint16_t wChair)
{
	if(wChair>=0 && wChair<GAME_PLAYER)
		return true;
	return false;
}

int64_t GetCurTotalSec()
{
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

	return now.time_of_day().total_seconds();
}
