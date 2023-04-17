/**
 * @brief 使用libuv实现的 TCP 客户链接管理模块
 * @version 1.0
 * @date 2021-2-20
 * @author 宋炜
 */

#pragma once

#include <string>
#include <memory>
#include <functional>
#include <atomic>

#include "uv.h"
class tcpConnect
{
public:
        enum emErrCode{
            ERR_ALLOC_MEM = -1000,
            ERR_INIT_SOCKET,
            ERR_SOCKET,
	    ERR_SEND_DATA,
	    ERR_TCP_NULL,
            OK = 0
        };

	struct stSendReq{
		uv_write_t   req;
		uv_buf_t     data;
		tcpConnect * obj;

		stSendReq():obj( nullptr ){}
	};
private:
        std::string                     m_name;
	std::string                     __m_peer_ip;
	int                             __m_peer_port;
	uv_tcp_t                      * __p_tcp;
	std::shared_ptr< char >         p_buff;
	std::atomic< bool >             __m_is_stop;

	std::function< void ( const std::string& , const char * , size_t ) >	__cb_on_data;
	std::function< void ( const std::string& ) >				__cb_disconnected;
	std::function< void ( const std::string& , emErrCode ) >		__cb_error;
public:
	tcpConnect( uv_tcp_t *tcp = nullptr );
        ~tcpConnect();
	/**
	 * @brief 响应内存分配请求
	 */
	void onAllocMem( uv_buf_t * buf );
	/**
	 * @brief 启动执行
	 */
	void start();
        /**
         * @brief 读取错误消息
         * @return 返回错误编号对应的错误内容
         */
        static std::string errMsg( emErrCode );
        /**
         * @brief 指定连接名称或者读取连接名称
         */
        void name( const std::string& n ){ m_name = n; }
        const std::string& name() const{ return m_name; }
	/**
	 * @brief 发送数据
	 * @param data
	 * @param len
	 */
	void send( const char * data , size_t len );
	/**
	 * @brief 获取对端对峙
	 */
	std::string peerAddress();
	/**
	 * @brief 获取对端端口
	 */
	int peerPort();
	/**
	 * @brief 关闭连接
	 */
	void close();
	/**
	 * @brief 设置事件回调函数
	 */
	inline void onRecv( std::function< void ( const std::string& , const char * , size_t len )> fun ){ __cb_on_data = fun; }
	inline void onDisconnected( std::function< void ( const std::string& ) > fun ){ __cb_disconnected = fun; }
	inline void onError( std::function< void ( const std::string& , emErrCode ) > fun){ __cb_error = fun; }
	/**
	 * @brief 处理接受数据
	 */
	void onData( const char * data , ssize_t len );
	/**
	 * @brief 关闭
	 */
	void onClosed();
	/**
	 * @brief 发送失败的回调函数
	 */
	void onSendFail();
private:
        /**
         * @brief 初始化模块
         */
        bool __init();
};
