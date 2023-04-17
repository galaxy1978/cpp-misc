/**
 * @brief 使用libuv实现的客户端连接.主要面向PC端的应用开发支持，也可以将这个库移植到android和
 *  IOS系统上完成三种应用上统一的接口。
 * @version 1.1
 * @date 2017-2-14 ~ 2020-10-10
 * @author 宋炜
 */

#ifndef __UV_TCP_CLIENT_HPP__
#define __UV_TCP_CLIENT_HPP__

#include <uv.h>
#include <string>
#include <functional>
#include <atomic>

#include "misc.hpp"
#include "uvlper.hpp"
#include "timer.hpp"

class CUVClient
{
public:
        /// 客户端操作错误代码定义
        /// @{
	enum err_code {
		ERR_UVCLIENT_INIT_MUTEX = -1000,
		ERR_UVCLIENT_LOOP_CONFIG ,
		ERR_UVCLIENT_LOOP_INIT,
		ERR_UVCLIENT_INIT_TCP,
		ERR_UVCLIENT_OPERATION,
		ERR_UVCLIENT_ALOCMEM,
		ERR_UVCLIENT_CONNECTION,
		ERR_UV_DNS_RESOLVE,
		ERR_ALLOC_MEM,
		ERR_UVCLIENT_RECV,
		ERR_UVCLIENT_SEND,
		ERR_UVCLIENT_DNS,
		UV_CLIENT_CREATE_THD,
		ERR_CONN_OVERTIME,
		ERR_SEND_DATA_EMPTY,
		ERR_DATA_NULL,
		OK = 0
        };
	using emErrCode = err_code;
	/**
	 * @}
	 * 发送请求结构体。
	 * 在发送完成后需要调用回调函数处理一些事情。因此需要将这些数据传递
	 * 给回调函数，方便回调函数能够找到返回的对象。发送的数据长度
	 */
	struct st_wd{
		void *__buff;
		void *__this;
		size_t  __ds;

		st_wd():__buff( nullptr ), __this( nullptr ), __ds( 0 ){}
	};
	/**
	 * 连接请求结构
	 */
	struct st_connreq{
		CUVClient  * obj;

		st_connreq():obj( nullptr ){}
	};
public:
	uv_connect_t          req;                //连接请求句柄
	uv_getaddrinfo_t      dns;                //dns解析返回值

	bool                  m_dns_flag;         //dns解析是否完成
	std::atomic<bool>     m_conn_flag;        //连接操作是否完成
	uv_mutex_t            m_mutex;            //缓冲操作互斥对象
	//uv_buf_t              m_o_buf;            //输出缓冲
	uv_buf_t              m_i_buf;            //输入缓冲
	uv_tcp_t            * p_client;           //客户端TCP连接句柄
	std::atomic<bool>     is_connected;       // 是否链接标志
	std::atomic<bool>     is_run;             // 是否正在运行

	std::function< void ( size_t , const char * )>    cb_recv;      // 接收到数据后的回调函数
	std::function< void ( size_t )>                   cb_send;      // 发送数据后的回调函数
	std::function< void ()>                           cb_close;     // 通道关闭后的回调函数
	std::function< void ()>                           cb_conn;      // 连接后的回调函数
	std::function< void( const struct addrinfo* cb )> cb_resolv;    // 域名解析后的回调函数
	std::function< void( err_code )>                  cb_error;     // 发送后的回调函数
private:
	uv_thread_t               threadhanlde;                // libuv 线程句柄
	CTimer                    m_conn_overtime;             // 连接超时计时器
	std::atomic<int>          m_port;                      // 服务器端口
	std::string               m_remote;                    // 远程地址，可以是url或者IP地址，支持IP6和IP4

