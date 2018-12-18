#pragma once

#include <db_base.h>
#include <enable_singleton.h>

//////////////////////////////////////////////////////////////////////////
static const std::string unknown_table = "DefaultTable";



//������ݿ�
class db_game : public db_base
	, public enable_singleton<db_game>
{
public:
	db_game();
	virtual ~db_game();
	virtual void init_index();
};