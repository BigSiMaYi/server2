// stdafx.cpp : ֻ������׼�����ļ���Դ�ļ�
// game_zjh.pch ����ΪԤ����ͷ
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

int64_t GetCurTotalSec()
{
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();

	return now.time_of_day().total_seconds();
}
