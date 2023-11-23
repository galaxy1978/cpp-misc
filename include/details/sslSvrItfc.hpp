/**
 * @brief SSL服务器接口定义
 * @version 1.0
 * @author 宋炜
 */


#pragma once
#include <string>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "mallocSharedPtr.hpp"
#include "container/rbTree.hpp"
#include "threadPool.hpp"

class sslSvrConnection;

struct sslSvrItfc
{
public:
#if defined( WIN32 ) || defined( WINNT )
#    include <windows.h>
	using native_socket = HANDLE;
#elif defined( __LINUX__ )
	using native_socket = int;
#endif
	

	using connection_t = std::shared_ptr< sslSvrConnection >
	using connData_t = wheels::container::rbTree< native_socket , connection_t >;

	enum class event
	{
		EVT_REQ_CONNECT,
		EVT_CONNECTED,
		EVT_CLOSE,
		EVT_DATA,
		EVT_ERROR
	};

	using dispatcher_t = wheels::dm::dispatcher< event >;
	struct evtData{
		connection_t    m_conn;
		wheels::mallocSharedPtr< uint8_t >   m_data;
	};
protected:
	SSL_CTX           * p_ctx__;
	
	connData_t          m_connections__;  // 连接表
	wheels::threadPool  m_thd_pool__;     // 线程池
protected:
        
public:
	sslSvrItfc( const std::string& ca , const std::string& cert , const std::string& key );
	virtual sslSvrItfc(){
		auto * p_evt_loop = mainLoop< event >::create();
		if( p_evt_loop == nullptr ){
			throw std::runtime_error( "启动事件循环失败" );
		}
		p_evt_loop->stop();
		m_thd_pool__.start( false );
	}
	
	bool accept( native_socket sock );

	connection_t get( native_socket h );
	connection_t operator[]( native_socket h );

	void erase( native_socket h );

	uint32_t counts();

	wheels::dm::dispatcher * getDispatcher();
};
