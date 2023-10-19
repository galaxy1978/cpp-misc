/**
 * @brief TCP客户端的接口定义
 * @version 1.0
 * @author 宋炜
 * @date 2023-9-11
 */

#pragma once

#include <functional>
#include <string>
// 这里是TCP接口定义，为了在不同的操作系统上统一，使用一个接口。
// 我们实现IOCP和EPOLL两种方式
struct itfcSocket
{
	// 这里就是native_socket的类型定义。
	// 下来看，tcp epoll的实现
#if defined( WINNT ) || defined( WIN32 )
#include <windows.h>
	using native_socket = HANDLE;
#elif defined( __LINUX__ )
	using native_socket = int;
#endif
	virtual bool connect( ) = 0;
	/**
	 * @brief 这个函数处理标准URL连接方式。
	 */
	virtual bool connect( const std::string& url ) = 0;
	/**
	 * 这个实现服务器地址，端口号的方式
	*/
	virtual bool connect( const std::string& url , uint16_t port ) = 0;
	/**
	 * 这个函数用来处理发送
	*/
	virtual size_t send( const uint8_t * data , size_t len ) = 0;
	/**
	 * 关闭接口
	 */
	virtual void close() = 0;
	/**
	 * 用来处理EPOLL通知和IOCP通知
	*/
	virtual void onRecv( std::function< void ( native_socket fd ) > fun ) = 0;
	/**
	 * 用来处理数据通知。配置这个函数，在读取到数据的时候会调用配置的回调函数
	*/
	virtual void onData( std::function<void( const uint8_t * data , size_t len )> fun ) = 0;
	/**
	 * 启动后端的通讯事件循环
	*/
	virtual void run( bool sw ) = 0;
	/**
	 * 获取操作系统的socket句柄
	 * 返回值根据操作系统来明确类型
	*/
	virtual native_socket getHandle() = 0;
};
