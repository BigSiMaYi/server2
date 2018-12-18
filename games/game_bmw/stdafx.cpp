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

int64_t GetSecondsCount()
{
#if 0
	boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
	return now.time_of_day().total_milliseconds();
#else
	boost::posix_time::ptime eponch(boost::gregorian::date(1970,boost::gregorian::Jan,1));
	boost::posix_time::ptime eponch2 = boost::posix_time::microsec_clock::universal_time();
	boost::posix_time::time_duration time_from_epoch = eponch2 - eponch;
	return time_from_epoch.total_seconds();
#endif
	
}
