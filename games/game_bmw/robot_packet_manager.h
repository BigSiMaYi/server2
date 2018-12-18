#pragma once
#include <enable_smart_ptr.h>

#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib, "libprotobuf-gd.lib")
#else
#pragma comment(lib, "libprotobuf.lib")
#endif

#endif

#include <enable_object_factory.h>
#include <enable_singleton.h>
#include <com_log.h>


template <typename T>
class robot_packet_factory :
	public object_factory_handler<T>
{
public:
	robot_packet_factory(){}
	virtual ~robot_packet_factory(){}

	virtual bool packet_process(uint32_t player_id, boost::shared_ptr<void> msg) 
	{
		return packet_process(player_id, CONVERT_POINT(T, msg));
	};

	virtual bool packet_process(uint32_t player_id, boost::shared_ptr<T> msg) = 0;
};


class robot_packet_manager:
	public enable_object_factory,
	public enable_singleton<robot_packet_manager>
{
public:
	robot_packet_manager(){}
	virtual ~robot_packet_manager(){}

};

#define ROBOT_PACKET_REGEDIT_RECV(packet) \
class packet##_rfactory : public robot_packet_factory<packet>\
{\
public:\
	packet##_rfactory()\
	{\
\
	};\
	static void regedit_factory()\
	{\
		packet tmp;\
		robot_packet_manager::instance().regedit_object(tmp.packet_id(), boost::make_shared<packet##_rfactory>());\
	};\
	virtual ~packet##_rfactory(){}\
	virtual bool packet_process(uint32_t player_id, boost::shared_ptr<packet> msg);\
};\








