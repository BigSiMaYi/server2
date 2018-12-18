// stdafx.cpp : ֻ������׼�����ļ���Դ�ļ�
// game_robcows.pch ����ΪԤ����ͷ
// stdafx.obj ������Ԥ����������Ϣ

#include "stdafx.h"

// TODO: �� STDAFX.H ��
// �����κ�����ĸ���ͷ�ļ����������ڴ��ļ�������

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