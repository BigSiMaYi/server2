#pragma once

#ifdef WIN32
//#define NOMINMAX
//#define MONGO_EXPOSE_MACROS
#pragma comment(lib, "dbghelp.lib")

//#ifdef _DEBUG
//#pragma comment(lib, "libmongoclient-gd.lib")
//#else
//#pragma comment(lib, "libmongoclient.lib")
//#endif

#endif


#include <mongo/db/jsobj.h>
#include <mongo/client/write_concern.h>
#include <enable_smart_ptr.h>
#include <string>
#include <vector>



namespace mongo
{
	class DBClientConnection;
	class DBClientBase;
	class DBClientReplicaSet;
}

//@初始化空闲连接池;
typedef boost::shared_ptr<mongo::DBClientConnection> connPtrType;

class db_base
{
public:
	db_base(void);
	virtual ~db_base(void);
	void set_userpwd(const std::string& user, const std::string& pwd, bool crypto = true);
	void init_db(std::string& _db_addr, std::string& _db_name);

	//Replica Set   
	//_db_addrs 空格间隔 "127.0.0.1:1000 127.0.0.1:1001"
	void init_db_rs(std::string& _db_addrs, std::string& _db_name);

	const std::string& insert(const std::string& table_name, const mongo::BSONObj& bObj);

	const std::string& insert(const std::string& table_name, const std::vector<mongo::BSONObj>& bObjs);

	mongo::BSONObj findone(const std::string& table_name, const mongo::BSONObj& bObj, const mongo::BSONObj* bField = nullptr);

	void find(std::vector<mongo::BSONObj>& vec,const std::string& table_name, const mongo::BSONObj& bObj, const mongo::BSONObj* bField = nullptr,
		int toReturn = 0, int toSkip = 0, const mongo::BSONObj* bSort = nullptr);

	const std::string& update(const std::string& table_name, const mongo::BSONObj& bObj, const mongo::BSONObj& bUp, bool upsert = true, bool multi = false);

	//@add by Hunter 2017/08/25;
	//@findAndModify
	const std::string & findAndModify(std::vector<mongo::BSONObj>& vec ,const std::string& 	table_name,
		const mongo::BSONObj & 	query,
		const mongo::BSONObj & 	update,
		bool 	upsert = false,
		bool 	returnNew = false,
		const mongo::BSONObj & 	sort = mongo::BSONObj(),
		const mongo::BSONObj & 	fields = mongo::BSONObj(),
		const mongo::WriteConcern * 	wc = NULL,
		bool 	bypassDocumentValidation = false
	);

	const std::string& remove(const std::string& table_name, const mongo::BSONObj& bObj, bool justone = false);

	bool ensure_index(const std::string& table_name, const mongo::BSONObj& bKey, const std::string& index_name, bool unique = false);

	int64_t get_count(const std::string& table_name, const mongo::BSONObj& bObj= mongo::BSONObj());
	// 删除表格table_name
	const std::string& clearTable(const std::string& table_name);
	bool exists(const std::string& table_name);

	const std::string& get_last_error();

	virtual void init_index(){};

	virtual void close();

	//@1.初始化一定量;
	void initConnPool(size_t Num);
	//@2.使用申请;
	connPtrType getConnFromPoll();
	//@3.用完回收;
	void putDBConn(connPtrType conn);

private:
	mongo::DBClientBase* get_conn();
	connPtrType createDBConn();
	std::string db_name_base;
	std::string db_name;
	std::string db_addr;
	std::string db_err;
	std::string db_user;
	std::string db_pwd;
	bool b_auth;
	bool b_init;
	//bool b_thread;
	bool b_useReplicaSet;
	
	boost::scoped_ptr<mongo::DBClientConnection> m_conn;
	//@add by Hunter,qinfen;
	//@2017/10/18;
	//std::mutex m_connMtx;
	std::list<connPtrType> m_vecFree;
	std::list<connPtrType> m_vecUsing;

	boost::scoped_ptr<mongo::DBClientReplicaSet> m_conn_rs;
};
