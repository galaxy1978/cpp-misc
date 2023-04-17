/**
 * @brief 使用libuv实现的udp客户端
 * @version 0.1
 * @date 2018-2-27
 * @author 宋炜
 */
#include <unistd.h>
#include <regex>
#include <thread>
#include <chrono>
#include "err_code.h"
#include "uv_udp_c.hpp"
#include "resolver.hpp"
#include "uvlper.hpp"

void uv_on_udp_shutdown_cb( uv_shutdown_t* req, int status)
{
    	if( req ) free( req );
}

void do_on_udp_alloc_buff( uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf)
{
	CUVUDPClient *pt = ( CUVUDPClient* )( handle->data );

	buf->base = pt->m_i_buf.base;
	buf->len = pt->m_i_buf.len;
}

void do_on_udp_recv(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags )
{
	CUVUDPClient *pt = (CUVUDPClient*) ( handle->data );
	if( pt != NULL && nread > 0){
        /*
#if defined( __DEBUG__ )

        char buff[ 100 ];
        memcpy( buff , buf->base , nread );
        buff[ nread ] = 0;
        std::cout << "\nRecv net: " << buff << std::endl;
        #endif*/
        //uv_mutex_lock( &(pt->m_mutex) );
        pt->OnRecv( buf->base , nread , CUVUDPClient::OK );
        //uv_mutex_unlock( &(pt->m_mutex) );
    	}
    	else if( nread < 0 ) {
       		LOGLN( true , "Sever Error." );
        //exit(-1);
    	}else{
        // TODO
    	}
}

void do_on_udp_write_cb(uv_udp_send_t* req, int status )
{
	if(req != nullptr ){
		CUVUDPClient::st_wd *wd = ( CUVUDPClient::st_wd*)( req->data );
		if( wd == NULL ) return;
		uv_buf_t *pt = ( uv_buf_t* )&( wd -> __buff);
		CUVUDPClient *obj = ( CUVUDPClient*)( wd -> __this);
		if( pt != nullptr && pt->base != NULL ){
			free( pt->base );
			size_t s;
			CUVUDPClient::err_code e;
			if( status < 0 ){
				e = CUVUDPClient::ERR_UVCLIENT_OPERATION;
			}
			s = wd->__ds;
			//回调入对象，处理对象内事务
			obj->OnSend( s , e );
		}
		free( wd );
		free( req );
	}
	if( status < 0 ){
		LOGLN( true , "Write fail." );
		exit(-1);
	}
}

bool CUVUDPClient :: need_dns( const std::string& url )
{
      bool ret = false;
      std::regex reg( "[12]\\d{2}(\\.[12]\\d{2}){3}" );   // IPV4 匹配正则表达式

      ret = std::regex_search( url , reg );
      ret = !ret;

      return ret;
}

//////////////////////////////////////////////////////////////////////////
CUVUDPClient :: CUVUDPClient( const std::string& url , int port )
{
	m_o_buf = uv_buf_init( (char*)malloc(1024) , 1024 );
	m_i_buf = uv_buf_init( (char*)malloc(1024) , 1024 );

	m_remote = url;    m_port = port;
	looper * lp = nullptr;     int err = 0;
	lp = GetLooper();

	if( lp != nullptr ){
		err = uv_udp_init( lp->get() , &m_udp );
		if( !err && url.empty() == false )
			connect();
		err = uv_mutex_init( &m_mutex );
		if( err ){ //初始化互斥对象错误
			m_err = ERR_UVCLIENT_INIT_MUTEX;
			err_msg = "Initialize mutex fail";
			return;
		}
		m_udp.data = this;
	}
	else{//初始化UDP错误
		m_err = ERR_UVCLIENT_INIT_TCP;    err_msg = "Initiallize tcp fail";     return;
	}

	m_conn_flag = true;
}
CUVUDPClient :: ~CUVUDPClient(  )
{
	uv_read_stop( (uv_stream_t*)&m_udp );

	uv_shutdown_t  req ;

	uv_shutdown( &req , (uv_stream_t*)&m_udp , nullptr );
	uv_close((uv_handle_t*) &m_udp , nullptr );
    //等候100ms，然所有的异步操作完成
#if defined( __WIN32 )
    	Sleep( 100 );
#else
    	usleep( 100000 );
#endif // defined
}

void CUVUDPClient :: OnSend( size_t s , CUVUDPClient::err_code e )
{
	if( cb_send ){
		cb_send( s );
	}
}


CUVUDPClient :: err_code CUVUDPClient :: Send( const char* data , size_t s  )
{
	int ret = 0;
	uv_udp_send_t   *req_write;                  //发送请求句柄
	//初始化需要传递给发送完成回调函数的参数.
	//分配的的内存在两个地方释放，1  如果正常完成操作，在回调函数中释放；2 如果没有发送完成在错误处理中释放
	struct st_wd *wd = ( struct st_wd*)malloc( sizeof( struct st_wd ) );
	if( wd == NULL ) throw FATAL_NO_MEM;   //内存分配失败
	wd->__this = this;
	wd->__ds = s;

	req_write = (uv_udp_send_t*)malloc( sizeof( uv_udp_send_t));
	if( req_write == nullptr ){
		return ERR_UVCLIENT_ALOCMEM;
	}
	wd->__buff.base =  (char*)malloc( s ) ;
	if( wd->__buff.base == nullptr ){
		return ERR_UVCLIENT_ALOCMEM;
	}else{
		wd->__buff.len = s;
	}

	//开始发送
	if( wd->__buff.base != NULL ){
		req_write->data = wd;
		memcpy( wd->__buff.base , data , s );
		ret = uv_udp_send( req_write , &m_udp, &wd->__buff , 1 , &m_addr , do_on_udp_write_cb );
		if( ret <  0 ){
			Error( ret );
			free( wd->__buff.base );
			free( wd );
			throw ERR_UVCLIENT_CONNECTION;
		}
	}
	else{
		free( wd );
		return ERR_UVCLIENT_ALOCMEM;
	}

	return m_err;
}


void CUVUDPClient :: connect()
{
	std::string url = m_remote;
	int port = m_port;
	if( need_dns( url ) ){// 进行域名解析
		resolver dns( url , port , false , [ this ]( struct addrinfo * i ){
			if( i ){
			memcpy( &m_addr , i -> ai_addr , sizeof( struct sockaddr));     // 保存数据
			uv_udp_recv_start( &m_udp , do_on_udp_alloc_buff , do_on_udp_recv );
			if( cb_conn )
				cb_conn();
			}else{
			// TODO add error code for DNS resolve fail
			}
		});

		while( !dns.finish() ){
			std::this_thread::yield();
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ));
		}
	}else{
		std::regex regip4("[12]\\d{0,2}(\\.[12]\\d{0,2}){3}");
		if( std::regex_match( url , regip4 ) == true ){// 是ip4的ip地址
			uv_ip4_addr( url.c_str() , port , (struct sockaddr_in*)&m_addr );
		}else{
			uv_ip6_addr( url.c_str() , port , (struct sockaddr_in6*)&m_addr );
		}

		if( cb_conn ){
			cb_conn();
		}
	}
}
void CUVUDPClient :: connect( const std::string& url , int port )
{
	m_remote = url;
	m_port = port;

	connect();
}

void CUVUDPClient :: OnRecv( const char* buff , size_t s , err_code e )
{
	if( buff == nullptr || s == 0 ) return;

	if( cb_recv ){
		cb_recv( s ,buff );
	}
}
