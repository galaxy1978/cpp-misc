/**
 * @brief ssl服务器EPOLL引擎
 * @version 1.0
 * @author 宋炜
 */

#pragma once

#include <vector>
#include <functional>
#include <unordered_set>

#include "designM/singleton.hpp"

#include "sslSvrItfc.hpp"

class sslEpollEgn : public wheels::dm::singleton< sslEpollEgn >
{
public:
	using cbFun_t = std::function< void ( native_socket ) >;
private:
	std::atomic< int >     m_id__;   // epoll id
	cbFun_t                m_cb__;   // 事件处理回调函数
	std::unordered_set< int >   m_handles__;
private:
	void backend__( long ovt );
public:
	sslEpollEgn( int maxEvt = 512 );
	virtual ~sslEpollEgn();
        /**
	 * @brief 添加socket
	 */
	void add( int handle );
	void remove( int handle );
	
	bool has( int handle ){
		auto it = std::find( m_handles__.begin() , m_handles__.end() , handle );
		return it != m_handles__.end();
	}

	void onSocket( cbFun_t fun ){ m_cb__ = fun; }

	void run( bool sw , long ovt );
};


