// stdafx.cpp : 只包括标准包含文件的源文件
// game_robcows.pch 将作为预编译头
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

std::string GBK_2_UTF8(std::string& utfstr)
{
	std::string asc_str;
	try
	{
		asc_str = boost::locale::conv::between(utfstr, "UTF-8", "GBK");
	}
	catch (std::exception& e)
	{
		SLOG_ERROR << e.what();
	}
	return std::move(asc_str);
}