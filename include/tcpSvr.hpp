/**
 * @brief 使用QTcpServer实现的TCP服务器模块
 * @version 1.0
 * @date 2020-9-22
 * @author 宋炜
 */

#pragma once

#include <string>
#include <map>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>

#include <uv.h>

#include "uvlper.hpp"
#include "connection.hpp"
#include "dcitfc.hpp"

class tcpSvr : public dcItfc
{
public:
        enum emErrCode{
		ERR_ALLOC_MEM = -1000,
		ERR_INIT,
		ERR_LISTEN,
		OK = 0
        };
private:
	uv_tcp_t                                __m_tcp;                   // 
        std::map< std::string , tcpConnect* >   __m_clients;               // 连接的客户端
        std::function<void ( const std::string& data ) > m_funUpdtTbl;     // 数据回调
	std::mutex    m_mutex;
        int           __m_port;
	/// @以下定义各个操作的事件回调
	/// @{
	std::function< void ( int err ) >		cb_err;
	std::function< void () >			cb_connected;
	std::function< void ( const std::string& ) >	cb_client_connected;
	std::function< void ( const std::string& ) >	cb_client_disconnected;
	std::function< void ( ) >			cb_closed;
	std::function< void ( const char * data , size_t len )> cb_recved;
	std::function< void ( int err ) >               cb_sended;
	std::function< void ()>				cb_stop;
	/// @}
public:
	tcpSvr( );
        tcpSvr( const std::string& ip, int port );
        ~tcpSvr();
        /**
         * @brief 设置数据更新处理的回调函数。主要用来更新表格和文件
         * @param fun[ I ], 回调函数
         *   函数原型为：
         *       void fun( const std::string& data ); 参数data是具体数据内容
         */
        void onUpdateTbl( std::function< void ( const std::string& data ) > fun ){
                m_funUpdtTbl = fun;
        }

	virtual void connect( const std::string& url , bool bind = false ) final;
	virtual void send( const char * data , size_t len ) final;
	virtual void close() final;
	virtual void onRecv( const char * data , size_t len ) final;
	virtual void onError( int err ) final;
	virtual void onConnected() final;
	virtual void onDisconnected() final;
	virtual void onClose() final;
	virtual void onSend( int err ) final;
	virtual void onClientConnected() final;

	virtual const std::string type() const{ return std::string( "tcpserver" ); }

	virtual void evtError( std::function< void ( int err ) > fun ) final;
	virtual void evtConnected( std::function< void () > fun ) final;
	/**
	 * @brief 对于服务器有效
	 * @param fun
	 */
	virtual void evtClientConnected( std::function< void (const std::string& ) > fun ) final;
	virtual void evtClientDisconnected( std::function< void ( const std::string& ) > fun ) final;
	virtual void evtClosed( std::function< void ( ) > fun ) final;
	virtual void evtRecv( std::function< void ( const char * data , size_t len )> fun ) final;
	virtual void evtSend( std::function< void ( int err ) > fun ) final;
	/**
	 * @brief 对于服务器有效.用于关闭监听
	 * @param fun
	 */
	virtual void evtStop( std::function< void ()> fun ) final;
        /*
         * 处理新链接
         * @param sock[ I ], 操作系统套接字句柄编号
         */
        void incomingConnection( int status );
public:
        /**
         * @brief 处理接收到数据，对数据进行解析，呈现等操作
         * @param name[ I ], 远程名称，实际是ip:port的方式
         * @param data[ I ], 接收到的数据
         * @param len[ I ], 数据长度
         */
        void onRecvData( const std::string& name , const char * data , size_t len );
        /**
         * @brief 处理客户端断开操作
         * @param name[ I ], 远程名称
         */
        void onDisconnected( const std::string& name );
        /**
         * @brief 处理客户端错误
         * @param name[ I ] 远程名称
         * @param code[ I ] 错误编号
         * @param msg[ I ] 错误信息
         */
        void onClientError( const std::string& name , tcpConnect::emErrCode code , const std::string& msg );
private:
        /**
         * @brief 初始化对象
	 * @param ip[ I ] 监听IP地址
         * @param port[ I ] 监听的端口
         */
	bool __init( const std::string& ip, int port);
	/**
	 * @brief 断开所有已经连接的客户端
	 */
	void __close_all_clients();
        /**
         * @brief 连接错误处理
         * @param err[ I ] 错误类型
         */
        void __on_acpt_err( int err );

	std::string __get_peer_ip_port( uv_tcp_t * conn );
};