	std::atomic<err_code>     m_error;                     // 错误代码
	std::string               err_msg;                     // 错误信息
	std::atomic< bool >       m_is_closed;                 // 通道是否开启
        bool                      m_closed_on_error;           // 
	struct addrinfo         * p_resolve_rst;               // DNS解析后的结果
	struct addrinfo         * p_conn_current;              // 当前尝试连接的地址
private:
	/**
	 * @brief 执行IPV4连接操作
	 * @param req , 连接请求
	 */
	void connect4( );
	/**
	 * @brief 执行IPV6连接请求
	 */
	void connect6( );
	/**
	 * @brief 执行DNS连接请求
	 */
	void connecturl( );
	/**
	 * @brief 最终的连接操作
	 * @param add , 目标地址
	 * @return 成功操作返回OK，否则返回错误代码
	 * @note 本函数返回OK后，并不意味着成功连接，只是成功发出连接请求
	 */
	err_code real_connect( struct sockaddr* add );
	/**
	 * @brief 𥘉始化TCP底层和事𢓐循环
	 */
	void init();
	/**
	 * @brief 结束通道
	 */
	void stop();
	/**
	 * @brief 保存DNS解析结果
	 * @param rest, DNS解析结果
	 */
	void save_dns_result( struct addrinfo *rest );
	/**
	 * @brief 准备DNS结果缓冲区
	 */
	void init_dns_result();
	/**
	 * @brief 释放DNS结果缓冲区
	 */
	void free_dns_result();
public:
	/**
	 * @brief 构造客户端对象
	 * @param url , 远程主机地址
	 * @param port , 远程服务端口
	 */
	CUVClient( const std::string& url , int port );
	virtual ~CUVClient(  );
	/**
	 * @brief 外界操作连接口
	 * @param url ， 服务器地址
	 * @param port ， 服务端口
	 */
	void connect();
	void connect( const std::string& url , int port );
	/**
	 * @brief 获取连状态
	 */
	operator bool()
	{
		return is_connected.load();
	}
	inline bool isConnected()
	{
		return is_connected.load();
	}
	/**
	 * @brief 发送数据。这个函数发送的时候不会将要发送的数据拷贝到缓冲区中
	 *   因此需要在具体应用的时候进行数据内存管理。
	 * @param data , 要发送的数据。
	 * @param s , 要发送的数据的长度
	 * @retval 返回操作结果，成功返回OK ,否则返回错误代码。
	 * @exception 出现致命错误时抛出错误代码
	 */
	err_code Send( const char* data , size_t s );
	/**
	 * @brief 完成发送后会调用这个函数
	 * @param s ,实际发送的长度
	 * @param e ,操作的错误代码
	 *
	 */
	virtual void OnSend( size_t s , err_code e  );
	/**
	 * @brief 接收到数据后，调用这个函数
	 * @param buff ， 接收到的数据的缓冲区指针
	 */
	virtual void OnRecv( const char* /*buff*/ , size_t /*s */, err_code ){}
	/**
	 * @brief 断开连接
	 */
	void Close();
	/**
	 * @brief 获取客户端指针
	 */
	uv_tcp_t* Client(  )
	{
		return p_client;
	}

	inline const std::string& GetURL(  )const
	{
		return m_remote;
	}

	inline int GetPort(  )
	{
		return m_port;
	}
	/**
	 * @brief 获取服务端地址，可能是URL也可能是IP地址
	 * @return 远程地址内容
	 */
	const std::string& server( )const{ return m_remote; }
	/**
         * @brief 指定远程服务器地址
	 */
	void server( const std::string& svr ) { m_remote = svr; }
	/**
	 * @brief 获取服务器端口
	 */
	int port(){ return m_port; }
	/**
	 * @brief 指定服务器端口
	 */
	void port( int port ){ m_port = port; }
	/**
         * @brief 返回操作错误代码
	 */
	err_code Error()
	{
		return m_error;
	}
	/**
	 * @brief 获取操作错误字符串描述
         * @param e[ I ]， 错误编号
	 */
	void Error( int e )
	{
		if( e ){
			m_error = ERR_UVCLIENT_OPERATION;
			err_msg = uv_strerror( e );
		}else{
			m_error = OK;
			err_msg = "";
		}
	}
	static std::string errMsg( int e );
	/**
	 * @brief 获取错误信息
	 */
	std::string ErrMsg()
	{
		return err_msg;
	}
	/**
	 * @brief 处理libuv的错误
	 * @param error[ I ], libuv错误编号
	 */
	void onError( int error );
	/**
	 * @brief 处理shutdown事件
	 */
	void onShutdown();
	/**
	 * @brief 处理通道关闭
	 */
	void OnClose();
	/**
	 * @brief 连接后调用这个函数
         * @param e[ I ], 连接操作的错误代码
	 */
	void OnConnected( err_code e );
	/**
	 * @brief 设置打开事𢓐回调函数
	 * @param fun ， 要设置的回调函数
	 */
	void on_open( std::function< void()> fun )
	{
		cb_conn = fun;
	}
	/**
	 * @brief 设置关闭回调函数
	 * @param f[ I ]， 关闭事件处理的回调函数
	 *    void fun();
	 */
	void on_close( std::function<void()> f )
	{
		cb_close = f;
	}
	/**
	 * @brief 设置数据接收回调函数
	 * @param fun【 I 】接收到数据的回调函数
         *      void fun( size_t len , const char * data );
	 */
	void on_data( std::function< void (size_t , const char *  ) > fun )
	{
		cb_recv = fun;
	}
	/**
	 * @brief 设置错误处理回调函数
	 * @param f【 I ]， 错误的回调函数
	 *        void fun( CUVClient::err_code e );
	 */
	void on_error( std::function< void (CUVClient::err_code) > f )
	{
		cb_error = f;
	}
	/**
	 * @brief 设置发送完成回调函数
	 * @param f [ I ], 发送完成的回调函数
         *    函数原型为：void fun( size_t len ); ， len是已经发送数据长度
	 */
	void on_send( std::function< void (size_t )> f )
	{
		cb_send = f;
	}
};
#endif
