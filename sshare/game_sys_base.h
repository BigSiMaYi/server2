#pragma  once
#include <enable_smart_ptr.h>

class game_sys_handler
{

};

class game_sys_base
{
public:
	typedef boost::shared_ptr<game_sys_base> SysBasePtr;
	game_sys_base()
	{
		m_handler = nullptr;
		m_load	  = false;
	};
	virtual ~game_sys_base()
	{
		m_handler = nullptr;
	};
	void attach(game_sys_handler* player)
	{
		m_handler = player;
		on_attach();
	}

	virtual void on_attach(){};
	//ע������
	virtual void init_sys_object(){};
	//��������
	virtual bool sys_load(){ m_load = true; return true;};
	//��ʼ������
	virtual void sys_init(){};
	// ÿ֡����
	virtual void sys_update(double delta){}
	//0�����
	virtual void sys_time_update(){};
	//ϵͳ����game_sys_def.h
	virtual uint16_t get_sys_type()=0;
	
	// ϵͳ�˳�
	virtual void sys_exit(){}

	template<class T>
	T* get_owner()
	{
		return static_cast<T*>(m_handler);
	};
private:
	game_sys_handler*	m_handler;
protected:
	bool				m_load;		//add by yxr 2017/6/30�´�����һ��в������sys_load���
};


//����ʹ��
#define MAKE_SYS_TYPE(stype)\
virtual uint16_t get_sys_type()\
{\
	return stype;\
};\
static uint16_t static_type()\
{\
	return stype;\
};\


//��ѡ
#define MAKE_GET_OWNER(stype)\
stype* get_##stype()\
{\
	return get_owner<stype>();\
};\




