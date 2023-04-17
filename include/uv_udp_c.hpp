/**
 * @brief 使用libuv实现的客户端连接.主要面向PC端的应用开发支持，也可以将这个库移植到android和
 *  IOS系统上完成三种应用上统一的接口。
 * @version 0.1
 * @date 2017-2-14
 * @author 宋炜
 */

#ifndef __UV_UDP_CLIENT_HPP__
#define __UV_UDP_CLIENT_HPP__

#include <uv.h>
#include <string>
#include "misc.hpp"

class CUVUDPClient
{
public:
	/**
	 * 客户端操作错误代码定义
	 */
	enum err_code
	{
		ERR_UVCLIENT_INIT_MUTEX = -1000,
		ERR_UVCLIENT_LOOP_CONFIG ,
		ERR_UVCLIENT_LOOP_INIT,
		ERR_UVCLIENT_INIT_TCP,
		ERR_UVCLIENT_OPERATION,
		ERR_UVCLIENT_ALOCMEM,
		ERR_UVCLIENT_CONNECTION,
		ERR_RESOLVE_DNS,
		OK = 0
	};
	/**
	 * 在发送完成后需要调用回调函数处理一些事情。因此需要将这些数据传递
	 * 给回调函数，方便回调函数能够找到返回的对象。发送的数据长度
	 */
	struct st_wd{
		uv_buf_t    __buff;
		void     *  __this;
		size_t      __ds;
	};
public:
	bool                  m_dns_flag;                //dns解析是否完成
	bool                  m_conn_flag;               //连接是否完成
	uv_mutex_t            m_mutex;                   //缓冲操作互斥对象
	uv_buf_t              m_o_buf;                   //输出缓冲
	uv_buf_t              m_i_buf;                   //输入缓冲


	std::function< void ( size_t , const char * )>   cb_recv;
	std::function< void ( size_t )>                  cb_send;
	std::function< void () >                         cb_close;
	std::function< void () >                         cb_conn;
private:
	uv_thread_t          threadhanlde;                //
	uv_udp_t             m_udp;                       // libuv UDP 句柄
	int                  m_port;                      //服务器端口
	std::string          m_remote;                    //远程地址，可以是url或者IP地址，支持IP6和IP4

	struct sockaddr      m_addr;                      // 通过域名获取的IP地址

	err_code             m_err;
	std::string          err_msg;
private:
    	void connecturl( );
public:
	/**
	 * @brief 构造客户端对象
	 * @param url , 远程主机地址
	 * @param port , 远程服务端口
	 */
	CUVUDPClient( const std::string& url , int port );
	virtual ~CUVUDPClient(  );
	/**
	 * @brief 连接操作。
	 * @note UDP的连接操作和TCP存在差别，在这里UDP的连接操作只是解析域名将域名转化成
	 *       sockaddr 结构的指针
	 * @param url
	 * @param port
	 */
	void connect();
	void connect( const std::string& url , int port );

	operator bool()
	{
		return true;
	}

	/**
	 * @brief 发送数据。这个函数发送的时候不会将要发送的数据拷贝到缓冲区中
	 *   因此需要在具体应用的时候进行数据内存管理。
	 * @note 因为UDP无法探测对点是否有效。所以当DNS解析出来多台服务器地址的时候无法确定
	 *   哪一个地址是有效的。因此需要解析后获取的目标地址应用可以有两个方案，一是向其中一个
	 *   地址发送数据;而是向每一个服务器发送数据。
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
	void OnRecv( const char* buff , size_t s , err_code );

	uv_udp_t* Client(  )
	{
		return &m_udp;
	}

	const std::string& GetURL(  )const
	{
		return m_remote;
	}

	int GetPort(  )
	{
		return m_port;
	}
	const std::string& server( )const{ return m_remote; }
	void server( const std::string& svr ) { m_remote = svr; }
	int port(){ return m_port; }
	void port( int port ){ m_port = port; }
	err_code Error(  )
	{
		return m_err;
	}

	void Error( int e )
	{
		if( !e ){
			m_err = ERR_UVCLIENT_OPERATION;
			err_msg = uv_strerror( e );
		}
		else{
			m_err = OK;
			err_msg.clear(  );
		}
	}

	std::string ErrMsg(  )
	{
		return err_msg;
	}

	bool need_dns( const std::string& url );
};
#endif
