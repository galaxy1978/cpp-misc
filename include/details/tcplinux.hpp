/**
 * @brief linux的TCP客户端.使用epoll实现
 * @version 1.0
 * @author 宋炜
 * @date 2023-9-11
 */

#pragma once
#include <sys/epoll.h>
#include <functional>
#include <memory>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>

#include "details/socketbase.hpp"

class tcpLinux : public itfcSocket
{
public:
	
private:
	std::string               m_server__;
	std::atomic<uint16_t>     m_port__;
	// 两个处理的回调函数，上面的处理对外数据通知。对SSL socket来说是已经解密完成的
	std::function< void (const uint8_t * data , size_t len ) > m_cb_recv__;
	// 这个用来处理IOCP和EPOLL时间
	std::function< void (native_socket sock ) >  m_cb_read_data__;
	std::atomic< int >        m_epoll_fd__;
	std::atomic< int >        m_tcp_fd__;
	// 事件循环开关
	std::atomic< bool >       m_is_running__;
	std::mutex                m_mutex__;
	std::atomic< int >        m_max__;
	// epoll 的时间数组
	epoll_event               pt_events__[512];
private:

	void backend__();
public:
	tcpLinux( const std::string& url , uint16_t port );
	virtual ~tcpLinux();
	virtual bool connect( )final;
	virtual bool connect( const std::string& url )final{ return false;}
	virtual bool connect( const std::string& url , uint16_t port ) final;
	virtual size_t send( const uint8_t * data , size_t len )final;
	virtual void close()final;
	virtual void onData( std::function< void ( const uint8_t * data , size_t len ) > fun ) final{
		m_cb_recv__ = fun;
	}
	// 这个函数中执行实际的读取数据操作，在sslSocket类中可以看到具体实现
	virtual void onRecv( std::function< void (native_socket sock ) > fun )final{
		m_cb_read_data__ = fun;
	}

	virtual void run( bool sw ) final;

	virtual native_socket getHandle() final{
		return m_tcp_fd__.load();
	}
};
