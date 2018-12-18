//基础服务器
#pragma once
#include <net/peer_tcp_server.h>

#if defined(WIN32)
#include <tchar.h>
#else
typedef wchar_t     _TCHAR;
#endif

#include <enable_smart_ptr.h>
#include <enable_xml_config.h>

#if defined(WIN32)
#include <enable_minidump.h>
#endif

#include <enable_id_generate.h>
#include <boost/atomic.hpp>
#include <boost/function.hpp>

#if defined(WIN32)
class server_base:
	//public interrupt_service_handler,
	public enable_minidump_handler
#else
class server_base
#endif
{
public:
#if defined(WIN32)
	bool s_init(int argc, _TCHAR* argv[]);	
#else
	bool s_init(int argc, char* argv[]);
#endif

	const enable_xml_config& get_server_cfg(){return xml_cfg;};

	void close();
	virtual void run() {};

	void push_id(uint16_t peer_id);
	uint16_t generate_id();
	uint16_t get_peer_count();
	enable_xml_config& get_cfg(){return xml_cfg;};
	uint16_t get_serverid(){return m_serverid;}
	std::string & get_serverIP() { return m_serverIP; }

	//boost::asio::io_service& get_timer_service(){return m_timer_service;};
	boost::asio::io_service& get_io_service(){return m_io_service;};
	//添加1个计时器
	void add_server_timer(boost::function0<void> func, int s);

	// 返回服务器ID(每组服务器有相同的ID)
	void set_groutid(uint16_t v);
	uint16_t get_groupid();
protected:
	server_base(void);
	~server_base(void);

	bool s_run();
	bool is_runing();

	//s_init 之后执行
	virtual bool on_init() = 0;

	virtual void on_exit() = 0;

	virtual boost::shared_ptr<peer_tcp> create_peer() = 0;


	void run_timer();//需要使用计时器时在run里调用
	
	boost::thread_group work_grp;
	enable_xml_config xml_cfg;
	boost::asio::io_service m_io_service;//网络使用的io_service
	boost::asio::io_service m_timer_service;//定时器使用的io_service
	boost::scoped_ptr<peer_tcp_server> tcp_server;
	enable_id_generate<uint16_t> id_queue;
	boost::atomic_uint16_t m_ncount;
protected:
	// 游戏服务器组标识ID号
	uint16_t m_groupid;
private:
	//boost::scoped_ptr<interrupt_service> it_service;
	boost::asio::signal_set m_signals;
	void s_exit();
	void io_run();	
	void post_accept();
	bool b_run;	
	uint16_t m_serverid;
	//@add by Hunter 2017/10/09;
	uint16_t m_serverport;
	//@add by Hunter 2017/07/21;
	//@服务注册;
	std::string m_serverIP;

	bool b_closing;
};

